#pragma once

#include <execution>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

class State
{
public:
	State(GLsizei vertex_count) : 
		vertex_count(vertex_count), 
		vertices((Vertex*)::operator new(vertex_count * sizeof(Vertex))),
		colors((Color*)::operator new(vertex_count * sizeof(Color)))
	{

	}
	~State()
	{

	}

	Vertex* get_vertices() const
	{
		return vertices;
	}
	Color* get_colors() const
	{
		return colors;
	}

	void draw(const Boid* boids, float interp)
	{
		std::for_each(
			std::execution::par_unseq,
			boids,
			boids + Config::boid_count,
			[&](const Boid& boid)
			{
				int v = (&boid - boids) * 3;

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
	Vertex* vertices;
	Color* colors;
};