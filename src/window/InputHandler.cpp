#include "InputHandler.h"

InputHandler::InputHandler()
	: _mouse_enabled(true), _keyboard_enabled(true), _joystick_enabled(false), _held_threshold(0.1f)
{
#if KEYBOARDMOUSE_ENABLED
	_scroll_delta = 0.0f;

	memset(_current_button_state, false, sizeof(_current_button_state));
	memset(_previous_button_state, false, sizeof(_previous_button_state));

	memset(_current_key_state, false, sizeof(_current_key_state));
	memset(_previous_key_state, false, sizeof(_previous_key_state));
#endif

#if JOYSTICK_ENABLED
	memset(_current_button_joystick_state, false, sizeof(_current_button_joystick_state));
	memset(_previous_button_joystick_state, false, sizeof(_previous_button_joystick_state));

	_available_joysticks.reserve(sf::Joystick::Count);

	for (int i = 0; i < sf::Joystick::Count; ++i) // add already available joysticks
		if (sf::Joystick::isConnected(i))
			_available_joysticks.insert(i);
#endif
}

void InputHandler::update(float dt)
{
#if KEYBOARDMOUSE_ENABLED
	_scroll_delta = 0.0f;

	for (uint i = 0; i < sf::Mouse::ButtonCount; ++i)
	{
		_previous_button_state[i] = _current_button_state[i];
		_current_button_state[i] = _mouse_enabled && sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

		_button_held_timer[i] = _current_button_state[i] ? 
			_button_held_timer[i] + (_button_held_timer[i] < _held_threshold ? dt : 0.0f) : 0.0f;
	}

	for (uint i = 0; i < sf::Keyboard::KeyCount; ++i)
	{
		_previous_key_state[i] = _current_key_state[i];
		_current_key_state[i] = _keyboard_enabled && sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));

		_key_held_timer[i] = _current_key_state[i] ? 
			_key_held_timer[i] + (_key_held_timer[i] < _held_threshold ? dt : 0.0f) : 0.0f;
	}
#endif

#if JOYSTICK_ENABLED
	for (const uint& i : _available_joysticks)
	{
		for (uint j = 0; j < sf::Joystick::getButtonCount(i); ++j)
		{
			uint k = j + i * sf::Joystick::ButtonCount;

			_previous_button_joystick_state[k] = _current_button_joystick_state[k];
			_current_button_joystick_state[k] = _joystick_enabled && sf::Joystick::isButtonPressed(i, j);
		}

		for (uint j = 0; j < sf::Joystick::AxisCount; ++j)
		{
			uint k = j + i * sf::Joystick::AxisCount;
			sf::Joystick::Axis axis = static_cast<sf::Joystick::Axis>(j);

			_joystick_axis[k] = sf::Joystick::hasAxis(i, axis) ? sf::Joystick::getAxisPosition(i, axis) : 0.0f;
		}
	}
#endif
}

void InputHandler::handle_event(const sf::Event& event)
{
	switch (event.type)
	{
#if KEYBOARDMOUSE_ENABLED
	case sf::Event::MouseWheelScrolled:
		_scroll_delta = event.mouseWheelScroll.delta;
		break;
#endif

#if JOYSTICK_ENABLED
	case sf::Event::JoystickConnected:
		_available_joysticks.insert(event.joystickConnect.joystickId);
		break;
	case sf::Event::JoystickDisconnected:
		_available_joysticks.erase(event.joystickConnect.joystickId);
		break;
#endif
	}
}
