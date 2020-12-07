#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/GL.h>
#include <gl/GLU.h>

class Camera
{
public:
	Camera() = delete;
	Camera(const sf::Window& window);

	void Update(const sf::Event& event);

private:
	void KeyPressed(const sf::Event& event);

	void MouseMoved(const sf::Event& event);
	void MouseWheelScrolled(const sf::Event& event);
	void MouseButtonPressed(const sf::Event& event);
	void MouseButtonReleased(const sf::Event& event);

public:
	inline double Scale() { return m_Scale; }
	inline sf::Vector2i Position() { return m_Position; }

private:
	const sf::Window& m_Window;

	// Camera
	bool m_MoveCamera;
	double m_Scale;
	sf::Vector2i m_Position;

	// Mouse
	sf::Mouse mouse;
	sf::Vector2i m_MousePos;
	sf::Vector2i m_MouseOldPos;
};

