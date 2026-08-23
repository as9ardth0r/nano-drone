"""Fait tourner un scénario complet : le drone reçoit une consigne de
vitesse désirée (ex. "avance tout droit") pendant N pas de temps, et on
enregistre trajectoire + lectures capteurs, avec ou sans la couche
d'évitement — pour comparer directement.
"""
from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np

from .avoidance import command_velocity
from .dynamics import DroneState, step
from .sensors import SensorArray
from .world import Room


@dataclass
class ScenarioResult:
    positions: list = field(default_factory=list)
    min_wall_clearance: float = float("inf")
    collided: bool = False


def run(room: Room, sensors: SensorArray, start_position: np.ndarray,
        desired_velocity: np.ndarray, duration_s: float, dt: float = 0.02,
        avoidance_enabled: bool = True, safety_margin: float = 0.5,
        gain: float = 1.5, max_speed: float = 3.0,
        max_acceleration: float = 8.0) -> ScenarioResult:
    """Simule `duration_s` secondes de vol. Si `avoidance_enabled` est
    False, la vitesse commandée est directement `desired_velocity`
    (aucune réaction aux capteurs) — sert de scénario de référence pour
    prouver que l'évitement fait une différence réelle, pas suppposée."""
    state = DroneState(position=np.array(start_position, dtype=float))
    result = ScenarioResult()
    n_steps = int(duration_s / dt)

    for _ in range(n_steps):
        distances = sensors.read(room, state.position)
        min_clearance = min(distances.values())
        result.min_wall_clearance = min(result.min_wall_clearance, min_clearance)

        if not room.is_inside(state.position):
            result.collided = True
            result.positions.append(state.position.copy())
            break

        if avoidance_enabled:
            cmd_v = command_velocity(desired_velocity, distances, safety_margin,
                                      gain, max_speed, sensors.directions)
        else:
            cmd_v = desired_velocity

        state = step(state, cmd_v, dt, max_acceleration)
        result.positions.append(state.position.copy())

    return result
