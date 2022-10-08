#pragma once

#include "../window/Camera.h"
#include "../window/Window.h"
#include "../window/InputHandler.h"
#include "../window/ResourceHolder.hpp"
#include "../window/State.h"
#include "../window/StateStack.h"

#include "../utilities/PolicySelect.h"

#include "Config.h"
#include "Debug.h"
#include "AudioMeter.h"
#include "Background.h"
#include "Grid.h"
#include "Impulse.hpp"
#include "Boid.h"
#include "Fluid.h"

class MainState final : public State
{
public:
	MainState(StateStack& stack, Context context, Config& config);

	bool handle_event(const sf::Event& event) override;

	bool pre_update(float dt) override;
	bool update(float dt) override;
	bool fixed_update(float dt) override;
	bool post_update(float dt, float interp) override;

	void draw() override;

private:
	Window*						_window;

	Config*						_config;
	Grid						_grid;
	Debug						_debug;
	IAudioMeterInfo::ptr		_audio_meter	{nullptr};
	Background					_background;
	Fluid						_fluid;

	std::vector<Impulse>		_impulses;
	std::vector<Boid>			_boids;
	std::vector<std::uint32_t>	_proxy;
	sf::VertexArray				_vertices;
	RectFloat					_border;

	sf::Vector2f				_mouse_pos;
	sf::Vector2f				_mouse_pos_prev;

	sf::Vector2i				_fluid_mouse_pos;
	sf::Vector2i				_fluid_mouse_pos_prev;

	float						_min_distance	{0.0f};
	Policy						_policy			{Policy::unseq};
};