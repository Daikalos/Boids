#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/ContextSettings.hpp>

#include "Rectangle.hpp"

enum class WindowBorder
{
	Windowed,
	Fullscreen,
	BorderlessWindowed
};

class Camera;

// Allow for toggle fullscreen, change resolution, and other settings
//
class Window : public sf::RenderWindow
{
public:
	Window(
		std::string name = "Title",
		const sf::VideoMode& mode = sf::VideoMode::getDesktopMode(),
		WindowBorder windowBorder = WindowBorder::Windowed, 
		bool verticalSync = false, 
		int frameRate = 60, 
		const sf::ContextSettings& settings = sf::ContextSettings{});

	void Initialize();
	void HandleEvent(const sf::Event& event);

	// clears and sets view
	//
	void Setup();

	void SetFramerate(int frameRate);
	void SetVerticalSync(bool flag);

	void Build(WindowBorder windowBorder, const sf::VideoMode& mode, const sf::ContextSettings& settings);

	void SetBorder(WindowBorder border);
	void SetMode(const sf::VideoMode& mode);
	void SetSettings(const sf::ContextSettings& settings);

	void SetClearColor(sf::Color Color);

	// false = hides and grabs the cursor
	// true = shows and unhooks the cursor
	//
	void SetCursorState(bool flag);

	RectFloat GetBorder() const;

private:
	std::string				m_name;
	sf::VideoMode			m_mode;
	WindowBorder			m_border;
	sf::ContextSettings		m_settings;
	bool					m_verticalSync	{false};
	int						m_framerate		{60};
	sf::Color				m_clearColor	{sf::Color::Black};
};