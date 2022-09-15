#pragma once

#include <SFML/Graphics.hpp>

#include "../utilities/NonCopyable.h"
#include "../utilities/Rectangle.hpp"
#include "../utilities/HideTaskbarIcon.h"

#include "Camera.h"

enum class WindowBorder
{
	Windowed,
	Fullscreen,
	BorderlessWindowed
};

// allow for toggle fullscreen, change resolution, and other settings
//
class Window : public sf::RenderWindow, NonCopyable
{
public:
	Window(std::string name, sf::VideoMode mode, WindowBorder window_border, sf::ContextSettings settings, bool vertical_sync, int frame_rate, Camera& camera);

	void initialize();
	void handle_event(const sf::Event& event);

	// clears and sets view
	//
	void setup();

	void set_framerate(int frame_rate);
	void set_vertical_sync(bool flag);

	void build(WindowBorder window_border, sf::VideoMode mode, sf::ContextSettings settings);

	void set_border(WindowBorder border);
	void set_mode(sf::VideoMode mode);
	void set_settings(sf::ContextSettings settings);

	void set_clear_color(sf::Color color);

	// false = hides and grabs the cursor
	// true = shows and unhooks the cursor
	//
	void set_cursor_state(bool flag);

	RectFloat get_border() const;

private:
	std::string				_name;
	sf::VideoMode			_mode;
	WindowBorder			_border;
	sf::ContextSettings		_settings;
	bool					_vertical_sync;
	int						_frame_rate;
	sf::Color				_clear_color;

	Camera*					_camera;
};