#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "NonCopyable.h"

#define KEYBOARDMOUSE_ENABLED 1
#define JOYSTICK_ENABLED 0

namespace Binding
{
#if KEYBOARDMOUSE_ENABLED
	enum class Button : uint32_t
	{
		Drag,

		ButtonBindingCount
	};

	enum class Key : uint32_t
	{
		Up,
		Left,
		Right,
		Down,

		KeyBindingCount
	};
#endif

#if JOYSTICK_ENABLED
	enum class XboxButton : uint32_t
	{
		A,
		B,
		X,
		Y,
		LB,
		RB,
		Back,
		Start,
		L,
		R,

		XboxButtonCount
	};
	enum class PlaystationButton : uint32_t // not checked
	{
		A,
		B,
		X,
		Y,
		LB,
		RB,
		Back,
		Start,
		L,
		R,

		Playstation
	};

	enum class XboxAxis : uint32_t
	{
		LX,
		LY,

		XboxAxisCount
	};
	enum class PlaystationAxis : uint32_t
	{

	};

	struct JoystickButton
	{
		JoystickButton(const XboxButton& xbox_button)
			: _button(static_cast<uint32_t>(xbox_button)) { }
		JoystickButton(const PlaystationButton& playstation_button)
			: _button(static_cast<uint32_t>(playstation_button)) { }

		bool operator==(const JoystickButton& other) const
		{
			return _button == other._button;
		}

		struct Hash
		{
			uint32_t operator()(const JoystickButton& joystick_button) const
			{
				return joystick_button._button;
			}
		};

		const uint32_t _button;
	};

	struct JoystickAxis
	{
		JoystickAxis(const XboxButton& xbox_axis)
			: _axis(static_cast<uint32_t>(xbox_axis)) { }
		JoystickAxis(const PlaystationButton& playstation_axis)
			: _axis(static_cast<uint32_t>(playstation_axis)) { }

		bool operator==(const JoystickAxis& other) const
		{
			return _axis == other._axis;
		}

		struct Hash
		{
			uint32_t operator()(const JoystickAxis& joystick_axis) const
			{
				return joystick_axis._axis;
			}
		};

		const uint32_t _axis;
	};
#endif
}

// translates basic input from keyboard and mouse for more extensible usage
//
class InputHandler : NonCopyable
{
public:
	using uint = uint32_t;

public:
	InputHandler();
	~InputHandler();

	// call at start of loop before poll event
	//
	void update(float dt);
	void handle_event(const sf::Event& event);

public:
#if KEYBOARDMOUSE_ENABLED
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
#endif

#if JOYSTICK_ENABLED
	inline bool get_joystick_button_held(const uint& id, const uint& button) const
	{ 
		int index = button + id * sf::Joystick::ButtonCount;
		return _current_button_joystick_state[index] && _previous_button_joystick_state[index];
	}
	inline bool get_joystick_button_pressed(const uint& id, const uint& button) const
	{
		int index = button + id * sf::Joystick::ButtonCount;
		return _current_button_joystick_state[index] && !_previous_button_joystick_state[index];
	}
	inline bool get_joystick_button_released(const uint& id, const uint& button) const
	{
		return !get_joystick_button_pressed(id, button);
	}

	inline float get_joystick_axis(const uint& id, const uint& axis) const
	{
		return _joystick_axis[axis + id * sf::Joystick::AxisCount];
	}

	inline bool get_joystick_button_held(const uint& id, const Binding::JoystickButton& name) const
	{ 
		return _joystick_button_bindings.contains(name) && get_joystick_button_held(id, _joystick_button_bindings.at(name));
	}
	inline bool get_joystick_button_pressed(const uint& id, const Binding::JoystickButton& name) const
	{
		return _joystick_button_bindings.contains(name) && get_joystick_button_pressed(id, _joystick_button_bindings.at(name));
	}
	inline bool get_joystick_button_released(const uint& id, const Binding::JoystickButton& name) const
	{
		return _joystick_button_bindings.contains(name) && get_joystick_button_released(id, _joystick_button_bindings.at(name));
	}

	inline float get_joystick_axis(const uint& id, const Binding::JoystickAxis& name) const
	{
		if (!_joystick_axis_bindings.contains(name))
			return 0.0f;

		return get_joystick_axis(id, _joystick_axis_bindings.at(name));
	}

	void set_joystick_button_binding(const Binding::JoystickButton& name, const uint& button)
	{
		_joystick_button_bindings[name] = button;
	}
	void set_joystick_axis_binding(const Binding::JoystickAxis& name, const uint& axis)
	{
		_joystick_axis_bindings[name] = axis;
	}
#endif

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
	bool _keyboard_enabled;
	bool _mouse_enabled;
	bool _joystick_enabled;

	float _held_threshold;

#if KEYBOARDMOUSE_ENABLED
	float _scroll_delta;

	bool _current_button_state[sf::Mouse::ButtonCount];
	bool _previous_button_state[sf::Mouse::ButtonCount];
	float _button_held_timer[sf::Mouse::ButtonCount];

	bool _current_key_state[sf::Keyboard::KeyCount];
	bool _previous_key_state[sf::Keyboard::KeyCount];
	float _key_held_timer[sf::Keyboard::KeyCount];

	std::unordered_map<Binding::Key, sf::Keyboard::Key> _key_bindings;
	std::unordered_map<Binding::Button, sf::Mouse::Button> _button_bindings;
#endif

#if JOYSTICK_ENABLED
	std::unordered_set<uint> _available_joysticks;

	bool _current_button_joystick_state[sf::Joystick::Count * sf::Joystick::ButtonCount];
	bool _previous_button_joystick_state[sf::Joystick::Count * sf::Joystick::ButtonCount];
	float _joystick_button_held_timer[sf::Joystick::Count * sf::Joystick::ButtonCount];

	float _joystick_axis[sf::Joystick::Count * sf::Joystick::AxisCount];

	std::unordered_map<Binding::JoystickButton, uint, Binding::JoystickButton::Hash> _joystick_button_bindings;
	std::unordered_map<Binding::JoystickAxis, uint, Binding::JoystickAxis::Hash> _joystick_axis_bindings;
#endif
};

