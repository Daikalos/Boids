#include "Application.h"

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>

#include "../boids/Config.h"

Application::Application(std::string name) 
	: m_window(
		std::move(name), 
		sf::VideoMode().getDesktopMode(), 
		WindowBorder::Fullscreen, 
		Config::Inst().Misc.VerticalSync, 
		Config::Inst().Misc.MaxFramerate)
	, m_inputHandler()
	, m_textureHolder()
	, m_mainState(State::Context(m_window, m_camera, m_inputHandler, m_textureHolder, m_fontHolder))
{
	m_camera.SetScale({ Config::Inst().Misc.CameraZoom, Config::Inst().Misc.CameraZoom });

	m_window.SetClearColor(sf::Color(
		static_cast<std::uint8_t>(Config::Inst().Background.Color.x * 255.0f),
		static_cast<std::uint8_t>(Config::Inst().Background.Color.y * 255.0f),
		static_cast<std::uint8_t>(Config::Inst().Background.Color.z * 255.0f)));
}

void Application::Run()
{
	m_window.Initialize();
	m_mainState.Initialize();

	m_camera.SetSize(sf::Vector2f(m_window.getSize()));
	m_camera.SetPosition(m_camera.GetSize() / 2.0f);

	sf::Clock clock;
	float dt = FLT_EPSILON;

	float fixedDT = 1.0f / 60.0f;
	float accumulator = FLT_EPSILON;

	int ticks = 0;
	int deathSpiral = 12; // guarantee prevention of infinite loop

	while (m_window.isOpen())
	{
		dt = std::fminf(clock.restart().asSeconds(), 0.075f);
		accumulator += dt;

		fixedDT = 1.0f / std::fmaxf(Config::Inst().Misc.PhysicsUpdateFreq, 1.0f);

		m_inputHandler.Update(dt);

		ProcessInput();

		PreUpdate(dt);

		Update(dt);

		ticks = 0;
		while (accumulator >= fixedDT && ticks++ < deathSpiral)
		{
			accumulator -= fixedDT;
			FixedUpdate(fixedDT);
		}

		float interp = accumulator / fixedDT;
		PostUpdate(dt, interp); // interp

		Draw();
	}
}

void Application::ProcessInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		m_inputHandler.HandleEvent(event);
		m_camera.HandleEvent(event);
		m_window.HandleEvent(event);
		m_mainState.HandleEvent(event);
	}
}

void Application::PreUpdate(float dt)
{
	if (Config::Inst().Misc.CameraEnabled)
		m_camera.Update(m_inputHandler, m_window);

	m_mainState.PreUpdate(dt);
}

void Application::Update(float dt)
{
	m_mainState.Update(dt);
}

void Application::FixedUpdate(float dt)
{
	m_mainState.FixedUpdate(dt);
}

void Application::PostUpdate(float dt, float interp)
{
	m_mainState.PostUpdate(dt, interp);
}

void Application::Draw()
{
	m_window.Setup();
	m_window.setView(m_camera);
	m_mainState.Draw();
	m_window.display();
}
