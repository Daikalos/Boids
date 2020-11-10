#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Boid.h"
#include "Quadtree.h"

const size_t BOID_COUNT = 4000;

const size_t BOID_CHUNK = BOID_COUNT / 6;
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
	sf::Window window(sf::VideoMode(1600, 900), "Boids");

	window.setFramerateLimit(144);
	window.setActive(true);

	sf::Clock clock;
	float deltaTime = FLT_EPSILON;

	sf::Mouse mouse;
	sf::Vector2<double> mousePos;
	sf::Vector2i mouseOldPos;

	bool moveCamera = false;
	double cameraPositionX = 0.0f;
	double cameraPositionY = 0.0f;
	double cameraScale = 1.0f;

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

		boids[i] = Boid(pos, size, color, 200.0, 0.09, 30.0);
	}

	sf::Thread thread00(std::bind(&Update, &window, boids, 0));
	sf::Thread thread01(std::bind(&Update, &window, boids, 1));
	sf::Thread thread02(std::bind(&Update, &window, boids, 2));
	sf::Thread thread03(std::bind(&Update, &window, boids, 3));
	sf::Thread thread04(std::bind(&Update, &window, boids, 4));
	sf::Thread thread05(std::bind(&Update, &window, boids, 5));

	thread00.launch();
	thread01.launch();
	thread02.launch();
	thread03.launch();
	thread04.launch();
	thread05.launch();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	gluOrtho2D(0, window.getSize().x, 0, window.getSize().y);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	float k = 0;
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
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Space)
					{
						cameraPositionX = 0.0f;
						cameraPositionY = 0.0f;
						cameraScale = 1.0f;
					}
					break;
				case sf::Event::MouseWheelScrolled:
					cameraScale *= (event.mouseWheelScroll.delta == 1) ? 1.15f : 0.85f;
					break;
				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Middle)
					{
						moveCamera = true;
						mouseOldPos = mouse.getPosition(window);
					}
					break;
				case sf::Event::MouseButtonReleased:
					if (event.mouseButton.button == sf::Mouse::Middle)
					{
						moveCamera = false;
					}
					break;
				case sf::Event::MouseMoved:
					mousePos = sf::Vector2<double>(
						(double)mouse.getPosition(window).x - cameraPositionX, 
						(double)mouse.getPosition(window).y - cameraPositionY) / cameraScale;

					if (moveCamera)
					{
						const sf::Vector2i mouseNewPos = mouse.getPosition(window);
						const sf::Vector2i deltaPos = mouseNewPos - mouseOldPos;

						cameraPositionX += deltaPos.x;
						cameraPositionY += deltaPos.y;

						mouseOldPos = mouseNewPos;
					}
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

			sf::Vector2<double> pos0 = RotatePoint(sf::Vector2<double>(
				boidPos.x, 
				boidPos.y + (boidSiz.y / 2)), boidOri, boidRot);
			sf::Vector2<double> pos1 = RotatePoint(sf::Vector2<double>(
				boidPos.x + boidSiz.x,
				boidPos.y), boidOri, boidRot);
			sf::Vector2<double> pos2 = RotatePoint(sf::Vector2<double>(
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

		glTranslated(cameraPositionX, cameraPositionY, 0);
		glScaled(cameraScale, cameraScale, 1.0f);

		glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

		glPopMatrix();

		window.display();
	}

	return 0;
}