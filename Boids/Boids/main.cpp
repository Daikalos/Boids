#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GLU.h>

#include "Boid.h"
#include "Grid.h"
#include "Quadtree.h"
#include "Vector2.h"
#include "Camera.h"

const size_t THREAD_COUNT = 6;

const size_t BOID_COUNT = 5400;

const size_t BOID_CHUNK = BOID_COUNT / THREAD_COUNT;
const size_t VERTEX_COUNT = BOID_COUNT * 3;

struct Vertex
{
	GLfloat x, y;
};

struct Color
{
	GLfloat r, g, b;
};

int update(sf::Window* window, Boid* boids, size_t index)
{
	sf::Clock clock;
	float deltaTime = FLT_EPSILON;

	QuadtreeB* quadtree = nullptr;

	while (window->isOpen())
	{
		delete quadtree;
		quadtree = new QuadtreeB(Rect_i(
			sf::Vector2i(0, 0), 
			sf::Vector2i(window->getSize().x, window->getSize().y)), 16);

		for (size_t i = 0; i < BOID_COUNT; ++i)
			quadtree->insert(boids[i]);

		for (size_t i = (index * BOID_CHUNK); i < ((index + 1) * BOID_CHUNK); ++i)
		{
			Boid& boid = boids[i];

			const sf::Vector2f ori = boid.get_origin();
			const double minDistance = boid.get_min_distance();

			std::vector<Boid> boids = quadtree->query(Rect_i(
				sf::Vector2i((int)(ori.x - minDistance), (int)(ori.y - minDistance)),
				sf::Vector2i((int)(ori.x + minDistance), (int)(ori.y + minDistance))));

			boid.update(window, deltaTime, boids);
		}

		deltaTime = clock.restart().asSeconds();
	}

	return 0;
}

int main()
{
	if (std::fmod(BOID_COUNT, THREAD_COUNT) != 0)
		return -1;

	srand((unsigned int)time(0));

	sf::Window window(sf::VideoMode(2240, 1260), "Boids");

	window.setFramerateLimit(144);
	window.setActive(true);

	Camera camera(window);

	Boid* boids = new Boid[BOID_COUNT];
	Vertex* vertices = new Vertex[VERTEX_COUNT];
	Color* colors = new Color[VERTEX_COUNT];

	for (int i = 0; i < BOID_COUNT; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			(float)(rand() % window.getSize().x),
			(float)(rand() % window.getSize().y));
		sf::Vector2f size = sf::Vector2f(6.0, 3.0);

		boids[i] = Boid(pos, size,
			1.460f, 1.320f, 1.280f, 
			250.0f, 5.0f, 
			40.0f, 280.0f);
	}

	std::vector<sf::Thread*> threads;

	for (size_t i = 0; i < THREAD_COUNT; ++i)
		threads.push_back(new sf::Thread(std::bind(&update, &window, boids, i)));

	std::for_each(threads.begin(), threads.end(),
	[](sf::Thread* thread)
	{
		thread->launch();
	});

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					glViewport(0, 0, event.size.width, event.size.height);
					break;
			}

			camera.poll_event(event);
		}

		int v = 0;
		for (size_t i = 0; i < BOID_COUNT; ++i)
		{
			const Boid& boid = boids[i];

			const sf::Vector2f& pos = boid.get_position();
			const sf::Vector2f& size = boid.get_size();
			const sf::Vector2f& ori = boid.get_origin();
			const sf::Vector3f& col = boid.get_color();
			const float& rot = boid.get_rotation();

			const sf::Vector2f pos0 = v2f::rotate_point(sf::Vector2f(
				pos.x,
				pos.y + (size.y / 2)), ori, rot);
			const sf::Vector2f pos1 = v2f::rotate_point(sf::Vector2f(
				pos.x + size.x,
				pos.y), ori, rot);
			const sf::Vector2f pos2 = v2f::rotate_point(sf::Vector2f(
				pos.x + size.x,
				pos.y + size.y), ori, rot);

			vertices[v    ] = *(Vertex*)(&pos0);
			vertices[v + 1] = *(Vertex*)(&pos1);
			vertices[v + 2] = *(Vertex*)(&pos2);

			const sf::Vector3f color = 
			{
				0.5f + ((ori.x) / window.getSize().x),
				(ori.x * ori.y) / ((long long)window.getSize().x * (long long)window.getSize().y),
				0.5f + ((ori.y) / window.getSize().y)
			};

			colors[v	] = *(Color*)(&color);
			colors[v + 1] = *(Color*)(&color);
			colors[v + 2] = *(Color*)(&color);

			v += 3;
		}
		
		glClear(GL_COLOR_BUFFER_BIT);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertices);
		glColorPointer(3, GL_FLOAT, sizeof(Color), colors);

		glPushMatrix();

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.world_matrix());

		glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

		glPopMatrix();

		window.display();
	}

	return 0;
}