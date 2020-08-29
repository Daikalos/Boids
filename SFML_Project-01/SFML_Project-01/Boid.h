#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>

#include "Utility.h"

struct Vertex
{
	GLfloat x, y;
};

struct Color
{
	GLfloat r, g, b;
};

class Boid
{
public:
	Boid();

	Boid(sf::Vector2f pos, sf::Vector2f size, sf::Vector3f color, float maxSpeed, float maxForce, float minDistance);
	~Boid();

	void Update(const sf::Window* window, const float& deltaTime, const std::vector<Boid>& boids);

	inline float GetRotation() const { return m_Rotation; }

	inline sf::Vector2f GetSize() const { return m_Size; }

	inline sf::Vector2f GetPosition() const { return m_Position; }
	inline sf::Vector2f GetVelocity() const { return m_Velocity; }
	inline sf::Vector2f GetAcceleration() const { return m_Acceleration; }

	inline sf::Vector2f GetOrigin() const 
	{ 
		return sf::Vector2f(
			m_Position.x + (m_Size.x / 2), 
			m_Position.y + (m_Size.y / 2)); 
	}

	inline sf::Vector3f GetColor() const { return m_Color; }

private:
	void Flock(const std::vector<Boid>& boids);

	void OutsideBorder(const sf::Window* window);

	sf::Vector2f Seperate(const std::vector<Boid>& boids);
	sf::Vector2f Align(const std::vector<Boid>& boids);
	sf::Vector2f Cohesion(const std::vector<Boid>& boids);

	inline void ApplyForce(const sf::Vector2f& force)
	{
		m_Acceleration += force;
	}

private:
	sf::Vector2f m_Position;
	sf::Vector2f m_Velocity;
	sf::Vector2f m_Acceleration;

	sf::Vector2f m_Size;

	sf::Vector3f m_Color;

	float m_Rotation;
	float m_MaxSpeed;
	float m_MaxForce;
	float m_MinDistance;
};

