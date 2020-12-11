#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>

#include "Utilities.h"
#include "Vector2.h"

using vec2d = sf::Vector2<double>;

class Boid
{
public:
	Boid();
	Boid(vec2d pos, vec2d size, sf::Vector3<double> color, double maxSpeed, double maxSteer, double minDistance, double viewAngle);

	void update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids);

private:
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

public:
	inline vec2d getPosition() const { return position; }
	inline vec2d getSize() const { return size; }

	inline vec2d getOrigin() const
	{
		return vec2d(
			position.x + (size.x / 2),
			position.y + (size.y / 2));
	}

	inline vec2d getVelocity() const { return velocity; }

	inline double getRotation() const { return rotation; }
	inline double getMinDistance() const { return minDistance; }

	inline sf::Vector3f getColor() const { return color; }

private:
	vec2d position;
	vec2d velocity;

	vec2d size;

	sf::Vector3f color;

	double rotation;	  // Current rotation
	double maxSpeed;	  // Maximum speed
	double maxSteer;	  // Maximum steering force towards target
	double minDistance; // Only interact with boids within this distance
	double viewAngle;	  // Only interact with boids within this angle
};

