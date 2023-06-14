#include "MainState.h"

MainState::MainState(Context context) 
	: State(context), m_window(context.window), m_camera(context.camera), m_inputHandler(context.inputHandler), m_boids(Config::Inst().BoidCount) {}

void MainState::Initialize()
{
	GetContext().textureHolder->Load(TextureID::Background, Config::Inst().BackgroundTexture);
	GetContext().fontHolder->Load(FontID::F8Bit, "font_8bit.ttf");

	m_debug.Load(*GetContext().fontHolder);

	m_background.LoadTexture(*GetContext().textureHolder);
	m_background.LoadProperties(sf::Vector2i(m_window->getSize()));

#if defined(_WIN32)
	m_audioMeter = std::make_unique<AudioMeterWin>(1.0f);
#else
	m_audioMeter = std::make_unique<AudioMeterEmpty>();
#endif

	m_audioMeter->Initialize();

	m_fluid = Fluid(m_window->getSize());
	m_fluidMousePosPrev = m_fluidMousePos = sf::Vector2i(m_camera->
		GetMouseWorldPosition(*m_window)) / Config::Inst().FluidScale;

	m_minDistance = std::sqrtf(std::fmaxf(std::fmaxf(Config::Inst().SepDistance, Config::Inst().AliDistance), Config::Inst().CohDistance));

	m_border = m_window->GetBorder();

	RectFloat gridBorder = m_border + (Config::Inst().TurnAtBorder ?
		RectFloat(
			-m_minDistance * Config::Inst().GridExtraCells,
			-m_minDistance * Config::Inst().GridExtraCells,
			+m_minDistance * Config::Inst().GridExtraCells,
			+m_minDistance * Config::Inst().GridExtraCells) : RectFloat());

	m_grid = Grid(gridBorder, sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);

	m_boids.Reserve(Config::Inst().BoidCount);

	for (int i = 0; i < Config::Inst().BoidCount; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0.0f, m_border.width()) - m_border.left,
			util::random(0.0f, m_border.height()) - m_border.top);

		m_boids.Push(pos);
	}

	m_vertices.resize(m_boids.GetSize() * 3);
	m_vertices.setPrimitiveType(sf::Triangles);

	m_policy = Config::Inst().BoidCount <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
}

bool MainState::HandleEvent(const sf::Event& event)
{
    switch (event.type)
    {
    case sf::Event::Resized:
        {
			m_background.LoadProperties(sf::Vector2i(m_window->getSize()));
			m_fluid = Fluid(m_window->getSize());
			m_border = m_window->GetBorder();

            RectFloat grid_border = m_border + (Config::Inst().TurnAtBorder ?
                RectFloat(
                    -m_minDistance * Config::Inst().GridExtraCells,
                    -m_minDistance * Config::Inst().GridExtraCells,
                    +m_minDistance * Config::Inst().GridExtraCells,
                    +m_minDistance * Config::Inst().GridExtraCells) : RectFloat());

            m_grid = Grid(grid_border, sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);
        }
        break;
    }
    return false;
}

bool MainState::PreUpdate(float dt)
{
    m_debug.Update(*m_inputHandler, m_boids.GetSize(), m_grid.GetCount(), dt);

	if (m_debug.GetRefresh()) // time to refresh data
	{
		Config prev = Config::Inst();
		for (const Rebuild& rebuild : Config::Inst().refresh(prev)) // not very clean, but it does not matter much for small project
		{
			switch (rebuild)
			{
			case RB_Grid:
				{
					m_minDistance = std::sqrtf(std::fmaxf(std::fmaxf(Config::Inst().SepDistance, Config::Inst().AliDistance), Config::Inst().CohDistance));

					RectFloat gridBorder = m_border + (Config::Inst().TurnAtBorder ?
						RectFloat(
							-m_minDistance * Config::Inst().GridExtraCells,
							-m_minDistance * Config::Inst().GridExtraCells,
							+m_minDistance * Config::Inst().GridExtraCells,
							+m_minDistance * Config::Inst().GridExtraCells) : RectFloat());

					m_grid = Grid(gridBorder, sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);
				}
				break;
			case RB_Boids:
				{
					if (Config::Inst().BoidCount > prev.BoidCount) // new is larger
					{
						m_boids.Reserve(Config::Inst().BoidCount);

						for (int i = prev.BoidCount; i < Config::Inst().BoidCount; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::random(0.0f, m_border.width()) - m_border.left,
								util::random(0.0f, m_border.height()) - m_border.top);

							m_boids.Push(pos);
						}
					}
					else
					{
						m_boids.Pop(m_boids.GetSize() - Config::Inst().BoidCount);
					}

					m_vertices.resize(m_boids.GetSize() * 3);
					m_policy = m_boids.GetSize() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
				}
				break;
			case RB_BoidsCycle:
				{
					m_boids.ResetCycleTimes();
				}
				break;
			case RB_BackgroundTex:
				{
					GetContext().textureHolder->Load(TextureID::Background, Config::Inst().BackgroundTexture);

					m_background.LoadTexture(*GetContext().textureHolder);
					m_background.LoadProperties(sf::Vector2i(m_window->getSize()));
				}
				break;
			case RB_BackgroundProp:
				{
					m_background.LoadProperties(sf::Vector2i(m_window->getSize()));

					m_window->SetClearColor(sf::Color(
						(sf::Uint8)(Config::Inst().BackgroundColor.x * 255.0f),
						(sf::Uint8)(Config::Inst().BackgroundColor.y * 255.0f),
						(sf::Uint8)(Config::Inst().BackgroundColor.z * 255.0f)));
				}
				break;
			case RB_Audio:
				m_audioMeter->Clear();
				break;
			case RB_Window:
				m_window->SetFramerate(Config::Inst().MaxFramerate);
				m_window->SetVerticalSync(Config::Inst().VerticalSync);
				break;
			case RB_Camera:
				m_camera->SetScale(sf::Vector2f(Config::Inst().CameraZoom, Config::Inst().CameraZoom));
				break;
			case RB_Fluid:
				m_fluid = Fluid(m_window->getSize());
				break;
			}
		}
	}

    return true;
}

bool MainState::Update(float dt)
{
	m_audioMeter->Update(dt);

	m_mousePosPrev = m_mousePos;
	m_mousePos = sf::Vector2f(m_camera->
		GetMouseWorldPosition(*m_window));

	if (m_inputHandler->GetButtonHeld(sf::Mouse::Middle))
	{
		const sf::Vector2f mouseDelta = vu::direction(m_mousePosPrev, m_mousePos);
		if (mouseDelta.length() > Config::Inst().BoidAddMouseDiff)
		{
			for (int i = 0; i < Config::Inst().BoidAddAmount; ++i)
			{
				const sf::Vector2f center = sf::Vector2f(Config::Inst().BoidWidth, Config::Inst().BoidHeight) / 2.0f;
				const sf::Vector2f initPos = m_mousePos - center;

				m_boids.Push(initPos, vu::rotate_point(mouseDelta, {}, util::random(-1.0f, 1.0f)));
			}

			m_vertices.resize(m_boids.GetSize() * 3);
			m_policy = m_boids.GetSize() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
		}
	}
	if (m_inputHandler->GetKeyHeld(sf::Keyboard::Key::Delete))
	{
		if (m_boids.GetSize() > Config::Inst().BoidCount)
		{
			std::size_t remove_amount = (m_boids.GetSize() - Config::Inst().BoidCount) >= Config::Inst().BoidRemoveAmount ? 
				Config::Inst().BoidRemoveAmount : m_boids.GetSize() - Config::Inst().BoidCount;

			m_boids.Pop(remove_amount);

			m_vertices.resize(m_boids.GetSize() * 3);
			m_policy = m_boids.GetSize() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
		}
	}

	if (Config::Inst().ImpulseEnabled && m_inputHandler->GetButtonPressed(sf::Mouse::Button::Left))
		m_impulses.push_back(Impulse(m_mousePos, Config::Inst().ImpulseSpeed, Config::Inst().ImpulseSize, -Config::Inst().ImpulseSize));

	for (int i = m_impulses.size() - 1; i >= 0; --i)
	{
		Impulse& impulse = m_impulses[i];

		impulse.Update(dt);
		if (impulse.GetLength() > Config::Inst().ImpulseFadeDistance)
			m_impulses.erase(m_impulses.begin() + i);
	}

	if ((Config::Inst().ColorFlag & CF_Fluid) == CF_Fluid)
	{
		m_fluidMousePosPrev = m_fluidMousePos;
		m_fluidMousePos = sf::Vector2i(m_mousePos / (float)Config::Inst().FluidScale);

		const sf::Vector2i amount = vu::abs(m_fluidMousePos - m_fluidMousePosPrev);

		if (std::abs(amount.x) > 0 || std::abs(amount.y) > 0)
		{
			m_fluid.StepLine(
				m_fluidMousePosPrev.x, m_fluidMousePosPrev.y,
				m_fluidMousePos.x, m_fluidMousePos.y,
				amount.x, amount.y, Config::Inst().FluidMouseStrength);
		}

		m_fluid.Update(dt);
	}

    return true;
}

bool MainState::FixedUpdate(float dt)
{
	m_grid.ResetBuffers();

	m_boids.PreUpdate(m_grid);
	m_boids.Sort();
	m_boids.UpdateCells(m_grid);
	m_boids.Interaction(*m_inputHandler, m_mousePos);
	m_boids.Flock(m_grid, m_policy);
	m_boids.Update(m_border, dt);
	m_boids.UpdateColors(m_border, m_fluid, m_audioMeter.get(), m_impulses);

    return true;
}

bool MainState::PostUpdate(float dt, float interp)
{
	m_boids.UpdateVertices(m_vertices, m_policy, interp);

    return true;
}

void MainState::Draw()
{
	m_background.Draw(*m_window);
	m_window->draw(m_vertices);
	m_debug.Draw(*m_window);
}
