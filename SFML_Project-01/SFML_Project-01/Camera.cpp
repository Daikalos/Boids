#include "Camera.h"

Camera::Camera(const sf::Window& window) : m_Window(window)
{
	m_MoveCamera = false;
	m_Position = sf::Vector2i(0, 0);
	m_Scale = 1.0;
}

void Camera::Update(const sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed: 
			KeyPressed(event);
			break;
		case sf::Event::MouseMoved: 
			MouseMoved(event);
			break;
		case sf::Event::MouseWheelScrolled: 
			MouseWheelScrolled(event);
			break;
		case sf::Event::MouseButtonPressed: 
			MouseButtonPressed(event);
			break;
		case sf::Event::MouseButtonReleased: 
			MouseButtonReleased(event);
			break;
	}
}

void Camera::KeyPressed(const sf::Event& event)
{
	if (event.key.code == sf::Keyboard::Space)
	{
		m_Position = sf::Vector2i(0, 0);
		m_Scale = 1.0;
	}
}

void Camera::MouseMoved(const sf::Event& event)
{
	m_MousePos = sf::Vector2i(
		(int)((mouse.getPosition(m_Window).x - (double)m_Position.x) / m_Scale),
		(int)((mouse.getPosition(m_Window).y - (double)m_Position.y) / m_Scale));

	if (m_MoveCamera)
	{
		const sf::Vector2i mouseNewPos = mouse.getPosition(m_Window);
		const sf::Vector2i deltaPos = mouseNewPos - m_MouseOldPos;

		m_Position.x += deltaPos.x;
		m_Position.y += deltaPos.y;

		m_MouseOldPos = mouseNewPos;
	}
}
void Camera::MouseWheelScrolled(const sf::Event& event)
{
	m_Scale *= (event.mouseWheelScroll.delta == 1) ? 1.15f : 0.85f;
}
void Camera::MouseButtonPressed(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		m_MoveCamera = true;
		m_MouseOldPos = mouse.getPosition(m_Window);
	}
}
void Camera::MouseButtonReleased(const sf::Event& event)
{
	if (event.mouseButton.button == sf::Mouse::Middle)
	{
		m_MoveCamera = false;
	}
}
