#include "InputHandler.h"

InputHandler::InputHandler()
	: _held_threshold(0.1f) { }

void InputHandler::update(float dt)
{
	_scroll_delta = 0.0f;

	for (uint i = 0; i < sf::Mouse::ButtonCount; ++i)
	{
		_previous_button_state[i] = _current_button_state[i];
		_current_button_state[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

		_button_held_timer[i] = _current_button_state[i] ? 
			_button_held_timer[i] + (_button_held_timer[i] < _held_threshold ? dt : 0.0f) : 0.0f;
	}

	for (uint i = 0; i < sf::Keyboard::KeyCount; ++i)
	{
		_previous_key_state[i] = _current_key_state[i];
		_current_key_state[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));

		_key_held_timer[i] = _current_key_state[i] ? 
			_key_held_timer[i] + (_key_held_timer[i] < _held_threshold ? dt : 0.0f) : 0.0f;
	}
}

void InputHandler::handle_event(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::MouseWheelScrolled:
		_scroll_delta = event.mouseWheelScroll.delta;
		break;
	}
}
