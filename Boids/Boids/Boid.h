#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities.h"
#include "Vector2.h"
#include "Grid.h"

class Boid
{
public:
	Boid();
	Boid(
		sf::Vector2f pos, sf::Vector2f size, 
		float w_sep, float w_ali, float w_coh, 
		float max_speed, float max_steer, float min_distance, float view_angle);

	void update(const sf::Window* window, const float& deltaTime, const std::vector<Boid>& boids);

	void assign_container(Container& cont)
	{
		if (container != nullptr && container != &cont)
		{
			container->erase(*this);
		}
		else
		{
			container = &cont;
			container->insert(*this);
		}
	}

private: // Flocking
	std::vector<Boid> visible_boids(const std::vector<Boid>& boids);

	void flock(const std::vector<Boid>& boids);

	sf::Vector2f seperate(const std::vector<Boid>& boids);
	sf::Vector2f align(const std::vector<Boid>& boids);
	sf::Vector2f cohesion(const std::vector<Boid>& boids);

	void outside_border(const sf::Window* window);

	inline void apply_force(const sf::Vector2f& force)
	{
		velocity += force;
	}

public: // Properties
	inline sf::Vector2f get_position() const { return position; }
	inline sf::Vector2f get_size() const { return size; }

	inline sf::Vector2f get_origin() const
	{
		return sf::Vector2f(
			position.x + (size.x / 2),
			position.y + (size.y / 2));
	}

	inline sf::Vector2f get_velocity() const { return velocity; }

	inline float get_rotation() const { return rotation; }
	inline float get_min_distance() const { return min_distance; }

	inline sf::Vector3f get_color() const { return color; }

private: // Variables
	sf::Vector2f position;
	sf::Vector2f velocity;		  // Current headed velocity
	sf::Vector2f size;

	sf::Vector3f color;

	float weight_sep;
	float weight_ali;
	float weight_coh;

	float rotation;		 // Current rotation
	float max_speed;	 // Maximum speed
	float max_steer;	 // Maximum steering force towards target
	float min_distance;  // Only interact with boids within this distance
	float view_angle;	 // Only interact with boids within this angle

	Container* container; // Currently assigned container...
};

