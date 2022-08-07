#pragma once

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <execution>
#include <vector>

struct Vertex
{
	GLfloat x, y;
};

struct Color
{
	GLfloat r, g, b;
};

class State
{
public:
	State(GLsizei vertex_count) : vertex_count(vertex_count)
	{
		vertices.resize(vertex_count);
		colors.resize(vertex_count);

		glVertexPointer(2, GL_FLOAT, 0, vertices.data());
		glColorPointer(3, GL_FLOAT, 0, colors.data());
	}
	~State()
	{

	}

	void resize(GLsizei vertex_count)
	{
		this->vertex_count = vertex_count;

		vertices.resize(vertex_count);
		colors.resize(vertex_count);

		glVertexPointer(2, GL_FLOAT, 0, vertices.data());
		glColorPointer(3, GL_FLOAT, 0, colors.data());
	}

	void update(const std::vector<Boid>& boids, float interp)
	{
		std::for_each(
			std::execution::par_unseq, boids.begin(), boids.end(),
			[&](const Boid& boid)
			{
				int v = (&boid - boids.data()) * 3;

				sf::Vector2f p0 = boid.get_pointA() * interp + boid.get_prev_pointA() * (1.0f - interp);
				sf::Vector2f p1 = boid.get_pointB() * interp + boid.get_prev_pointB() * (1.0f - interp);
				sf::Vector2f p2 = boid.get_pointC() * interp + boid.get_prev_pointC() * (1.0f - interp);

				vertices[v	  ] = *(Vertex*)(&p0);
				vertices[v + 1] = *(Vertex*)(&p1);
				vertices[v + 2] = *(Vertex*)(&p2);

				sf::Vector3f color = boid.get_color() * interp + boid.get_prev_color() * (1.0f - interp);

				colors[v	] = *(Color*)(&color);
				colors[v + 1] = *(Color*)(&color);
				colors[v + 2] = *(Color*)(&color);
			});
	}

private:
	GLsizei vertex_count;

	std::vector<Vertex> vertices;
	std::vector<Color> colors;
};