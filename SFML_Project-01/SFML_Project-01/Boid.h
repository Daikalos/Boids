#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>

#include "Utility.h"

struct Vertex
{
	GLdouble x, y;
};

struct Color
{
	GLdouble r, g, b;
};

class Boid
{
public:
	Boid();

	Boid(sf::Vector2<double> pos, sf::Vector2<double> size, sf::Vector3<double> color, double maxSpeed, double maxForce, double minDistance);
	~Boid();

	void Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids);

	inline double GetRotation() const { return m_Rotation; }

	inline sf::Vector2<double> GetSize() const { return m_Size; }

	inline sf::Vector2<double> GetPosition() const { return m_Position; }
	inline sf::Vector2<double> GetVelocity() const { return m_Velocity; }
	inline sf::Vector2<double> GetAcceleration() const { return m_Acceleration; }

	inline sf::Vector2<double> GetOrigin() const 
	{ 
		return sf::Vector2<double>(
			m_Position.x + (m_Size.x / 2), 
			m_Position.y + (m_Size.y / 2)); 
	}

	inline sf::Vector3f GetColor() const { return m_Color; }

	inline double GetMinDistance() const { return m_MinDistance; }

private:
	void Flock(const std::vector<Boid>& boids);

	void OutsideBorder(const sf::Window* window, const sf::Vector2<double>& nextPos);

	sf::Vector2<double> Seperate(const std::vector<Boid>& boids);
	sf::Vector2<double> Align(const std::vector<Boid>& boids);
	sf::Vector2<double> Cohesion(const std::vector<Boid>& boids);

	inline void ApplyForce(const sf::Vector2<double>& force)
	{
		m_Acceleration += force;
	}

private:
	sf::Vector2<double> m_Position;
	sf::Vector2<double> m_Velocity;
	sf::Vector2<double> m_Acceleration;

	sf::Vector2<double> m_Size;

	sf::Vector3f m_Color;

	double m_Rotation;
	double m_MaxSpeed;
	double m_MaxForce;
	double m_MinDistance;
};

