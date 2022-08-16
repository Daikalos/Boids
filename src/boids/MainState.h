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
#include "Grid.hpp"
#include "Impulse.hpp"
#include "Boid.h"

struct Vertex
{
	GLfloat x{0.0f}, y{0.0f};
};

struct Color
{
	GLfloat r{0.0f}, g{0.0f}, b{0.0f};
};

class MainState : public State
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
	Config* _config;

	Grid _grid;
	Debug _debug;
	AudioMeter _audio_meter;
	Background _background;

	std::vector<Impulse> _impulses;
	std::vector<Boid> _boids;
	std::vector<int> _sorted_boids;

	std::vector<Vertex> _vertices;
	std::vector<Color> _colors;
	
	RectInt _border;

	sf::Vector2f _mouse_pos;
	float _min_distance;

	Policy _policy;
};