#pragma once

#include <SFML/Graphics.hpp>

#include "../boids/Config.h"
#include "../boids/MainState.h"

#include "Camera.h"
#include "Window.h"
#include "ResourceHolder.hpp"
#include "StateStack.h"

class Application
{
public:
	Application(const std::string& name);
	~Application();

	void run();

private:
	void process_input();

	void pre_update(float dt);
	void update(float dt);
	void fixed_update(float dt); // after update, before post_update
	void post_update(float dt, float interp);

	void draw();
		
	void register_states();

private:
	Config			_config;
	Camera			_camera;
	Window			_window;
	InputHandler	_input_handler;
	TextureHolder	_texture_holder;
	FontHolder		_font_holder;
	StateStack		_state_stack;
};

