#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>

#include <stdint.h>

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
class InputHandler
{
public:
	InputHandler();

public:
	bool GetButtonHeld(sf::Mouse::Button button) const;
	bool GetButtonPressed(sf::Mouse::Button button) const;
	bool GetButtonReleased(sf::Mouse::Button button) const;

	bool GetScrollUp() const;
	bool GetScrollDown() const;

	bool GetKeyHeld(sf::Keyboard::Key key) const;
	bool GetKeyPressed(sf::Keyboard::Key key) const;
	bool GetKeyReleased(sf::Keyboard::Key key) const;

public:
	// call at start of loop before poll event
	//
	void Update(float dt);
	void HandleEvent(const sf::Event& event);

private: // VARIABLES
	float	m_scrollDelta		{0.0f};
	float	m_heldThreshold		{0.0f};

	bool	m_currButtonState	[sf::Mouse::ButtonCount] = {false};
	bool	m_prevButtonState	[sf::Mouse::ButtonCount] = {false};
	float	m_heldButtonTime	[sf::Mouse::ButtonCount] = {0.0f};

	bool	m_currKeyState		[sf::Keyboard::KeyCount] = {false};
	bool	m_prevKeyState		[sf::Keyboard::KeyCount] = {false};
	float	m_heldKeyTime		[sf::Keyboard::KeyCount] = {0.0f};
};

