#include "Camera.h"

Camera::Camera(const sf::Window* window, Config* config) 
	: window(window), config(config), position(sf::Vector2f(window->getSize()) / 2.0f), scale({ config->camera_zoom, config->camera_zoom })
{

}

void Camera::update(const InputHandler& inputHandler)
{
	if (!config->camera_enabled)
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
