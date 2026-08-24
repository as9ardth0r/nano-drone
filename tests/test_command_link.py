from nanodrone_sim.command_link import CommandLink, encode_command, parse_command


def test_encode_then_parse_roundtrip():
    frame = encode_command(500, -200, 0)
    cmd = parse_command(frame)
    assert cmd is not None
    assert (cmd.vx_mm_s, cmd.vy_mm_s, cmd.vz_mm_s) == (500, -200, 0)


def test_parse_rejects_bad_checksum():
    frame = encode_command(500, 0, 0)
    corrupted = frame.replace("*", "X", 1) if "*" not in frame[:1] else frame
    # force un checksum manifestement faux
    bad = frame.rsplit("*", 1)[0] + "*00\n"
    assert parse_command(bad) is None


def test_parse_rejects_garbage():
    assert parse_command("not a command") is None
    assert parse_command("") is None
    assert parse_command("V1,2*AA") is None  # il manque un champ


def test_link_defaults_to_hover_before_any_command():
    link = CommandLink()
    assert link.get_desired_velocity_mm_s(now_s=0.0) == (0, 0, 0)


def test_link_returns_last_command_within_timeout():
    link = CommandLink(failsafe_timeout_s=0.5)
    link.on_line_received(encode_command(300, 0, 0), now_s=10.0)
    assert link.get_desired_velocity_mm_s(now_s=10.3) == (300, 0, 0)


def test_link_falls_back_to_hover_after_timeout():
    """Le test de sécurité le plus important de ce module : un lien
    perdu doit ramener le drone en vol stationnaire, pas le laisser
    continuer sur la dernière commande reçue."""
    link = CommandLink(failsafe_timeout_s=0.5)
    link.on_line_received(encode_command(300, 0, 0), now_s=10.0)
    assert link.get_desired_velocity_mm_s(now_s=10.6) == (0, 0, 0)


def test_invalid_frame_does_not_update_link_state():
    link = CommandLink(failsafe_timeout_s=0.5)
    link.on_line_received(encode_command(300, 0, 0), now_s=10.0)
    updated = link.on_line_received("garbage", now_s=10.1)
    assert not updated
    assert link.get_desired_velocity_mm_s(now_s=10.1) == (300, 0, 0)
