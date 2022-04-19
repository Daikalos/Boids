#include <iostream>
#include <execution>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GLU.h>

#include "Boid.h"
#include "Grid.h"
#include "Quadtree.h"
#include "VecUtil.h"
#include "Camera.h"
#include "InputHandler.h"

const size_t BOID_COUNT = 20000;
const size_t VERTEX_COUNT = BOID_COUNT * 3;

const short MIN_DISTANCE = 40;

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

	sf::Window window(sf::VideoMode(2240, 1240), "Boids");

	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(144);
	window.setActive(true);

	Camera camera(window);
	InputHandler inputHandler;

	Rect_i border(
		0, 
		0, 
		window.getSize().x + 2400, 
		window.getSize().y + 2400);

	sf::Clock clock;
	float deltaTime = FLT_EPSILON;

	Boid* boids = new Boid[BOID_COUNT];
	Vertex* vertices = new Vertex[VERTEX_COUNT];
	Color* colors = new Color[VERTEX_COUNT];

	QuadtreeB* quadtree = nullptr;
	GridB* grid = new GridB(
		border.left  - MIN_DISTANCE,
		border.top   - MIN_DISTANCE, 
		border.right + MIN_DISTANCE,
		border.bot   + MIN_DISTANCE,
		MIN_DISTANCE, MIN_DISTANCE);

	for (int i = 0; i < BOID_COUNT; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			(float)(rand() % border.width()) - border.left,
			(float)(rand() % border.height() - border.top));
		sf::Vector2f size = sf::Vector2f(6.0, 3.0);

		boids[i] = Boid(pos, size,
			2.200f, 1.050f, 1.550f, 
			400.0f, 3.0f, 
			MIN_DISTANCE, 220.0f);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	while (window.isOpen())
	{
		deltaTime = clock.restart().asSeconds();

		inputHandler.update();

		if (inputHandler.get_key_pressed(sf::Keyboard::Key::O))
			window.setTitle(std::to_string(1.0f / deltaTime));

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

						delete grid;
						grid = new GridB(
							border.left  - MIN_DISTANCE,
							border.top   - MIN_DISTANCE,
							border.right + MIN_DISTANCE,
							border.bot   + MIN_DISTANCE,
							MIN_DISTANCE, MIN_DISTANCE);
					}
					break;
				case sf::Event::MouseWheelScrolled:
					inputHandler.set_scrollDelta(event.mouseWheelScroll.delta);
					break;
			}
		}

		camera.update(inputHandler);
		sf::Vector2f mousePos = (sf::Vector2f)camera.get_mouse_world_position();

		for (size_t i = 0; i < BOID_COUNT; ++i)
			grid->insert(boids[i]);

		std::for_each(
			std::execution::par_unseq,
			boids,
			boids + BOID_COUNT,
			[&](Boid& boid)
			{
				const sf::Vector2f ori = boid.get_origin();
				const float minDistance = boid.get_min_distance();

				std::vector<const Container<Boid>*> cntns = grid->query_containers(ori, minDistance);

				if (inputHandler.get_left_held())
					boid.steer_towards(mousePos, 1.50f);
				if (inputHandler.get_right_held())
					boid.steer_away(mousePos, 1.50f);

				boid.update(deltaTime, border, cntns);

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

		glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

		glPopMatrix();

		window.display();
	}

	return 0;
}