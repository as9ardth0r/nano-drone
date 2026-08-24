"""Vérifie que le parseur de commandes embarqué
(firmware/Core/Src/command_link.c) calcule EXACTEMENT la même chose que
la version Python — même méthode que pour avoidance.c.
"""
from __future__ import annotations

import ctypes
import subprocess
from pathlib import Path

import pytest

from nanodrone_sim.command_link import encode_command, parse_command

REPO_ROOT = Path(__file__).resolve().parent.parent
FIRMWARE_SRC = REPO_ROOT / "firmware" / "Core" / "Src" / "command_link.c"
FIRMWARE_INC = REPO_ROOT / "firmware" / "Core" / "Inc"


class CCommand(ctypes.Structure):
    _fields_ = [("vx", ctypes.c_int32), ("vy", ctypes.c_int32), ("vz", ctypes.c_int32)]


class CLinkState(ctypes.Structure):
    _fields_ = [
        ("failsafe_timeout_s", ctypes.c_float),
        ("last_command", CCommand),
        ("last_received_at_s", ctypes.c_float),
        ("has_received_ever", ctypes.c_bool),
    ]


@pytest.fixture(scope="module")
def link_lib(tmp_path_factory):
    out_dir = tmp_path_factory.mktemp("command_link_native")
    lib_path = out_dir / "libcommand_link.so"
    subprocess.run(
        ["gcc", "-shared", "-fPIC", "-O2",
         "-o", str(lib_path), str(FIRMWARE_SRC), f"-I{FIRMWARE_INC}"],
        check=True,
    )
    lib = ctypes.CDLL(str(lib_path))

    lib.cl_parse_command.argtypes = [ctypes.c_char_p, ctypes.POINTER(CCommand)]
    lib.cl_parse_command.restype = ctypes.c_bool

    lib.cl_link_init.argtypes = [ctypes.POINTER(CLinkState), ctypes.c_float]
    lib.cl_on_line_received.argtypes = [ctypes.POINTER(CLinkState), ctypes.c_char_p, ctypes.c_float]
    lib.cl_on_line_received.restype = ctypes.c_bool
    lib.cl_get_desired_velocity.argtypes = [ctypes.POINTER(CLinkState), ctypes.c_float]
    lib.cl_get_desired_velocity.restype = CCommand
    return lib


@pytest.mark.parametrize("vx,vy,vz", [
    (0, 0, 0), (500, -200, 0), (-999, 999, -1), (1, 2, 3), (12345, -12345, 0),
])
def test_c_parser_matches_python_on_valid_frames(link_lib, vx, vy, vz):
    frame = encode_command(vx, vy, vz)

    py_cmd = parse_command(frame)
    c_cmd = CCommand()
    ok = link_lib.cl_parse_command(frame.strip().encode(), ctypes.byref(c_cmd))

    assert ok
    assert py_cmd is not None
    assert (c_cmd.vx, c_cmd.vy, c_cmd.vz) == (py_cmd.vx_mm_s, py_cmd.vy_mm_s, py_cmd.vz_mm_s)


@pytest.mark.parametrize("garbage", [
    "not a command", "", "V1,2*AA", "V1,2,3*00",  # dernier cas : checksum volontairement faux
])
def test_c_parser_rejects_same_garbage_as_python(link_lib, garbage):
    py_result = parse_command(garbage)
    c_cmd = CCommand()
    c_ok = link_lib.cl_parse_command(garbage.encode(), ctypes.byref(c_cmd))

    assert py_result is None
    assert not c_ok


def test_c_failsafe_matches_python_timeout_behavior(link_lib):
    link = CLinkState()
    link_lib.cl_link_init(ctypes.byref(link), ctypes.c_float(0.5))

    frame = encode_command(300, 0, 0).strip().encode()
    link_lib.cl_on_line_received(ctypes.byref(link), frame, ctypes.c_float(10.0))

    within_timeout = link_lib.cl_get_desired_velocity(ctypes.byref(link), ctypes.c_float(10.3))
    assert (within_timeout.vx, within_timeout.vy, within_timeout.vz) == (300, 0, 0)

    after_timeout = link_lib.cl_get_desired_velocity(ctypes.byref(link), ctypes.c_float(10.6))
    assert (after_timeout.vx, after_timeout.vy, after_timeout.vz) == (0, 0, 0)
