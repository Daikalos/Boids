#include "Camera.h"

void Camera::update(const InputHandler& input_handler, const sf::RenderWindow& window)
{
	if (!config->camera_enabled)
		return;

	if (input_handler.get_button_pressed(Binding::Button::Drag))
		drag_pos = get_mouse_world_position(window);
	if (input_handler.get_button_held(Binding::Button::Drag))
		position += (drag_pos - get_mouse_world_position(window));

	set_position(position);
}

void Camera::handle_event(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Resized:
		set_size(sf::Vector2f(event.size.width, event.size.height));
		break;
	}
}
