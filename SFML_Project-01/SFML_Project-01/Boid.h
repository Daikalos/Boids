#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Utility.h"

#define WALL_COLLISION 0

struct Vertex
{
	GLfloat x, y;
	GLfloat r, g, b;
};

struct Triangle
{
	Vertex vertices[3];
};

class Boid
{
public:
	Boid();

	Boid(sf::Vector2f pos, sf::Vector3f color);
	~Boid();

	void Update(const sf::Window* window, const float& deltaTime);

	void ApplyForce(const sf::Vector2f& force) 
	{
		m_Velocity += force;
	}

	inline sf::Vector2f GetPosition() const { return m_Position; }
	inline sf::Vector2f GetVelocity() const { return m_Velocity; }

	inline sf::Vector3f GetColor() const { return m_Color; }

private:
	float m_Friction;

	sf::Vector2f m_Position;
	sf::Vector2f m_Velocity;

	sf::Vector3f m_Color;
};

