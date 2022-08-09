#pragma once

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <execution>
#include <vector>

#include "Config.h"

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
	State(GLsizei vertex_count)
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
		vertices.resize(vertex_count);
		colors.resize(vertex_count);

		glVertexPointer(2, GL_FLOAT, 0, vertices.data());
		glColorPointer(3, GL_FLOAT, 0, colors.data());
	}

	void update(const std::vector<Boid>& boids, const Config& config, float interp)
	{
		std::for_each(
			std::execution::par_unseq, boids.begin(), boids.end(),
			[&](const Boid& boid)
			{
				int v = std::distance(boids.data(), &boid) * 3;

				sf::Vector2f pos = boid.get_position();
				sf::Vector2f prev_pos = boid.get_prev_position();

				sf::Vector2f origin = sf::Vector2f(pos.x + config.boid_size_width / 2.0f, pos.y + config.background_height / 2.0f);
				sf::Vector2f prev_origin = sf::Vector2f(prev_pos.x + config.boid_size_width / 2.0f, prev_pos.y + config.background_height / 2.0f);

				float rot = v2f::angle(boid.get_velocity());
				float prev_rot = v2f::angle(boid.get_prev_velocity());

				sf::Vector2f pointA = v2f::rotate_point({ pos.x + config.boid_size_width, pos.y + (config.boid_size_height / 2) }, origin, rot); // middle right tip
				sf::Vector2f pointB = v2f::rotate_point({ pos.x							, pos.y									}, origin, rot); // top left corner
				sf::Vector2f pointC = v2f::rotate_point({ pos.x							, pos.y + config.boid_size_height		}, origin, rot); // bot left corner

				sf::Vector2f prev_pointA = v2f::rotate_point({ prev_pos.x + config.boid_size_width, prev_pos.y + (config.boid_size_height / 2) }, prev_origin, prev_rot); // middle right tip
				sf::Vector2f prev_pointB = v2f::rotate_point({ prev_pos.x						  , prev_pos.y								   }, prev_origin, prev_rot); // top left corner
				sf::Vector2f prev_pointC = v2f::rotate_point({ prev_pos.x						  , prev_pos.y + config.boid_size_height	   }, prev_origin, prev_rot); // bot left corner

				sf::Vector2f p0 = pointA * interp + prev_pointA * (1.0f - interp);
				sf::Vector2f p1 = pointB * interp + prev_pointB * (1.0f - interp);
				sf::Vector2f p2 = pointC * interp + prev_pointC * (1.0f - interp);

				vertices[v	  ] = *(Vertex*)(&p0);
				vertices[v + 1] = *(Vertex*)(&p1);
				vertices[v + 2] = *(Vertex*)(&p2);

				sf::Vector3f color = boid.get_color();

				colors[v	] = *(Color*)(&color);
				colors[v + 1] = *(Color*)(&color);
				colors[v + 2] = *(Color*)(&color);
			});
	}

private:
	std::vector<Vertex> vertices;
	std::vector<Color> colors;
};