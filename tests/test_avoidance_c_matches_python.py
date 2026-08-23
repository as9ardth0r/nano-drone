"""Vérifie que le code d'évitement embarqué (firmware/Core/Src/avoidance.c)
calcule EXACTEMENT la même chose que la version Python utilisée pour la
simulation — pas juste "porté à l'œil". Compile le C nativement (pas la
cible ARM) et compare numériquement via ctypes.
"""
from __future__ import annotations

import ctypes
import subprocess
from pathlib import Path

import numpy as np
import pytest

from nanodrone_sim.avoidance import command_velocity
from nanodrone_sim.sensors import DEFAULT_DIRECTIONS

REPO_ROOT = Path(__file__).resolve().parent.parent
FIRMWARE_SRC = REPO_ROOT / "firmware" / "Core" / "Src" / "avoidance.c"
FIRMWARE_INC = REPO_ROOT / "firmware" / "Core" / "Inc"

# doit correspondre à l'ordre de nd_sensor_id_t dans avoidance.h
SENSOR_ORDER = ["front", "back", "up", "left", "right"]


class Vec3(ctypes.Structure):
    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float), ("z", ctypes.c_float)]


@pytest.fixture(scope="module")
def avoidance_lib(tmp_path_factory):
    out_dir = tmp_path_factory.mktemp("avoidance_native")
    lib_path = out_dir / "libavoidance.so"
    subprocess.run(
        ["gcc", "-shared", "-fPIC", "-O2", "-lm",
         "-o", str(lib_path), str(FIRMWARE_SRC), f"-I{FIRMWARE_INC}"],
        check=True,
    )
    lib = ctypes.CDLL(str(lib_path))
    lib.nd_command_velocity.argtypes = [
        Vec3, ctypes.c_float * 5, ctypes.c_float, ctypes.c_float, ctypes.c_float,
    ]
    lib.nd_command_velocity.restype = Vec3
    return lib


def _call_c(lib, desired_velocity, distances, safety_margin, gain, max_speed):
    dv = Vec3(*desired_velocity)
    dist_array = (ctypes.c_float * 5)(*[distances[name] for name in SENSOR_ORDER])
    result = lib.nd_command_velocity(dv, dist_array, safety_margin, gain, max_speed)
    return np.array([result.x, result.y, result.z])


@pytest.mark.parametrize("desired_velocity,distances", [
    ([2.0, 0.0, 0.0], {"front": 0.3, "back": 2.0, "up": 2.0, "left": 2.0, "right": 2.0}),
    ([0.0, 0.0, 2.0], {"front": 2.0, "back": 2.0, "up": 0.2, "left": 2.0, "right": 2.0}),
    ([1.0, 1.0, 0.0], {"front": 0.4, "back": 2.0, "up": 2.0, "left": 0.4, "right": 2.0}),
    ([0.0, 0.0, 0.0], {"front": 2.0, "back": 2.0, "up": 2.0, "left": 2.0, "right": 2.0}),
    ([-1.5, 0.5, -0.5], {"front": 2.0, "back": 0.15, "up": 2.0, "left": 2.0, "right": 0.45}),
])
def test_c_firmware_matches_python_simulation(avoidance_lib, desired_velocity, distances):
    safety_margin, gain, max_speed = 0.5, 1.5, 3.0

    python_result = command_velocity(
        np.array(desired_velocity), distances, safety_margin, gain, max_speed,
        DEFAULT_DIRECTIONS,
    )
    c_result = _call_c(avoidance_lib, desired_velocity, distances, safety_margin, gain, max_speed)

    np.testing.assert_allclose(c_result, python_result, atol=1e-4)
