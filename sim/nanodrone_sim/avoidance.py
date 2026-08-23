"""Contrôleur d'évitement réactif — champ de potentiel artificiel
(Khatib, 1986) : chaque capteur proche d'un obstacle génère une
répulsion inversement proportionnelle à la distance, sommée à la
consigne de vitesse commandée. Algorithme volontairement simple pour
pouvoir être porté tel quel en C embarqué (voir firmware/Core/Src/avoidance.c,
qui reproduit exactement cette formule).
"""
from __future__ import annotations

import numpy as np

from .sensors import DEFAULT_DIRECTIONS


def repulsion_vector(distances: dict[str, float], safety_margin: float,
                      gain: float, directions: dict[str, np.ndarray] = DEFAULT_DIRECTIONS
                      ) -> np.ndarray:
    """Calcule le vecteur de répulsion total à partir des lectures ToF.

    Pour chaque capteur dont la distance d < safety_margin, ajoute une
    répulsion de norme gain * (1/d - 1/safety_margin), dirigée à
    l'opposé du capteur (formule classique du champ de potentiel
    répulsif). Au-delà de safety_margin, un capteur ne contribue rien.
    """
    total = np.zeros(3)
    for name, d in distances.items():
        if d >= safety_margin or d <= 1e-6:
            continue
        direction = directions[name]
        magnitude = gain * (1.0 / d - 1.0 / safety_margin)
        total += -direction * magnitude
    return total


def command_velocity(desired_velocity: np.ndarray, distances: dict[str, float],
                      safety_margin: float, gain: float, max_speed: float,
                      directions: dict[str, np.ndarray] = DEFAULT_DIRECTIONS) -> np.ndarray:
    """Combine la vitesse désirée (pilote/mission) et la répulsion
    d'évitement, puis borne la norme du résultat à max_speed."""
    repulsion = repulsion_vector(distances, safety_margin, gain, directions)
    combined = desired_velocity + repulsion
    speed = np.linalg.norm(combined)
    if speed > max_speed:
        combined = combined / speed * max_speed
    return combined
