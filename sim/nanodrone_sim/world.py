"""Modèle du monde : une pièce parallélépipédique simple (murs, sol,
plafond alignés sur les axes). Suffisant pour valider la logique
d'évitement — pas un moteur physique complet.
"""
from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class Room:
    """Pièce rectangulaire : x in [0, size_x], y in [0, size_y], z in [0, size_z].
    z=0 est le sol, z=size_z est le plafond."""
    size_x: float = 10.0
    size_y: float = 6.0
    size_z: float = 3.0

    def distance_to_wall(self, position: np.ndarray, direction: np.ndarray,
                          max_range: float) -> float:
        """Distance jusqu'au premier mur touché en partant de `position`
        dans la direction `direction` (normalisée), plafonnée à
        `max_range` (portée du capteur ToF). Méthode du "slab test"
        (intersection rayon/boîte), standard en géométrie 3D."""
        direction = direction / np.linalg.norm(direction)
        t_min, t_max = 0.0, max_range

        bounds_min = np.array([0.0, 0.0, 0.0])
        bounds_max = np.array([self.size_x, self.size_y, self.size_z])

        for axis in range(3):
            if abs(direction[axis]) < 1e-9:
                # rayon parallèle à cette paire de murs : doit déjà être
                # à l'intérieur, sinon aucune intersection possible
                if position[axis] < bounds_min[axis] or position[axis] > bounds_max[axis]:
                    return max_range
                continue
            inv_d = 1.0 / direction[axis]
            t1 = (bounds_min[axis] - position[axis]) * inv_d
            t2 = (bounds_max[axis] - position[axis]) * inv_d
            t_near, t_far = min(t1, t2), max(t1, t2)
            t_min = max(t_min, t_near)
            t_max = min(t_max, t_far)
            if t_min > t_max:
                return max_range

        return float(np.clip(t_max, 0.0, max_range))

    def is_inside(self, position: np.ndarray, margin: float = 0.0) -> bool:
        return bool(
            margin <= position[0] <= self.size_x - margin
            and margin <= position[1] <= self.size_y - margin
            and margin <= position[2] <= self.size_z - margin
        )
