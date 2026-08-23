"""Dynamique simplifiée du drone : masse ponctuelle dont la vitesse suit
la vitesse commandée avec une accélération bornée (modèle du premier
ordre). Hypothèse assumée : la stabilisation d'attitude bas niveau
(boucle PID taux/angle) est supposée parfaite ici — ce module ne valide
que la couche de guidage/évitement, pas le vol lui-même. Voir la section
"Limites" du README.
"""
from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class DroneState:
    position: np.ndarray
    velocity: np.ndarray = None

    def __post_init__(self):
        if self.velocity is None:
            self.velocity = np.zeros(3)


def step(state: DroneState, commanded_velocity: np.ndarray, dt: float,
         max_acceleration: float = 8.0) -> DroneState:
    """Fait avancer la simulation d'un pas de temps `dt` (s). La vitesse
    évolue vers `commanded_velocity` avec une accélération plafonnée à
    `max_acceleration` (m/s²), puis la position est intégrée (Euler
    explicite — suffisant au pas de temps utilisé ici, voir tests)."""
    velocity_error = commanded_velocity - state.velocity
    error_norm = np.linalg.norm(velocity_error)
    max_delta_v = max_acceleration * dt

    if error_norm > max_delta_v and error_norm > 1e-9:
        velocity_error = velocity_error / error_norm * max_delta_v

    new_velocity = state.velocity + velocity_error
    new_position = state.position + new_velocity * dt
    return DroneState(position=new_position, velocity=new_velocity)
