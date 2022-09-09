#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "../utilities/NonCopyable.h"

namespace Binding
{
	enum class Button : uint32_t
	{
		Drag,

		ButtonBindingCount
	};

	enum class Key : uint32_t
	{
		None,

		Count
	};
}

// translates basic input from keyboard and mouse for more extensible usage
//
class InputHandler : NonCopyable
{
public:
	using uint = uint32_t;

public:
	InputHandler();

	// call at start of loop before poll event
	//
	void update(float dt);
	void handle_event(const sf::Event& event);

public:
	inline bool get_button_held(const sf::Mouse::Button& button) const
	{
		return _current_button_state[button] && _button_held_timer[button] >= _held_threshold;
	}
	inline bool get_button_pressed(const sf::Mouse::Button& button) const
	{
		return _current_button_state[button] && !_previous_button_state[button];
	}
	inline bool get_button_released(const sf::Mouse::Button& button) const
	{
		return !get_button_pressed(button);
	}

	inline bool get_scroll_up() const { return _scroll_delta > 0; }
	inline bool get_scroll_down() const { return _scroll_delta < 0; }

	inline bool get_key_held(const sf::Keyboard::Key& key) const
	{
		return _current_key_state[key] && _key_held_timer[key] >= _held_threshold;
	}
	inline bool get_key_pressed(const sf::Keyboard::Key& key) const
	{
		return _current_key_state[key] && !_previous_key_state[key];
	}
	inline bool get_key_released(const sf::Keyboard::Key& key) const
	{
		return !get_key_pressed(key);
	}

	inline bool get_button_held(const Binding::Button& bind) const { return _button_bindings.contains(bind) && get_button_held(_button_bindings.at(bind)); }
	inline bool get_button_pressed(const Binding::Button& bind) const { return _button_bindings.contains(bind) && get_button_pressed(_button_bindings.at(bind)); }
	inline bool get_button_released(const Binding::Button& bind) const { return _button_bindings.contains(bind) && get_button_released(_button_bindings.at(bind)); }

	inline bool get_key_held(const Binding::Key& bind) const { return _key_bindings.contains(bind) && get_key_held(_key_bindings.at(bind)); }
	inline bool get_key_pressed(const Binding::Key& bind) const { return _key_bindings.contains(bind) && get_key_pressed(_key_bindings.at(bind)); }
	inline bool get_key_released(const Binding::Key& bind) const { return _key_bindings.contains(bind) && get_key_released(_key_bindings.at(bind)); }

	void set_key_binding(const Binding::Key& name, const sf::Keyboard::Key& key)
	{
		_key_bindings[name] = key;
	}
	void set_button_binding(const Binding::Button& name, const sf::Mouse::Button& button)
	{
		_button_bindings[name] = button;
	}

	//////////////////////

	void set_keyboard_enabled(bool flag)
	{
		_keyboard_enabled = flag;
	}
	void set_mouse_enabled(bool flag)
	{
		_mouse_enabled = flag;
	}
	void set_joystick_enabled(bool flag)
	{
		_joystick_enabled = flag;
	}

private: // VARIABLES
	bool	_keyboard_enabled;
	bool	_mouse_enabled;
	bool	_joystick_enabled;

	float	_held_threshold;

	float	_scroll_delta;

	bool	_current_button_state[sf::Mouse::ButtonCount];
	bool	_previous_button_state[sf::Mouse::ButtonCount];
	float	_button_held_timer[sf::Mouse::ButtonCount];

	bool	_current_key_state[sf::Keyboard::KeyCount];
	bool	_previous_key_state[sf::Keyboard::KeyCount];
	float	_key_held_timer[sf::Keyboard::KeyCount];

	std::unordered_map<Binding::Key, sf::Keyboard::Key> _key_bindings;
	std::unordered_map<Binding::Button, sf::Mouse::Button> _button_bindings;
};

