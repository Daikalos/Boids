#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities.h"
#include "VecUtil.h"
#include "Rectangle.h"
#include "Config.h"
#include "Grid.h"

class Boid
{
public:
	Boid(Grid* grid, Boid* boids, sf::Vector2f pos);

	void update(float deltaTime, const Rect_i& border);
	void steer_towards(sf::Vector2f point, float weight);

	void update_grid_cells() const;

	void set_cell_index()
	{
		this->cell_index = grid->at_pos(get_origin());
	}
	int get_cell_index() const
	{
		return cell_index;
	}

private: // Flocking
	void flock();

	sf::Vector2f steer_at(const sf::Vector2f& steer_direction);

	void outside_border(const Rect_i& border, float deltaTime);

	inline void apply_force(const sf::Vector2f& force)
	{
		velocity += force;
	}

public: // Properties
	inline sf::Vector2f get_pointA() const { return pointA; }
	inline sf::Vector2f get_pointB() const { return pointB; }
	inline sf::Vector2f get_pointC() const { return pointC; }

	inline sf::Vector3f get_color() const { return color; }

	inline sf::Vector2f get_position() const { return position; }
	inline sf::Vector2f get_velocity() const { return velocity; }
	inline float get_rotation() const { return rotation; }

	inline sf::Vector2f get_origin() const
	{
		return sf::Vector2f(
			position.x + (Config::boid_size_width / 2),
			position.y + (Config::boid_size_height / 2));
	}

private:
	int interpolate(int a, int b, int c, int d, double t, double s) const
	{
		return (int)(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

private:
	Grid* grid;
	Boid* boids;

	int cell_index;
	int index;

	sf::Vector2f pointA, pointB, pointC;
	sf::Vector3f color;

	sf::Vector2f position;
	sf::Vector2f velocity;
	float rotation;
};

