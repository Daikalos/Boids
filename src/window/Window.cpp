#include "Window.h"

#include <SFML/OpenGL.hpp>
#include <SFML/Window/Event.hpp>

#include "../utilities/WindowsUtilities.h"

#include "Camera.h"

Window::Window(std::string name, const sf::VideoMode& mode, WindowBorder windowBorder, bool verticalSync, int frameRate, const sf::ContextSettings& settings)
	: m_name(std::move(name))
	, m_mode(mode)
	, m_border(windowBorder)
	, m_settings(settings)
	, m_verticalSync(verticalSync)
	, m_framerate(frameRate) {}

void Window::Initialize()
{
	Build(m_border, m_mode, m_settings);

#ifdef _WIN32
	HideTaskbarIcon(*this);
#endif

	if (!setActive(true))
		throw std::runtime_error("Window could not be activated.");

	glViewport(0, 0, getSize().x, getSize().y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	glOrtho(0, getSize().x, 0, getSize().y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void Window::HandleEvent(const sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::Closed:
		{
			close();
			break;
		}
		case sf::Event::Resized:
		{
			glViewport(0, 0, getSize().x, getSize().y);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glScalef(1.0f, -1.0f, 1.0f);
			glOrtho(0, getSize().x, 0, getSize().y, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
			break;
		}
	}
}

void Window::Setup()
{
	clear(sf::Color(m_clearColor.r, m_clearColor.g, m_clearColor.b, 0));
}

void Window::SetFramerate(int frameRate)
{
	m_framerate = frameRate;
	setFramerateLimit(m_verticalSync ? 0 : frameRate);
}

void Window::SetVerticalSync(bool flag)
{
	m_verticalSync = flag;

	setFramerateLimit(m_verticalSync ? 0 : m_framerate);
	setVerticalSyncEnabled(m_verticalSync);
}

void Window::Build(WindowBorder windowBorder, const sf::VideoMode& mode, const sf::ContextSettings& settings)
{
	m_border = windowBorder;
	m_mode = mode;
	m_settings = settings;

	switch (windowBorder)
	{
	case WindowBorder::Windowed:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), m_name, sf::Style::Close, settings);
		break;
	case WindowBorder::Fullscreen:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), m_name, sf::Style::Fullscreen, settings);
		break;
	case WindowBorder::BorderlessWindowed:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), m_name, sf::Style::None, settings);
		break;
	}

	setFramerateLimit(m_verticalSync ? 0 : m_framerate);
	setVerticalSyncEnabled(m_verticalSync);
}

void Window::SetBorder(WindowBorder border)
{
	if (m_border != border)
		Build(border, m_mode, m_settings);
}
void Window::SetMode(const sf::VideoMode& mode)
{
	if (m_mode != mode)
		Build(m_border, mode, m_settings);
}
void Window::SetSettings(const sf::ContextSettings& settings)
{
	Build(m_border, m_mode, settings);
}

void Window::SetClearColor(sf::Color Color)
{
	m_clearColor = Color;
}

void Window::SetCursorState(bool flag)
{
	setMouseCursorVisible(flag);
	setMouseCursorGrabbed(!flag);
}

RectFloat Window::GetBorder() const
{
	return RectFloat(0, 0, 
		static_cast<float>(getSize().x), 
		static_cast<float>(getSize().y));
}
