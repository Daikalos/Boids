#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Boid.h"
#include "Quadtree.h"

#define BOID_COUNT 4000
#define BOID_CHUNK BOID_COUNT / 4

#define VERTEX_COUNT BOID_COUNT * 3

int Update(sf::Window* window, std::vector<Boid>* boids, int index)
{
	sf::Clock clock;

	Quad quadtree(sf::Vector2i(0, 0), sf::Vector2i(window->getSize().x, window->getSize().y), 16);

	double deltaTime = FLT_EPSILON;

	while (window->isOpen())
	{
		deltaTime = clock.restart().asSeconds();

		quadtree.~Quad();
		quadtree = Quad(sf::Vector2i(0, 0), sf::Vector2i(window->getSize().x, window->getSize().y), 16);

		for (int i = 0; i < BOID_COUNT; ++i)
		{
			quadtree.Insert((*boids)[i]);
		}

		for (int i = (index * BOID_CHUNK); i < ((index + 1) * BOID_CHUNK); ++i)
		{
			const sf::Vector2<double> boidOri = (*boids)[i].GetOrigin();
			const double boidMinDistance = (*boids)[i].GetMinDistance();

			const std::vector<Boid> nearBoids = quadtree.Query(
				sf::Vector2i((int)(boidOri.x - boidMinDistance), (int)(boidOri.y - boidMinDistance)), 
				sf::Vector2i((int)(boidOri.x + boidMinDistance), (int)(boidOri.y + boidMinDistance)));

			(*boids)[i].Update(window, deltaTime, nearBoids);
		}
	}

	return 0;
}

int main()
{
	sf::Window window(sf::VideoMode(1600, 900), "Boids");

	window.setFramerateLimit(60);
	window.setActive(true);

	sf::Mouse mouse;
	sf::Vector2<double> mousePos;
	sf::Vector2i mouseOldPos;

	bool moveCamera = false;
	double cameraPositionX = 0.0f;
	double cameraPositionY = 0.0f;
	double cameraScale = 1.0f;

	std::vector<Boid>* boids = new std::vector<Boid>();

	Vertex* vertices = new Vertex[VERTEX_COUNT];
	Color* colors = new Color[VERTEX_COUNT];

	for (int i = 0; i < BOID_COUNT; i++)
	{
		sf::Vector2<double> pos = sf::Vector2<double>(
			(double)(rand() % window.getSize().x),
			(double)(rand() % window.getSize().y));
		sf::Vector2<double> size = sf::Vector2<double>(6.0, 3.0);
		sf::Vector3<double> color = sf::Vector3<double>(1.0, 0.0, 0.0);

		boids->push_back(Boid(pos, size, color, 200.0, 0.3, 30.0));
	}

	sf::Thread thread00(std::bind(&Update, &window, boids, 0));
	sf::Thread thread01(std::bind(&Update, &window, boids, 1));
	sf::Thread thread02(std::bind(&Update, &window, boids, 2));
	sf::Thread thread03(std::bind(&Update, &window, boids, 3));

	thread00.launch();
	thread01.launch();
	thread02.launch();
	thread03.launch();

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
		for (int i = 0; i < boids->size(); ++i)
		{
			const sf::Vector2<double> boidPos = (*boids)[i].GetPosition();
			const sf::Vector2<double> boidSiz = (*boids)[i].GetSize();
			const sf::Vector2<double> boidOri = (*boids)[i].GetOrigin();
			const sf::Vector3f boidCol = (*boids)[i].GetColor();
			const double boidRot = (*boids)[i].GetRotation();

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