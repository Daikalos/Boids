#include "Camera.h"

Camera::Camera(const sf::Window* window) 
	: window(window), position(sf::Vector2f(window->getSize()) / 2.0f), scale({ Config::camera_zoom, Config::camera_zoom })
{

}

void Camera::update(const InputHandler& inputHandler)
{
	if (!Config::camera_enabled)
		return;

	if (inputHandler.get_key_pressed(sf::Keyboard::Key::Space))
	{
		set_position(sf::Vector2f(window->getSize()) / 2.0f);
	}

	if (inputHandler.get_middle_pressed())
		dragPos = get_mouse_world_position();
	if (inputHandler.get_middle_held())
		position += (sf::Vector2f)(dragPos - get_mouse_world_position());
}
