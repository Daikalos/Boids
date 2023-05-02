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
	InputHandler();

public:
	bool GetButtonHeld(const sf::Mouse::Button& button) const;
	bool GetButtonPressed(const sf::Mouse::Button& button) const;
	bool GetButtonReleased(const sf::Mouse::Button& button) const;

	bool GetScrollUp() const;
	bool GetScrollDown() const;

	bool GetKeyHeld(const sf::Keyboard::Key& key) const;
	bool GetKeyPressed(const sf::Keyboard::Key& key) const;
	bool GetKeyReleased(const sf::Keyboard::Key& key) const;

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

