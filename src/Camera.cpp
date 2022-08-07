#include "Camera.h"

Camera::Camera(const sf::RenderWindow& window, Config& config)
	: window(&window), config(&config), position(sf::Vector2f(this->window->getSize()) / 2.0f), scale({ this->config->camera_zoom, this->config->camera_zoom })
{

}

void Camera::update(const InputHandler& input_handler)
{
	if (!config->camera_enabled)
		return;

	if (input_handler.get_key_pressed(sf::Keyboard::Key::Space))
		set_position(sf::Vector2f(window->getSize()) / 2.0f);

	if (input_handler.get_middle_pressed())
		dragPos = get_mouse_world_position();
	if (input_handler.get_middle_held())
		position += (sf::Vector2f)(dragPos - get_mouse_world_position());
}
