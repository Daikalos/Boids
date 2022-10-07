#include "Application.h"

Application::Application(const std::string& name) :
	_window(name, sf::VideoMode().getDesktopMode(), WindowBorder::Fullscreen, sf::ContextSettings(), _config.vertical_sync, _config.max_framerate, _camera),
	_input_handler(), 
	_texture_holder(), 
	_state_stack(State::Context(_window, _camera, _input_handler, _texture_holder, _font_holder))
{
	register_states();
	_state_stack.push(States::ID::Main);

	_camera.set_scale({ _config.camera_zoom, _config.camera_zoom });

	_window.set_clear_color(sf::Color(
		(sf::Uint8)(_config.background_color.x * 255),
		(sf::Uint8)(_config.background_color.y * 255),
		(sf::Uint8)(_config.background_color.z * 255), 255));
}

void Application::run()
{
	_window.initialize();

	sf::Clock clock;
	float dt = FLT_EPSILON;

	float dt_per_frame = 1.0f / 60.0f;
	float accumulator = FLT_EPSILON;

	int ticks = 0;
	int death_spiral = 12; // guarantee prevention of infinite loop

	while (_window.isOpen())
	{
		dt = std::fminf(clock.restart().asSeconds(), 0.075f);
		accumulator += dt;

		dt_per_frame = 1.0f / std::fmaxf(_config.physics_update_freq, 1.0f);

		_input_handler.update(dt);

		process_input();

		if (_config.camera_enabled)
			_camera.update(_input_handler, _window);

		pre_update(dt);

		update(dt);

		ticks = 0;
		while (accumulator >= dt_per_frame && ticks++ < death_spiral)
		{
			accumulator -= dt_per_frame;
			fixed_update(dt_per_frame);
		}

		float interp = accumulator / dt_per_frame;
		post_update(dt, interp); // interp

		if (_state_stack.is_empty())
			_window.close();

		draw();
	}
}

void Application::process_input()
{
	sf::Event event;
	while (_window.pollEvent(event))
	{
		_input_handler.handle_event(event);
		_camera.handle_event(event);
		_window.handle_event(event);

		_state_stack.handle_event(event);
	}
}

void Application::pre_update(float dt)
{
	_state_stack.pre_update(dt);
}

void Application::update(float dt)
{
	_state_stack.update(dt);
}

void Application::fixed_update(float dt)
{
	_state_stack.fixed_update(dt);
}

void Application::post_update(float dt, float interp)
{
	_state_stack.post_update(dt, interp);
}

void Application::draw()
{
	_window.setup();
	_state_stack.draw();
	_window.display();
}

void Application::register_states()
{
	_state_stack.register_state<MainState>(States::ID::Main, _config);
}
