#include <iostream>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

#include "Boid.h"

#define MAX_BOIDS_X 1000
#define MAX_BOIDS_Y 1000

#define BOID_COUNT MAX_BOIDS_X * MAX_BOIDS_Y

#define BOID_CHUNK BOID_COUNT / 4

void Update(sf::Window* window, Boid* boids, sf::Vector2f* mousePos, int* force, int index)
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
				boids[i].ApplyForce(Normalize(Direction(*mousePos, boids[i].GetPosition())) * 100.0f * (float)(*force));

				boids[i].Update(window, deltaTime);
			}

			accumulator -= deltaTime;
		}
	}
}

int main()
{
	sf::Window window(sf::VideoMode(1600, 900), "boids");

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

	Boid* boids = new Boid[BOID_COUNT];
	Triangle* triangles = new Triangle[BOID_COUNT];

	float size = BOID_COUNT;
	for (int x = 0; x < MAX_BOIDS_X; x++)
	{
		for (int y = 0; y < MAX_BOIDS_Y; y++)
		{
			int i = y * MAX_BOIDS_X + x;

			sf::Vector2f pos = sf::Vector2f((float)x, (float)y);
			sf::Vector3f color = sf::Vector3f(0.1f + ((float)(x * y) / size), 0.0f, 0.0f);

			boids[i] = Boid(pos, color);
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

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(1.0f);

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

		glClear(GL_COLOR_BUFFER_BIT);

		glPushMatrix();

		glTranslatef(cameraPositionX, cameraPositionY, 0);
		glScalef(cameraScale, cameraScale, 1.0f);



		glPopMatrix();

		window.display();
	}

	return 0;
}