#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Boid.h"
#include "Quadtree.h"
#include "Camera.h"

const size_t THREAD_COUNT = 6;

const size_t BOID_COUNT = 3900;

const size_t BOID_CHUNK = BOID_COUNT / THREAD_COUNT;
const size_t VERTEX_COUNT = BOID_COUNT * 3;

struct Vertex
{
	GLdouble x, y;
};

struct Color
{
	GLdouble r, g, b;
};

int Update(sf::Window* window, Boid* boids, size_t index)
{
	sf::Clock clock;

	Quad* quadtree = nullptr;

	double deltaTime = FLT_EPSILON;

	while (window->isOpen())
	{
		deltaTime = clock.restart().asSeconds();

		delete quadtree;
		quadtree = new Quad(sf::Vector2i(0, 0), sf::Vector2i(window->getSize().x, window->getSize().y), 16);

		for (long long i = 0; i < BOID_COUNT; ++i)
		{
			(*quadtree).Insert(boids[i]);
		}

		for (size_t i = (index * BOID_CHUNK); i < ((index + 1) * BOID_CHUNK); ++i)
		{
			Boid& boid = boids[i];

			const sf::Vector2<double> boidOri = boid.GetOrigin();
			const double boidMinDistance = boid.GetMinDistance();

			const std::vector<Boid> nearBoids = (*quadtree).Query(
				sf::Vector2i((int)(boidOri.x - boidMinDistance), (int)(boidOri.y - boidMinDistance)), 
				sf::Vector2i((int)(boidOri.x + boidMinDistance), (int)(boidOri.y + boidMinDistance)));

			boid.Update(window, deltaTime, nearBoids);
		}
	}

	return 0;
}

int main()
{
	srand((unsigned int)time(0));

	sf::Window window(sf::VideoMode(1600, 900), "Boids");

	window.setFramerateLimit(144);
	window.setActive(true);

	sf::Clock clock;
	float deltaTime = FLT_EPSILON;

	Camera camera(window);

	Boid* boids = new Boid[BOID_COUNT];
	Vertex* vertices = new Vertex[VERTEX_COUNT];
	Color* colors = new Color[VERTEX_COUNT];

	for (int i = 0; i < BOID_COUNT; i++)
	{
		sf::Vector2<double> pos = sf::Vector2<double>(
			(double)(rand() % window.getSize().x),
			(double)(rand() % window.getSize().y));
		sf::Vector2<double> size = sf::Vector2<double>(6.0, 3.0);
		sf::Vector3<double> color = sf::Vector3<double>(1.0, 0.0, 0.0);

		boids[i] = Boid(pos, size, color, 150.0, 1.5, 30.0, 260.0);
	}

	std::vector<sf::Thread*> threads;
	for (size_t i = 0; i < THREAD_COUNT; ++i)
	{
		threads.push_back(new sf::Thread(std::bind(&Update, &window, boids, i)));
	}

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
			camera.Update(event);

			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					glViewport(0, 0, event.size.width, event.size.height);
					break;
			}
		}

		int v = 0;
		for (int i = 0; i < BOID_COUNT; ++i)
		{
			const Boid boid = boids[i];

			const sf::Vector2<double> boidPos = boid.GetPosition();
			const sf::Vector2<double> boidSiz = boid.GetSize();
			const sf::Vector2<double> boidOri = boid.GetOrigin();
			const sf::Vector3f boidCol = boid.GetColor();
			const double boidRot = boid.GetRotation();

			sf::Vector2<double> pos0 = Vector2::RotatePoint(sf::Vector2<double>(
				boidPos.x, 
				boidPos.y + (boidSiz.y / 2)), boidOri, boidRot);
			sf::Vector2<double> pos1 = Vector2::RotatePoint(sf::Vector2<double>(
				boidPos.x + boidSiz.x,
				boidPos.y), boidOri, boidRot);
			sf::Vector2<double> pos2 = Vector2::RotatePoint(sf::Vector2<double>(
				boidPos.x + boidSiz.x,
				boidPos.y + boidSiz.y), boidOri, boidRot);

			vertices[v	  ].x = pos0.x;
			vertices[v	  ].y = pos0.y;
			vertices[v + 1].x = pos1.x;
			vertices[v + 1].y = pos1.y;
			vertices[v + 2].x = pos2.x;
			vertices[v + 2].y = pos2.y;

			double col0 = 0.5f + ((boidOri.x) / window.getSize().x);
			double col1 = (boidOri.x * boidOri.y) / ((long long)window.getSize().x * (long long)window.getSize().y);
			double col2 = 0.5f + ((boidOri.y) / window.getSize().y);

			colors[v    ].r	= col0;
			colors[v    ].g	= col1;
			colors[v    ].b	= col2;
			colors[v + 1].r = col0;
			colors[v + 1].g = col1;
			colors[v + 1].b = col2;
			colors[v + 2].r = col0;
			colors[v + 2].g = col1;
			colors[v + 2].b = col2;

			v += 3;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glVertexPointer(2, GL_DOUBLE, sizeof(Vertex), vertices);
		glColorPointer(3, GL_DOUBLE, sizeof(Color), colors);

		glPushMatrix();

		glTranslated(camera.Position().x, camera.Position().y, 0);
		glScaled(camera.Scale(), camera.Scale(), 1.0f);

		glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

		glPopMatrix();

		window.display();
	}

	return 0;
}