#include "InputHandler.h"

InputHandler::InputHandler() : m_heldThreshold(0.1f) { }

bool InputHandler::GetButtonHeld(const sf::Mouse::Button& button) const
{
	return m_currButtonState[button] && m_heldButtonTime[button] >= m_heldThreshold;
}
bool InputHandler::GetButtonPressed(const sf::Mouse::Button& button) const
{
	return m_currButtonState[button] && !m_prevButtonState[button];
}
bool InputHandler::GetButtonReleased(const sf::Mouse::Button& button) const
{
	return !m_currButtonState[button] && m_prevButtonState[button];
}

bool InputHandler::GetScrollUp() const { return m_scrollDelta > 0; }
bool InputHandler::GetScrollDown() const { return m_scrollDelta < 0; }

bool InputHandler::GetKeyHeld(const sf::Keyboard::Key& key) const
{
	return m_currKeyState[key] && m_heldKeyTime[key] >= m_heldThreshold;
}
bool InputHandler::GetKeyPressed(const sf::Keyboard::Key& key) const
{
	return m_currKeyState[key] && !m_prevKeyState[key];
}
bool InputHandler::GetKeyReleased(const sf::Keyboard::Key& key) const
{
	return !m_currKeyState[key] && m_prevKeyState[key];
}

void InputHandler::Update(float dt)
{
	m_scrollDelta = 0.0f;

	for (uint32_t i = 0; i < sf::Mouse::ButtonCount; ++i)
	{
		bool& prevState = m_prevButtonState[i];
		bool& currState = m_currButtonState[i];
		float& heldTime = m_heldButtonTime[i];

		prevState = currState;
		currState = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

		heldTime = currState ? (heldTime + dt) : 0.0f;
	}

	for (uint32_t i = 0; i < sf::Keyboard::KeyCount; ++i)
	{
		bool& prevState = m_prevKeyState[i];
		bool& currState = m_currKeyState[i];
		float& heldTime = m_heldKeyTime[i];

		prevState = currState;
		currState = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));

		heldTime = currState ? (heldTime + dt) : 0.0f;
	}
}

void InputHandler::HandleEvent(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::MouseWheelScrolled:
		m_scrollDelta = event.mouseWheelScroll.delta;
		break;
	}
}
