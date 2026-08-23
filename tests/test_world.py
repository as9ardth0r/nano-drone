import numpy as np
import pytest

from nanodrone_sim.world import Room


@pytest.fixture
def room():
    return Room(size_x=10.0, size_y=6.0, size_z=3.0)


def test_distance_to_front_wall(room):
    p = np.array([5.0, 3.0, 1.5])
    d = np.array([1.0, 0.0, 0.0])
    assert room.distance_to_wall(p, d, max_range=8.0) == pytest.approx(5.0)


def test_distance_capped_at_sensor_max_range(room):
    # le mur physique est à 5 m mais le capteur ne voit qu'à 4 m : la
    # lecture doit être plafonnée à max_range, pas donner la vraie distance
    p = np.array([5.0, 3.0, 1.5])
    d = np.array([1.0, 0.0, 0.0])
    assert room.distance_to_wall(p, d, max_range=4.0) == pytest.approx(4.0)


def test_distance_to_ceiling(room):
    p = np.array([5.0, 3.0, 1.5])
    d = np.array([0.0, 0.0, 1.0])
    assert room.distance_to_wall(p, d, max_range=4.0) == pytest.approx(1.5)


def test_distance_near_wall_is_small(room):
    p = np.array([9.5, 3.0, 1.5])
    d = np.array([1.0, 0.0, 0.0])
    assert room.distance_to_wall(p, d, max_range=4.0) == pytest.approx(0.5)


def test_distance_to_side_wall(room):
    p = np.array([5.0, 3.0, 1.5])
    d = np.array([0.0, 1.0, 0.0])
    assert room.distance_to_wall(p, d, max_range=4.0) == pytest.approx(3.0)


def test_is_inside(room):
    assert room.is_inside(np.array([5.0, 3.0, 1.5]))
    assert not room.is_inside(np.array([-1.0, 3.0, 1.5]))
    assert not room.is_inside(np.array([5.0, 3.0, 3.5]))
