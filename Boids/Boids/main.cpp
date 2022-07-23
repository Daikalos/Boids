#include <iostream>
#include <execution>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Impulse.h"
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

#include "State.h"

int main()
{
	srand(time(NULL));
	Config::load();

	sf::Window window(sf::VideoMode::getDesktopMode(), "Boids", sf::Style::Fullscreen);

	window.setVerticalSyncEnabled(Config::vertical_sync);

	if (!Config::vertical_sync)
		window.setFramerateLimit(Config::max_framerate);
	
	if (!window.setActive(true))
		return -1;

	Rect_i border(0, 0, window.getSize().x, window.getSize().y);

	Camera camera(&window);
	InputHandler inputHandler;

	sf::Clock clock;
	float deltaTime = 1.0f / 80.0f;
	float accumulator = 0.0f;

	sf::Vector2f mouse_pos;
	GLsizei vertex_count = Config::boid_count * 3;

	Boid* boids = (Boid*)::operator new(Config::boid_count * sizeof(Boid));

	Grid grid(
		border.left	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.top	 - Config::min_distance * (Config::grid_extra_cells + 1),
		border.right + Config::min_distance * (Config::grid_extra_cells + 1),
		border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
		Config::min_distance * 2.0f, Config::min_distance * 2.0f);

	State state(vertex_count);

	for (int i = 0; i < Config::boid_count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0, border.width()) - border.left,
			util::random(0, border.height()) - border.top);

		new(boids + i) Boid(&grid, boids, pos);

		state.get_vertices()[i] = *(Vertex*)(&pos);
		state.get_colors()[i] = {};
	}

	glClearColor(
		Config::background.x, 
		Config::background.y, 
		Config::background.z, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	glOrtho(0, window.getSize().x, 0, window.getSize().y, -1.0, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, state.get_vertices());
	glColorPointer(3, GL_FLOAT, 0, state.get_colors());

	while (window.isOpen())
	{
		accumulator += std::fminf(clock.restart().asSeconds(), 0.075f);

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
						glOrtho(0, window.getSize().x, 0, window.getSize().y, -1.0, 1.0);
						glMatrixMode(GL_MODELVIEW);

						border = Rect_i(0, 0, window.getSize().x, window.getSize().y);
						camera.set_position((sf::Vector2f)window.getSize() / 2.0f);

						grid = Grid(
							border.left  - Config::min_distance * (Config::grid_extra_cells + 1),
							border.top   - Config::min_distance * (Config::grid_extra_cells + 1),
							border.right + Config::min_distance * (Config::grid_extra_cells + 1),
							border.bot   + Config::min_distance * (Config::grid_extra_cells + 1),
							Config::min_distance * 2.0f, Config::min_distance * 2.0f);
					}
					break;
			}
		}

		camera.update(inputHandler);
		mouse_pos = sf::Vector2f(camera.get_mouse_world_position());

		if (inputHandler.get_left_pressed())
			Config::impulses.push_back(Impulse(mouse_pos, Config::impulse_speed, Config::impulse_size, 0.0f));

		while (accumulator >= deltaTime)
		{
			for (int i = Config::impulses.size() - 1; i >= 0; --i)
			{
				Impulse& impulse = Config::impulses[i];

				impulse.update(deltaTime);

				if (impulse.get_length() > Config::impulse_fade_distance)
					Config::impulses.erase(Config::impulses.begin() + i);
			}

			grid.reset_buffers();

			std::for_each(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](Boid& boid)
				{
					boid.set_cell_index();
				});

			std::sort(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](const Boid& b0, const Boid& b1)
				{
					return b0.get_cell_index() < b1.get_cell_index();
				});

			std::for_each(std::execution::par_unseq,
				boids,
				boids + Config::boid_count,
				[](const Boid& boid)
				{
					boid.update_grid_cells();
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
							boid.steer_towards(mouse_pos, Config::gravity_towards_factor);
						if (inputHandler.get_right_held())
							boid.steer_towards(mouse_pos, -Config::gravity_away_factor);
					}

					if (Config::predator_enabled)
					{
						float dist = v2f::distance(boid.get_origin(), mouse_pos);

						if (dist <= Config::predator_distance)
						{
							float factor = (dist > FLT_EPSILON) ? (dist / Config::predator_distance) : FLT_EPSILON;
							boid.steer_towards(mouse_pos, -Config::predator_factor / factor);
						}
					}

					boid.update(deltaTime, border);
				});

			accumulator -= deltaTime;
		}

		float interp = accumulator / deltaTime;
		state.draw(boids, interp);

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