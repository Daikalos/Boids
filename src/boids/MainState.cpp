#include "MainState.h"

MainState::MainState(Context context) 
	: State(context), m_window(context.window), m_camera(context.camera), m_inputHandler(context.inputHandler) {}

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

	m_boids.reserve(Config::Inst().BoidCount);
	m_proxy.reserve(Config::Inst().BoidCount);

	for (int i = 0; i < Config::Inst().BoidCount; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::random(0.0f, m_border.width()) - m_border.left,
			util::random(0.0f, m_border.height()) - m_border.top);

		m_boids.emplace_back(pos);
		m_proxy.emplace_back(i);
	}

	m_vertices.resize(m_boids.size() * 3);
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
    m_debug.Update(*m_inputHandler, m_boids.size(), m_grid.GetCount(), dt);

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
						m_boids.reserve(Config::Inst().BoidCount);
						m_proxy.reserve(Config::Inst().BoidCount);

						for (int i = prev.BoidCount; i < Config::Inst().BoidCount; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::random(0.0f, m_border.width()) - m_border.left,
								util::random(0.0f, m_border.height()) - m_border.top);

							m_boids.emplace_back(pos);
							m_proxy.emplace_back(i);
						}
					}
					else
					{
						m_boids.erase(m_boids.begin() + Config::Inst().BoidCount, m_boids.end());

						m_proxy.erase(std::remove_if(
							m_proxy.begin(), m_proxy.end(),
							[this](const int index)
							{
								return index >= m_boids.size();
							}), m_proxy.end());
					}

					m_vertices.resize(m_boids.size() * 3);
					m_policy = m_boids.size() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
				}
				break;
			case RB_BoidsCycle:
				{
					for (Boid& Boid : m_boids)
					{
						Boid.SetCycleTime(Config::Inst().BoidCycleColorsRandom ?
							util::random(0.0f, 1.0f) : 0.0f);
					}
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
		if (vu::distance(mouseDelta) > Config::Inst().BoidAddMouseDiff)
		{
			for (int i = 0; i < Config::Inst().BoidAddAmount; ++i)
			{
				const sf::Vector2f center = sf::Vector2f(Config::Inst().BoidWidth, Config::Inst().BoidHeight) / 2.0f;
				const sf::Vector2f initPos = m_mousePos - center;

				Boid& Boid = m_boids.emplace_back(initPos,
					vu::rotate_point(mouseDelta, {}, util::random(-1.0f, 1.0f)));

				m_proxy.push_back(static_cast<std::uint32_t>(m_boids.size() - 1));
			}

			m_vertices.resize(m_boids.size() * 3);
			m_policy = m_boids.size() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
		}
	}
	if (m_inputHandler->GetKeyHeld(sf::Keyboard::Key::Delete))
	{
		if (m_boids.size() > Config::Inst().BoidCount)
		{
			std::size_t remove_amount = (m_boids.size() - Config::Inst().BoidCount) >= Config::Inst().BoidRemoveAmount ? 
				Config::Inst().BoidRemoveAmount : m_boids.size() - Config::Inst().BoidCount;

			for (std::size_t i = 0; i < remove_amount; ++i)
				m_boids.pop_back();

			m_proxy.erase(std::remove_if(
				m_proxy.begin(), m_proxy.end(),
				[this](const int index)
				{
					return index >= m_boids.size();
				}), m_proxy.end());

			m_vertices.resize(m_boids.size() * 3);
			m_policy = m_boids.size() <= Config::Inst().PolicyThreshold ? Policy::unseq : Policy::par_unseq;
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

	std::for_each(m_boids.begin(), m_boids.end(),
		[this](Boid& boid)
		{
			boid.PreUpdate(m_grid);
		});

	std::sort(m_proxy.begin(), m_proxy.end(),
		[this](const std::uint32_t& i0, const std::uint32_t& i1)
		{
			return m_boids[i0].GetCellIndex() < m_boids[i1].GetCellIndex();
		});

	std::for_each(m_proxy.begin(), m_proxy.end(),
		[this](const std::uint32_t& index)
		{
			m_boids[index].UpdateGridCells(m_grid, m_boids, m_proxy, (std::uint32_t)(&index - m_proxy.data()));
		});

	PolicySelect(
		[this](auto& pol)
		{
			std::for_each(pol, m_boids.begin(), m_boids.end(),
				[this](Boid& boid)
				{
					bool holdLeft  = m_inputHandler->GetButtonHeld(sf::Mouse::Button::Left);
					bool holdRight = m_inputHandler->GetButtonHeld(sf::Mouse::Button::Right);

					if (Config::Inst().SteerEnabled && (holdLeft || holdRight))
					{
						sf::Vector2f dir = vu::direction(boid.GetPosition(), m_mousePos);

						const float factor = holdLeft ? 1.0f :
							(holdRight ? -1.0f : 0.0f);

						const float lengthOpt = vu::distance_opt(dir);
						const float weight = 15.0f / (std::sqrtf(lengthOpt) + FLT_EPSILON); // hard coded because cant update Config::Inst().. 

						boid.SteerTowards(dir, lengthOpt, Config::Inst().SteerTowardsFactor * weight * factor);
					}
					else if (Config::Inst().PredatorEnabled)
					{
						sf::Vector2f dir = vu::direction(boid.GetPosition(), m_mousePos);

						float lengthSqr = vu::distance_sq(dir);
						if (lengthSqr <= Config::Inst().PredatorDistance)
						{
							float weight = std::sqrtf(lengthSqr / Config::Inst().PredatorDistance);
							boid.SteerTowards(dir, -Config::Inst().PredatorFactor / (weight + FLT_EPSILON));
						}
					}

					boid.Flock(m_grid, m_boids, m_proxy);
				});
		}, m_policy);


	std::for_each(m_boids.begin(), m_boids.end(),
		[&dt, this](Boid& boid)
		{
			boid.Update(m_border, m_impulses, dt);
		});

    return true;
}

bool MainState::PostUpdate(float dt, float interp)
{
	PolicySelect(
		[&interp, this](auto& pol)
		{
			std::for_each(pol, m_boids.begin(), m_boids.end(),
				[&interp, this](const Boid& boid)
				{
					Config& config = Config::Inst();

					const sf::Vector2f ori = boid.GetOrigin();
					const sf::Vector2f prevOri = boid.GetPrevOrigin();

					const float rot = boid.GetVelocity().angle().asRadians();
					const float prev_rot = boid.GetPrevVelocity().angle().asRadians();

					const sf::Vector2f pointA = vu::rotate_point({ ori.x + (config.BoidWidth / 2), ori.y							}, ori, rot); // middle right tip
					const sf::Vector2f pointB = vu::rotate_point({ ori.x - (config.BoidWidth / 2), ori.y - (config.BoidHeight / 2)	}, ori, rot); // top left corner
					const sf::Vector2f pointC = vu::rotate_point({ ori.x - (config.BoidWidth / 2), ori.y + (config.BoidHeight / 2)	}, ori, rot); // bot left corner

					const sf::Vector2f prevPointA = vu::rotate_point({ prevOri.x + (config.BoidWidth / 2), prevOri.y							}, prevOri, prev_rot); // middle right tip
					const sf::Vector2f prevPointB = vu::rotate_point({ prevOri.x - (config.BoidWidth / 2), prevOri.y - (config.BoidHeight / 2)	}, prevOri, prev_rot); // top left corner
					const sf::Vector2f prevPointC = vu::rotate_point({ prevOri.x - (config.BoidWidth / 2), prevOri.y + (config.BoidHeight / 2)	}, prevOri, prev_rot); // bot left corner

					const sf::Vector2f p0 = pointA * interp + prevPointA * (1.0f - interp);
					const sf::Vector2f p1 = pointB * interp + prevPointB * (1.0f - interp);
					const sf::Vector2f p2 = pointC * interp + prevPointC * (1.0f - interp);

					sf::Vector3f bc = boid.GetColor(m_border, m_audioMeter.get(), m_impulses);

					sf::Vector3f bc0 = bc;
					sf::Vector3f bc1 = bc;
					sf::Vector3f bc2 = bc;

					if ((config.ColorFlag & CF_Fluid) == CF_Fluid)
					{
						bc0 += m_fluid.GetColor(p0);
						bc1 += m_fluid.GetColor(p1);
						bc2 += m_fluid.GetColor(p2);
					}

					bc0.x = std::clamp(bc0.x, 0.0f, 1.0f);
					bc0.y = std::clamp(bc0.y, 0.0f, 1.0f);
					bc0.z = std::clamp(bc0.z, 0.0f, 1.0f);

					bc1.x = std::clamp(bc1.x, 0.0f, 1.0f);
					bc1.y = std::clamp(bc1.y, 0.0f, 1.0f);
					bc1.z = std::clamp(bc1.z, 0.0f, 1.0f);

					bc2.x = std::clamp(bc2.x, 0.0f, 1.0f);
					bc2.y = std::clamp(bc2.y, 0.0f, 1.0f);
					bc2.z = std::clamp(bc2.z, 0.0f, 1.0f);

					const sf::Color c0 = sf::Color(
						(sf::Uint8)(bc0.x * 255.0f), (sf::Uint8)(bc0.y * 255.0f), (sf::Uint8)(bc0.z * 255.0f));
					const sf::Color c1 = sf::Color(
						(sf::Uint8)(bc1.x * 255.0f), (sf::Uint8)(bc1.y * 255.0f), (sf::Uint8)(bc1.z * 255.0f));
					const sf::Color c2 = sf::Color(
						(sf::Uint8)(bc2.x * 255.0f), (sf::Uint8)(bc2.y * 255.0f), (sf::Uint8)(bc2.z * 255.0f));

					const auto v = (&boid - m_boids.data()) * 3;

					m_vertices[v + 0].position = p0;
					m_vertices[v + 1].position = p1;
					m_vertices[v + 2].position = p2;

					m_vertices[v + 0].color = c0;
					m_vertices[v + 1].color = c1;
					m_vertices[v + 2].color = c2;
				});
		}, m_policy);

    return true;
}

void MainState::Draw()
{
	m_background.Draw(*m_window);
	m_window->draw(m_vertices);
	m_debug.Draw(*m_window);
}
