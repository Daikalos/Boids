#pragma once

#include "../window/Camera.h"
#include "../window/Window.h"
#include "../window/InputHandler.h"
#include "../window/ResourceHolder.hpp"
#include "../window/State.h"

#include "../utilities/PolicySelect.h"

#include "Config.h"
#include "Debug.h"
#include "AudioMeter.h"
#include "Background.h"
#include "Grid.h"
#include "Impulse.h"
#include "Boid.h"
#include "Fluid.h"

class MainState final : public State
{
public:
	MainState(Context context);

public:
	void Initialize();

	bool HandleEvent(const sf::Event& event) override;

	bool PreUpdate(float dt) override;
	bool Update(float dt) override;
	bool FixedUpdate(float dt) override;
	bool PostUpdate(float dt, float interp) override;

	void Draw() override;

private:
	Window*						m_window		{nullptr};
	Camera*						m_camera		{nullptr};
	InputHandler*				m_inputHandler	{nullptr};

	Grid						m_grid;
	Debug						m_debug;
	IAudioMeterInfo::Ptr		m_audioMeter	{nullptr};
	Background					m_background;
	Fluid						m_fluid;

	std::vector<Impulse>		m_impulses;
	BoidContainer				m_boids;
	sf::VertexArray				m_vertices;
	RectFloat					m_border;

	sf::Vector2f				m_mousePos;
	sf::Vector2f				m_mousePosPrev;

	sf::Vector2i				m_fluidMousePos;
	sf::Vector2i				m_fluidMousePosPrev;

	float						m_minDistance	{0.0f};
	Policy						m_policy		{Policy::unseq};
};