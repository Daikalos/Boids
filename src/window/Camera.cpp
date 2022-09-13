#include "Camera.h"

void Camera::update(const InputHandler& input_handler, const sf::RenderWindow& window)
{
	if (input_handler.get_button_pressed(sf::Mouse::Button::Middle))
		_drag_pos = get_mouse_world_position(window);
	if (input_handler.get_button_held(sf::Mouse::Button::Middle))
		_position += (_drag_pos - get_mouse_world_position(window));

	set_position(_position);
}

void Camera::handle_event(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Resized:
		set_size(sf::Vector2f(event.size.width, event.size.height));
		set_position(get_size() / 2.0f);
		break;
	}
}
