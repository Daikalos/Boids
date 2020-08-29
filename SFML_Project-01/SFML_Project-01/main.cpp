#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Boid.h"

#define MAX_BOIDS_X 10
#define MAX_BOIDS_Y 10

#define BOID_COUNT MAX_BOIDS_X * MAX_BOIDS_Y

#define BOID_CHUNK BOID_COUNT / 4

#define VERTEX_COUNT BOID_COUNT * 3

void Update(sf::Window* window, std::vector<Boid>* boids, sf::Vector2f* mousePos, int* force, int index)
{
	sf::Clock clock;

	float deltaTime = (1.0f / 60.0f);
	float accumulator = 0.0f;

	while (window->isOpen())
	{
		float newTime = clock.restart().asSeconds();
		accumulator += newTime;

		while (accumulator >= deltaTime)
		{
			for (int i = (index * BOID_CHUNK); i < ((index + 1) * BOID_CHUNK); i++)
			{
				(*boids)[i].Update(window, deltaTime, boids);
			}

			accumulator -= deltaTime;
		}
	}
}

int main()
{
	sf::Window window(sf::VideoMode(1600, 900), "Boids");

	window.setFramerateLimit(60);
	window.setActive(true);

	sf::Mouse mouse;
	sf::Vector2f mousePos;
	sf::Vector2i mouseOldPos;

	int force = 0;

	bool moveCamera = false;
	float cameraPositionX = 0.0f;
	float cameraPositionY = 0.0f;
	float cameraScale = 1.0f;

	std::vector<Boid>* boids = new std::vector<Boid>();
	Vertex* vertices = new Vertex[VERTEX_COUNT];
	Color* colors = new Color[VERTEX_COUNT];

	float count = BOID_COUNT;
	for (int x = 0; x < MAX_BOIDS_X; x++)
	{
		for (int y = 0; y < MAX_BOIDS_Y; y++)
		{
			int i = y * MAX_BOIDS_X + x;

			sf::Vector2f size = sf::Vector2f(6.0f, 3.0f);
			sf::Vector2f pos = sf::Vector2f((float)(x * size.x) + 120.f, (float)(y * size.y) + 120.f);
			sf::Vector3f color = sf::Vector3f(0.1f + ((float)(x * y) / count), 0.0f, 0.0f);
			float maxSpeed = 10.0f;
			float minDistance = 100.0f;

			boids->push_back(Boid(pos, size, color, maxSpeed, minDistance));
		}
	}

	sf::Thread thread00(std::bind(&Update, &window, boids, &mousePos, &force, 0));
	sf::Thread thread01(std::bind(&Update, &window, boids, &mousePos, &force, 1));
	sf::Thread thread02(std::bind(&Update, &window, boids, &mousePos, &force, 2));
	sf::Thread thread03(std::bind(&Update, &window, boids, &mousePos, &force, 3));

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
					if (event.mouseButton.button == sf::Mouse::Left)
					{
						force = 1;
					}
					if (event.mouseButton.button == sf::Mouse::Right)
					{
						force = -1;
					}
					break;
				case sf::Event::MouseButtonReleased:
					if (event.mouseButton.button == sf::Mouse::Middle)
					{
						moveCamera = false;
					}
					if (event.mouseButton.button == sf::Mouse::Left || event.mouseButton.button == sf::Mouse::Right)
					{
						force = 0;
					}
					break;
				case sf::Event::MouseMoved:
					mousePos = sf::Vector2f(
						(float)mouse.getPosition(window).x - cameraPositionX, 
						(float)mouse.getPosition(window).y - cameraPositionY) / cameraScale;

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
		for (int i = 0; i < boids->size(); i++)
		{
			const sf::Vector2f boidPos = (*boids)[i].GetPosition();
			const sf::Vector2f boidSiz = (*boids)[i].GetSize();
			const sf::Vector3f boidCol = (*boids)[i].GetColor();
			const sf::Vector2f boidOri = (*boids)[i].GetOrigin();
			const float boidRot = (*boids)[i].GetRotation();

			sf::Vector2f pos0 = RotatePoint(sf::Vector2f(
				boidPos.x, 
				boidPos.y + (boidSiz.y / 2)), boidOri, boidRot);
			sf::Vector2f pos1 = RotatePoint(sf::Vector2f(
				boidPos.x + boidSiz.x,
				boidPos.y), boidOri, boidRot);
			sf::Vector2f pos2 = RotatePoint(sf::Vector2f(
				boidPos.x + boidSiz.x,
				boidPos.y + boidSiz.y), boidOri, boidRot);

			vertices[v	  ].x = pos0.x;
			vertices[v	  ].y = pos0.y;
			vertices[v + 1].x = pos1.x;
			vertices[v + 1].y = pos1.y;
			vertices[v + 2].x = pos2.x;
			vertices[v + 2].y = pos2.y;

			colors[v    ].r	= boidCol.x;
			colors[v    ].g	= boidCol.y;
			colors[v    ].b	= boidCol.z;
			colors[v + 1].r = boidCol.x;
			colors[v + 1].g = boidCol.y;
			colors[v + 1].b = boidCol.z;
			colors[v + 2].r = boidCol.x;
			colors[v + 2].g = boidCol.y;
			colors[v + 2].b = boidCol.z;

			v += 3;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vertices);
		glColorPointer(3, GL_FLOAT, sizeof(Color), colors);

		glPushMatrix();

		glTranslatef(cameraPositionX, cameraPositionY, 0);
		glScalef(cameraScale, cameraScale, 1.0f);

		glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

		glPopMatrix();

		window.display();
	}

	return 0;
}