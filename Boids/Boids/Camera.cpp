#include "Camera.h"

Camera::Camera(const sf::Window& window) : window(window)
{
	position = (sf::Vector2f)window.getSize() / 2.0f;
	moveCamera = false;
	scale = 1.0f;
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

sf::Vector2f Camera::view_to_world(const sf::Vector2f& position) const
{
	return view_matrix() * position;
}

void Camera::key_pressed(const sf::Event& event)
{
	if (event.key.code == sf::Keyboard::Space)
	{
		position = (sf::Vector2f)window.getSize() / 2.0f;
		scale = 1.0f;
		moveCamera = false;
	}
}

void Camera::mouse_moved(const sf::Event& event)
{
	sf::Vector2f deltaDragPos = view_to_world((sf::Vector2f)mouse.getPosition());

	if (moveCamera)
		position += (dragPos - deltaDragPos);
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
		dragPos = view_to_world((sf::Vector2f)mouse.getPosition());
	}
}
void Camera::mouse_button_released(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		moveCamera = false;
	}
}
