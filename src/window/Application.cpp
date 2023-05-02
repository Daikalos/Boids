#include "Application.h"

Application::Application(const std::string& name) :
	m_window(name, sf::VideoMode().getDesktopMode(), WindowBorder::Fullscreen, sf::ContextSettings(), Config::Inst().VerticalSync, Config::Inst().MaxFramerate, m_camera),
	m_inputHandler(), 
	m_textureHolder(), 
	m_mainState(State::Context(m_window, m_camera, m_inputHandler, m_textureHolder, m_fontHolder))
{
	m_camera.SetScale({ Config::Inst().CameraZoom, Config::Inst().CameraZoom });

	m_window.SetClearColor(sf::Color(
		(sf::Uint8)(Config::Inst().BackgroundColor.x * 255.0f),
		(sf::Uint8)(Config::Inst().BackgroundColor.y * 255.0f),
		(sf::Uint8)(Config::Inst().BackgroundColor.z * 255.0f)));
}

void Application::Run()
{
	m_window.Initialize();
	m_mainState.Initialize();

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

		fixedDT = 1.0f / std::fmaxf(Config::Inst().PhysicsUpdateFreq, 1.0f);

		m_inputHandler.Update(dt);

		ProcessInput();

		if (Config::Inst().CameraEnabled)
			m_camera.Update(m_inputHandler, m_window);

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
	m_mainState.Draw();
	m_window.display();
}
