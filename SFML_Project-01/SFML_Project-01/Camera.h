#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

class Camera
{
public:
	Camera() = delete;
	Camera(const sf::Window& window);

	void poll_event(const sf::Event& event);

private:
	void key_pressed(const sf::Event& event);

	void mouse_moved(const sf::Event& event);
	void mouse_wheel_scrolled(const sf::Event& event);
	void mouse_button_pressed(const sf::Event& event);
	void mouse_button_released(const sf::Event& event);

public:
	inline double get_scale() { return scale; }
	inline sf::Vector2i get_position() { return position; }

private:
	const sf::Window& window;

	// Camera
	sf::Vector2i position;
	bool moveCamera;
	double scale;

	// Mouse
	sf::Mouse mouse;
	sf::Vector2i mousePos;
	sf::Vector2i mouseOldPos;
};

