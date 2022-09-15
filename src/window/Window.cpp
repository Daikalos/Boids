#include "Window.h"

Window::Window(std::string name, sf::VideoMode mode, WindowBorder window_border, sf::ContextSettings settings, bool vertical_sync, int frame_rate, Camera& _camera)
	: _name(name), _mode(mode), _border(window_border), _settings(settings), _vertical_sync(vertical_sync), _frame_rate(frame_rate), _camera(&_camera) {}

void Window::initialize()
{
	_mode = sf::VideoMode::getDesktopMode();

	build(_border, _mode, _settings);

	_camera->set_size(sf::Vector2f(getSize()));
	_camera->set_position(_camera->get_size() / 2.0f);

#ifdef _WIN32
	hide_taskbar_icon(*this);
#endif

	if (!setActive(true))
		throw std::runtime_error("window could not be activated");

	glViewport(0, 0, getSize().x, getSize().y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f, -1.0f, 1.0f);
	glOrtho(0, getSize().x, 0, getSize().y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void Window::handle_event(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		close();
		break;
	case sf::Event::Resized:
		{
			glViewport(0, 0, getSize().x, getSize().y);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glScalef(1.0f, -1.0f, 1.0f);
			glOrtho(0, getSize().x, 0, getSize().y, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
		}
		break;
	}
}

void Window::setup()
{
	clear(sf::Color(_clear_color.r, _clear_color.g, _clear_color.b, 0));
	setView(*_camera);
}

void Window::set_framerate(int frame_rate)
{
	_frame_rate = frame_rate;
	setFramerateLimit(_vertical_sync ? 0 : frame_rate);
}

void Window::set_vertical_sync(bool flag)
{
	_vertical_sync = flag;

	setFramerateLimit(_vertical_sync ? 0 : _frame_rate);
	setVerticalSyncEnabled(_vertical_sync);
}

void Window::build(WindowBorder window_border, sf::VideoMode mode, sf::ContextSettings settings)
{
	_border = window_border;
	_mode = mode;
	_settings = settings;

	switch (window_border)
	{
	case WindowBorder::Windowed:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), _name, sf::Style::Close, settings);
		break;
	case WindowBorder::Fullscreen:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), _name, sf::Style::Fullscreen, settings);
		break;
	case WindowBorder::BorderlessWindowed:
		create(sf::VideoMode(mode.size, sf::VideoMode::getDesktopMode().bitsPerPixel), _name, sf::Style::None, settings);
		break;
	}

	setFramerateLimit(_vertical_sync ? 0 : _frame_rate);
	setVerticalSyncEnabled(_vertical_sync);
}

void Window::set_border(WindowBorder border)
{
	if (_border != border)
		build(border, _mode, _settings);
}
void Window::set_mode(sf::VideoMode mode)
{
	if (_mode != mode)
	{
		build(_border, mode, _settings);
		_camera->set_size(sf::Vector2f(mode.size));
	}
}
void Window::set_settings(sf::ContextSettings settings)
{
	build(_border, _mode, settings);
}

void Window::set_clear_color(sf::Color color)
{
	_clear_color = color;
}

void Window::set_cursor_state(bool flag)
{
	setMouseCursorVisible(flag);
	setMouseCursorGrabbed(!flag);
}

RectFloat Window::get_border() const
{
	return RectFloat(0, 0, getSize().x, getSize().y);
}
