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

	void Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids);

private:
	std::vector<Boid> VisibleBoids(const std::vector<Boid>& boids);

	void Flock(const std::vector<Boid>& boids);

	vec2d Seperate(const std::vector<Boid>& boids);
	vec2d Align(const std::vector<Boid>& boids);
	vec2d Cohesion(const std::vector<Boid>& boids);

	void OutsideBorder(const sf::Window* window);

	inline void ApplyForce(const vec2d& force)
	{
		velocity += force;
	}

public:
	inline vec2d GetPosition() const { return position; }
	inline vec2d GetSize() const { return size; }

	inline vec2d GetOrigin() const
	{
		return vec2d(
			position.x + (size.x / 2),
			position.y + (size.y / 2));
	}

	inline vec2d GetVelocity() const { return velocity; }

	inline double GetRotation() const { return rotation; }
	inline double GetMinDistance() const { return minDistance; }

	inline sf::Vector3f GetColor() const { return color; }

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

