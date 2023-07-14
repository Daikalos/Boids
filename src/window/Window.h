#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/ContextSettings.hpp>

#include "../utilities/Rectangle.hpp"

enum class WindowBorder
{
	Windowed,
	Fullscreen,
	BorderlessWindowed
};

class Camera;

// allow for toggle fullscreen, change resolution, and other settings
//
class Window : public sf::RenderWindow
{
public:
	Window(std::string name, sf::VideoMode mode, WindowBorder windowBorder, sf::ContextSettings settings, bool verticalSync, int framerate, Camera& camera);

	void Initialize();
	void HandleEvent(const sf::Event& event);

	// clears and sets view
	//
	void Setup();

	void SetFramerate(int frame_rate);
	void SetVerticalSync(bool flag);

	void Build(WindowBorder windowBorder, sf::VideoMode mode, sf::ContextSettings settings);

	void SetBorder(WindowBorder border);
	void SetMode(sf::VideoMode mode);
	void SetSettings(sf::ContextSettings settings);

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

	Camera*					m_camera;
};