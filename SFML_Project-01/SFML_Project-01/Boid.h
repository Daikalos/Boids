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
		m_Velocity += force;
	}

public:
	inline vec2d GetPosition() const { return m_Position; }
	inline vec2d GetSize() const { return m_Size; }

	inline vec2d GetOrigin() const
	{
		return vec2d(
			m_Position.x + (m_Size.x / 2),
			m_Position.y + (m_Size.y / 2));
	}

	inline vec2d GetVelocity() const { return m_Velocity; }

	inline double GetRotation() const { return m_Rotation; }
	inline double GetMinDistance() const { return m_MinDistance; }

	inline sf::Vector3f GetColor() const { return m_Color; }

private:
	vec2d m_Position;
	vec2d m_Velocity;

	vec2d m_Size;

	sf::Vector3f m_Color;

	double m_Rotation;	  // Current rotation
	double m_MaxSpeed;	  // Maximum speed
	double m_MaxSteer;	  // Maximum steering force towards target
	double m_MinDistance; // Only interact with boids within this distance
	double m_ViewAngle;	  // Only interact with boids within this angle
};

