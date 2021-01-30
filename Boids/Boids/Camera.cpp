#include "Camera.h"

Camera::Camera(const sf::Window& window) : window(window)
{
	position = sf::Vector2i(0, 0);
	moveCamera = false;
	scale = 1.0;
}

void Camera::poll_event(const sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed: 
			key_pressed(event);
			break;
		case sf::Event::MouseMoved: 
			mouse_moved(event);
			break;
		case sf::Event::MouseWheelScrolled: 
			mouse_wheel_scrolled(event);
			break;
		case sf::Event::MouseButtonPressed: 
			mouse_button_pressed(event);
			break;
		case sf::Event::MouseButtonReleased: 
			mouse_button_released(event);
			break;
	}
}

void Camera::key_pressed(const sf::Event& event)
{
	if (event.key.code == sf::Keyboard::Space)
	{
		position = sf::Vector2i(0, 0);
		scale = 1.0;
	}
}

void Camera::mouse_moved(const sf::Event& event)
{
	mousePos = sf::Vector2i(
		(int)((mouse.getPosition(window).x - (double)position.x) / scale),
		(int)((mouse.getPosition(window).y - (double)position.y) / scale));

	if (moveCamera)
	{
		const sf::Vector2i mouseNewPos = mouse.getPosition(window);
		const sf::Vector2i deltaPos = mouseNewPos - mouseOldPos;

		position.x += deltaPos.x;
		position.y += deltaPos.y;

		mouseOldPos = mouseNewPos;
	}
}
void Camera::mouse_wheel_scrolled(const sf::Event& event)
{
	scale *= (event.mouseWheelScroll.delta == 1) ? 1.15f : 0.85f;
}
void Camera::mouse_button_pressed(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		moveCamera = true;
		mouseOldPos = mouse.getPosition(window);
	}
}
void Camera::mouse_button_released(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		moveCamera = false;
	}
}
