"""Protocole de commande reçu par liaison BLE (module HM-10, voir
docs/hardware.md), et logique de sécurité associée. Miroir exact de
firmware/Core/Src/command_link.c — voir
tests/test_command_link_c_matches_python.py pour la validation croisée.

Format de trame ASCII, une ligne : "V<vx>,<vy>,<vz>*<checksum_hex>\n"
  - vx, vy, vz : entiers signés, mm/s
  - checksum : XOR de tous les octets précédant le '*', en hexadécimal
    2 chiffres (ex. "V500,0,0*4B")
Format volontairement simple (ASCII, pas de flottants à parser) pour
rester utilisable directement depuis une appli BLE générique
("Serial Bluetooth Terminal" ou équivalent), sans appli dédiée.
"""
from __future__ import annotations

from dataclasses import dataclass

FAILSAFE_TIMEOUT_S = 0.5  # au-delà de ce délai sans commande valide, vol stationnaire


@dataclass
class ParsedCommand:
    vx_mm_s: int
    vy_mm_s: int
    vz_mm_s: int


def _checksum(payload: str) -> int:
    value = 0
    for ch in payload:
        value ^= ord(ch)
    return value & 0xFF


def parse_command(line: str) -> ParsedCommand | None:
    """Parse une trame. Retourne None si le format ou le checksum est
    invalide — ne lève jamais d'exception sur une entrée malformée (une
    trame corrompue par le lien radio ne doit jamais faire planter la
    boucle de contrôle)."""
    line = line.strip()
    if not line.startswith("V") or "*" not in line:
        return None

    payload, _, checksum_hex = line.partition("*")
    try:
        expected = int(checksum_hex, 16)
    except ValueError:
        return None
    if _checksum(payload) != expected:
        return None

    fields = payload[1:].split(",")
    if len(fields) != 3:
        return None
    try:
        vx, vy, vz = (int(f) for f in fields)
    except ValueError:
        return None

    return ParsedCommand(vx, vy, vz)


def encode_command(vx_mm_s: int, vy_mm_s: int, vz_mm_s: int) -> str:
    """Construit une trame valide — utilisé par les tests et comme
    référence pour implémenter le côté émetteur (appli téléphone)."""
    payload = f"V{vx_mm_s},{vy_mm_s},{vz_mm_s}"
    return f"{payload}*{_checksum(payload):02X}\n"


class CommandLink:
    """Suivi de l'état du lien : mémorise la dernière commande valide et
    son horodatage, applique le repli sécuritaire si le lien est trop
    silencieux."""

    def __init__(self, failsafe_timeout_s: float = FAILSAFE_TIMEOUT_S):
        self.failsafe_timeout_s = failsafe_timeout_s
        self._last_command = ParsedCommand(0, 0, 0)
        self._last_received_at: float | None = None

    def on_line_received(self, line: str, now_s: float) -> bool:
        """À appeler à chaque trame reçue sur l'UART. Retourne True si la
        trame était valide et a mis à jour la commande courante."""
        cmd = parse_command(line)
        if cmd is None:
            return False
        self._last_command = cmd
        self._last_received_at = now_s
        return True

    def get_desired_velocity_mm_s(self, now_s: float) -> tuple[int, int, int]:
        """Vitesse désirée courante. Vol stationnaire (0,0,0) si aucune
        commande valide n'a été reçue depuis failsafe_timeout_s — un
        lien coupé ne doit jamais laisser la dernière commande active
        indéfiniment (ex. "avance" reçu puis téléphone hors de portée)."""
        if self._last_received_at is None:
            return (0, 0, 0)
        if now_s - self._last_received_at > self.failsafe_timeout_s:
            return (0, 0, 0)
        return (self._last_command.vx_mm_s, self._last_command.vy_mm_s, self._last_command.vz_mm_s)
