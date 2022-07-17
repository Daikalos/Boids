#include <iostream>
#include <execution>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GLU.h>

#include "Boid.h"
#include "Grid.h"
#include "VecUtil.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Config.h"

struct Vertex
{
	GLfloat x, y;
};

struct Color
{
	GLfloat r, g, b;
};

int main()
{
	Config::load();

	sf::Window window(sf::VideoMode(
		sf::VideoMode::getDesktopMode().width,
		sf::VideoMode::getDesktopMode().height), "Boids");//, sf::Style::Fullscreen);

	window.setVerticalSyncEnabled(Config::vertical_sync);
	window.setFramerateLimit(Config::max_framerate);
	window.setActive(true);

	Camera camera(window);
	InputHandler inputHandler;

	sf::Vector2f mousePos;

	sf::Clock clock;
	float deltaTime = FLT_EPSILON;

	Rect_i border(0, 0, window.getSize().x, window.getSize().y);
	GLsizei vertex_count = Config::boid_count * 3;

	Boid* boids = (Boid*)::operator new(Config::boid_count * sizeof(Boid));
	Vertex* vertices = (Vertex*)::operator new(vertex_count * sizeof(Vertex));
	Color* colors = (Color*)::operator new(vertex_count * sizeof(Color));

	Grid grid(
		border.left	 - Config::boid_min_distance * (Config::grid_extra_cells + 1),
		border.top	 - Config::boid_min_distance * (Config::grid_extra_cells + 1),
		border.right + Config::boid_min_distance * (Config::grid_extra_cells + 1),
		border.bot   + Config::boid_min_distance * (Config::grid_extra_cells + 1),
		Config::boid_min_distance * 2.0f, Config::boid_min_distance * 2.0f);

	std::for_each(
		std::execution::par_unseq,
		boids,
		boids + Config::boid_count,
		[&](Boid& boid)
		{
			__int64 i = &boid - boids;

			sf::Vector2f pos = sf::Vector2f(
				util::random(0, border.width()) - border.left,
				util::random(0, border.height()) - border.top);

			new(boids + i) Boid(&grid, boids, pos);
		});


	glClearColor(
		Config::background.x, 
		Config::background.y, 
		Config::background.z, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);

	while (window.isOpen())
	{
		deltaTime = std::clamp(clock.restart().asSeconds(), 0.0f, 0.075f);

		inputHandler.update();

		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					{
						glViewport(0, 0, window.getSize().x, window.getSize().y);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						glScalef(1.0f, -1.0f, 1.0f);
						gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);
						glMatrixMode(GL_MODELVIEW);

						border = Rect_i(0, 0, window.getSize().x, window.getSize().y);
						camera.set_position((sf::Vector2f)window.getSize() / 2.0f);

						grid = Grid(
							border.left  - Config::boid_min_distance * (Config::grid_extra_cells + 1),
							border.top   - Config::boid_min_distance * (Config::grid_extra_cells + 1),
							border.right + Config::boid_min_distance * (Config::grid_extra_cells + 1),
							border.bot   + Config::boid_min_distance * (Config::grid_extra_cells + 1),
							Config::boid_min_distance * 2.0f, Config::boid_min_distance * 2.0f);
					}
					break;
				case sf::Event::MouseWheelScrolled:
					inputHandler.set_scrollDelta(event.mouseWheelScroll.delta);
					break;
			}
		}

		camera.update(inputHandler);
		mousePos = (sf::Vector2f)camera.get_mouse_world_position();

		grid.reset_buffers();

		std::for_each(std::execution::par_unseq,
			boids,
			boids + Config::boid_count,
			[&](Boid& boid)
			{
				boid.set_index(grid.at_pos(boid.get_origin()));
			});

		std::sort(std::execution::par_unseq, 
			boids, 
			boids + Config::boid_count, 
			[](const Boid& b0, const Boid& b1)
			{
				return b0.get_index() < b1.get_index();
			});

		std::for_each(std::execution::par_unseq,
			boids,
			boids + Config::boid_count,
			[&](const Boid& boid)
			{
				__int64 index = &boid - boids;
				int thisIndex = boid.get_index();

				if (thisIndex < 0)
					return;

				if (index == 0)
				{
					grid.cellsStartIndices[thisIndex] = index;
					return;
				}

				if (index == Config::boid_count - 1)
					grid.cellsEndIndices[thisIndex] = index;

				int otherIndex = boids[index - 1].get_index();

				if (otherIndex < 0)
					return;

				if (otherIndex != thisIndex)
				{
					grid.cellsStartIndices[thisIndex] = index;
					grid.cellsEndIndices[otherIndex] = index - 1;
				}
			});

		std::for_each(
			std::execution::par_unseq,
			boids,
			boids + Config::boid_count,
			[&](Boid& boid)
			{
				if (Config::gravity_enabled)
				{
					if (inputHandler.get_left_held())
						boid.steer_towards(mousePos, Config::gravity_towards_factor);
					if (inputHandler.get_right_held())
						boid.steer_towards(mousePos, -Config::gravity_away_factor);
				}

				if (Config::predator_enabled)
				{
					float dist = v2f::distance(boid.get_origin(), mousePos);

					if (dist <= Config::predator_distance)
					{
						float factor = (dist > FLT_EPSILON) ? (dist / Config::predator_distance) : FLT_MIN;
						boid.steer_towards(mousePos, -Config::predator_factor / factor);
					}
				}

				boid.update(deltaTime, border);

				__int64 v = (&boid - boids) * 3;

				sf::Vector2f p0 = boid.get_pointA();
				sf::Vector2f p1 = boid.get_pointB();
				sf::Vector2f p2 = boid.get_pointC();

				vertices[v	  ] = *(Vertex*)(&p0);
				vertices[v + 1] = *(Vertex*)(&p1);
				vertices[v + 2] = *(Vertex*)(&p2);

				sf::Vector3f color = boid.get_color();

				colors[v    ] = *(Color*)(&color);
				colors[v + 1] = *(Color*)(&color);
				colors[v + 2] = *(Color*)(&color);
			});

		glClear(GL_COLOR_BUFFER_BIT);

		glPushMatrix();

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.get_world_matrix());

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		glPopMatrix();

		window.display();
	}

	return 0;
}