import numpy as np
import pytest

from nanodrone_sim.scenario import run
from nanodrone_sim.sensors import SensorArray
from nanodrone_sim.world import Room


@pytest.fixture
def room():
    return Room(size_x=10.0, size_y=6.0, size_z=3.0)


@pytest.fixture
def sensors():
    return SensorArray(max_range=2.0)


def test_baseline_without_avoidance_hits_the_wall(room, sensors):
    """Scénario de référence : le drone vole droit devant lui vers un mur
    sans aucune réaction aux capteurs. Doit heurter le mur — confirme que
    le scénario de test est bien exigeant (sinon le test suivant ne
    prouverait rien)."""
    result = run(
        room, sensors,
        start_position=[1.0, 3.0, 1.5],
        desired_velocity=np.array([2.0, 0.0, 0.0]),
        duration_s=6.0,
        avoidance_enabled=False,
    )
    assert result.collided


def test_avoidance_prevents_wall_collision(room, sensors):
    """Même scénario, avec la couche d'évitement active : ne doit jamais
    toucher le mur, et doit conserver une marge de sécurité positive
    tout au long du vol."""
    result = run(
        room, sensors,
        start_position=[1.0, 3.0, 1.5],
        desired_velocity=np.array([2.0, 0.0, 0.0]),
        duration_s=6.0,
        avoidance_enabled=True,
    )
    assert not result.collided
    assert result.min_wall_clearance > 0.0


def test_baseline_without_avoidance_hits_the_ceiling(room, sensors):
    result = run(
        room, sensors,
        start_position=[5.0, 3.0, 0.5],
        desired_velocity=np.array([0.0, 0.0, 2.0]),
        duration_s=6.0,
        avoidance_enabled=False,
    )
    assert result.collided


def test_avoidance_prevents_ceiling_collision(room, sensors):
    result = run(
        room, sensors,
        start_position=[5.0, 3.0, 0.5],
        desired_velocity=np.array([0.0, 0.0, 2.0]),
        duration_s=6.0,
        avoidance_enabled=True,
    )
    assert not result.collided
    assert result.min_wall_clearance > 0.0


def test_avoidance_still_allows_forward_progress_when_far_from_walls(room, sensors):
    """L'évitement ne doit pas bloquer le mouvement quand aucun obstacle
    n'est proche — sinon un évitement "trop prudent" serait aussi un
    échec (drone qui ne bouge jamais)."""
    result = run(
        room, sensors,
        start_position=[1.0, 3.0, 1.5],
        desired_velocity=np.array([1.0, 0.0, 0.0]),
        duration_s=1.0,
        avoidance_enabled=True,
    )
    distance_traveled = np.linalg.norm(np.array(result.positions[-1]) - np.array([1.0, 3.0, 1.5]))
    assert distance_traveled > 0.5  # a bien avancé, pas resté figé


def test_diagonal_approach_toward_corner_avoids_both_walls(room, sensors):
    """Cas plus dur : approche en diagonale d'un coin (deux murs
    simultanément proches). Vérifie que les répulsions de plusieurs
    capteurs se combinent correctement plutôt que de s'annuler."""
    result = run(
        room, sensors,
        start_position=[1.0, 1.0, 1.5],
        desired_velocity=np.array([2.0, 2.0, 0.0]),
        duration_s=6.0,
        avoidance_enabled=True,
    )
    assert not result.collided
    assert result.min_wall_clearance > 0.0
