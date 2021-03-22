#include "Camera.h"

Camera::Camera(const sf::Window& window) : window(window)
{
	position = (sf::Vector2f)window.getSize() / 2.0f;
	moveCamera = false;
	leftHold = false;
	rightHold = false;
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

template<typename T>
sf::Vector2<T> Camera::view_to_world(const sf::Vector2<T>& position) const
{
	return (sf::Vector2<T>)(view_matrix() * (sf::Vector2f)position);
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
	sf::Vector2i deltaDragPos = get_mouse_world_position();

	if (moveCamera)
		position += (sf::Vector2f)(dragPos - deltaDragPos);
}
void Camera::mouse_wheel_scrolled(const sf::Event& event)
{
	if (event.mouseWheelScroll.delta < 0)
		scale *= 0.85f;
	if (event.mouseWheelScroll.delta > 0)
		scale *= 1.15f;
}
void Camera::mouse_button_pressed(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Left)
		leftHold = true;
	if (event.mouseButton.button == sf::Mouse::Right)
		rightHold = true;

	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		moveCamera = true;
		dragPos = view_to_world(mouse.getPosition(window));
	}
}
void Camera::mouse_button_released(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Left)
		leftHold = false;
	if (event.mouseButton.button == sf::Mouse::Right)
		rightHold = false;

	if (event.mouseButton.button == sf::Mouse::Middle)
		moveCamera = false;
}
