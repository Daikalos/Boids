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

	//////////////////////

private: // VARIABLES
	float	_held_threshold{0.0f};

	float	_scroll_delta{0.0f};

	bool	_current_button_state	[sf::Mouse::ButtonCount] = {false};
	bool	_previous_button_state	[sf::Mouse::ButtonCount] = {false};
	float	_button_held_timer		[sf::Mouse::ButtonCount] = {0.0f};

	bool	_current_key_state		[sf::Keyboard::KeyCount] = {false};
	bool	_previous_key_state		[sf::Keyboard::KeyCount] = {false};
	float	_key_held_timer			[sf::Keyboard::KeyCount] = {0.0f};
};

