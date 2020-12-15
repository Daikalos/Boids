#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities.h"
#include "Vector2.h"

using vec2d = sf::Vector2<double>;
using vec3d = sf::Vector3<double>;

class Boid
{
public:
	Boid();
	Boid(vec2d pos, vec2d size, double w_sep, double w_ali, double w_coh, double max_speed, double max_steer, double min_distance, double view_angle);

	void update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids);

private: // Flocking
	std::vector<Boid> visible_boids(const std::vector<Boid>& boids);

	void flock(const std::vector<Boid>& boids);

	vec2d seperate(const std::vector<Boid>& boids);
	vec2d align(const std::vector<Boid>& boids);
	vec2d cohesion(const std::vector<Boid>& boids);

	void outside_border(const sf::Window* window);

	inline void apply_force(const vec2d& force)
	{
		velocity += force;
	}

public: // Properties
	inline vec2d get_position() const { return position; }
	inline vec2d get_size() const { return size; }

	inline vec2d get_origin() const
	{
		return vec2d(
			position.x + (size.x / 2),
			position.y + (size.y / 2));
	}

	inline vec2d get_velocity() const { return velocity; }

	inline double get_rotation() const { return rotation; }
	inline double get_min_distance() const { return min_distance; }

	inline vec3d get_color() const { return color; }

private: // Variables
	vec2d position;
	vec2d velocity;		  // Current headed velocity
	vec2d size;

	vec3d color;

	double weight_sep;
	double weight_ali;
	double weight_coh;

	double rotation;	  // Current rotation
	double max_speed;	  // Maximum speed
	double max_steer;	  // Maximum steering force towards target
	double min_distance;   // Only interact with boids within this distance
	double view_angle;	  // Only interact with boids within this angle
};

