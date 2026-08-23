"""Simulation du réseau de télémètres ToF (VL53L1X, un capteur = une
direction fixe par rapport au châssis). Reproduit la disposition
"avant/haut/côtés" retenue pour le firmware réel.
"""
from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np

from .world import Room

# Directions body-frame par défaut : avant (+x), arrière (-x), haut (+z),
# gauche (+y), droite (-y). Le firmware réel n'a pas forcément les 5 —
# voir docs/hardware.md pour le choix retenu sur la carte.
DEFAULT_DIRECTIONS = {
    "front": np.array([1.0, 0.0, 0.0]),
    "back": np.array([-1.0, 0.0, 0.0]),
    "up": np.array([0.0, 0.0, 1.0]),
    "left": np.array([0.0, 1.0, 0.0]),
    "right": np.array([0.0, -1.0, 0.0]),
}


@dataclass
class SensorArray:
    """Réseau de capteurs ToF. `max_range` reprend la portée réelle du
    VL53L1X en mode "long distance" (jusqu'à 4 m, mais plus fiable et
    rapide autour de 2 m — voir docs/hardware.md)."""
    max_range: float = 2.0
    directions: dict[str, np.ndarray] = field(default_factory=lambda: dict(DEFAULT_DIRECTIONS))

    def read(self, room: Room, position: np.ndarray) -> dict[str, float]:
        """Retourne la distance lue (m) par chaque capteur à la position
        donnée. Ne modélise pas le bruit de mesure — voir noise.py pour
        une version avec bruit, utilisée dans les tests de robustesse."""
        return {
            name: room.distance_to_wall(position, direction, self.max_range)
            for name, direction in self.directions.items()
        }
