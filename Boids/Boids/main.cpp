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
	srand((unsigned int)time(0));

	Config cfg;
	cfg.load();

	sf::Window window(sf::VideoMode(
		sf::VideoMode::getDesktopMode().width,
		sf::VideoMode::getDesktopMode().height), "Boids"); //sf::Style::Fullscreen);

	window.setVerticalSyncEnabled(cfg.vertical_sync);
	window.setFramerateLimit(cfg.max_framerate);
	window.setActive(true);

	Camera camera(window);
	InputHandler inputHandler;

	sf::Clock clock;
	float deltaTime = FLT_EPSILON;;

	Rect_i border(0, 0, window.getSize().x, window.getSize().y);
	size_t vertex_count = cfg.boid_count * 3;
	const int extra_grid_cells = 24;

	Boid* boids = (Boid*)malloc(cfg.boid_count * sizeof(Boid));
	Vertex* vertices = (Vertex*)malloc(vertex_count * sizeof(Vertex));
	Color* colors = (Color*)malloc(vertex_count * sizeof(Color));

	std::for_each(
		std::execution::par_unseq,
		boids,
		boids + cfg.boid_count,
		[&](Boid& boid)
		{
			int i = &boid - boids;

			sf::Vector2f pos = sf::Vector2f(
				util::random(0, border.width()) - border.left,
				util::random(0, border.height()) - border.top);

			boids[i] = Boid(pos, &cfg);
		});

	Grid* grid = new Grid(
		border.left - cfg.boid_min_distance * extra_grid_cells,
		border.top - cfg.boid_min_distance * extra_grid_cells,
		border.right + cfg.boid_min_distance * extra_grid_cells,
		border.bot + cfg.boid_min_distance * extra_grid_cells,
		cfg.boid_min_distance, cfg.boid_min_distance);

	glClearColor(
		cfg.background.x, 
		cfg.background.y, 
		cfg.background.z, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

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

						border = Rect_i(
							0,
							0,
							window.getSize().x,
							window.getSize().y);

						camera.set_position((sf::Vector2f)window.getSize() / 2.0f);

						std::for_each(
							std::execution::par_unseq,
							boids,
							boids + cfg.boid_count,
							[&](Boid& boid) 
							{
								boid.set_container(nullptr); 
							});

						delete grid;
						grid = new Grid(
							border.left  - cfg.boid_min_distance * extra_grid_cells,
							border.top   - cfg.boid_min_distance * extra_grid_cells,
							border.right + cfg.boid_min_distance * extra_grid_cells,
							border.bot   + cfg.boid_min_distance * extra_grid_cells,
							cfg.boid_min_distance, cfg.boid_min_distance);
					}
					break;
				case sf::Event::MouseWheelScrolled:
					inputHandler.set_scrollDelta(event.mouseWheelScroll.delta);
					break;
			}
		}

		camera.update(inputHandler);
		sf::Vector2f mousePos = (sf::Vector2f)camera.get_mouse_world_position();

		std::for_each(
			std::execution::seq,
			boids,
			boids + cfg.boid_count,
			[&](Boid& boid)
			{
				grid->insert(boid);
			});

		std::for_each(
			std::execution::par_unseq,
			boids,
			boids + cfg.boid_count,
			[&](Boid& boid)
			{
				if (cfg.cursor_enabled)
				{
					if (inputHandler.get_left_held())
						boid.steer_towards(mousePos, cfg.cursor_towards);
					if (inputHandler.get_right_held())
						boid.steer_away(mousePos, cfg.cursor_away);
				}

				boid.update(deltaTime, border);

				int v = (&boid - boids) * 3;

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

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertices);
		glColorPointer(3, GL_FLOAT, sizeof(Color), colors);

		glPushMatrix();

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.get_world_matrix());

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		glPopMatrix();

		window.display();
	}

	delete[] boids;
	delete[] vertices;
	delete[] colors;
	delete grid;

	return 0;
}