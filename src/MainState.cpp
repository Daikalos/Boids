#include "MainState.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

#include "Camera.h"
#include "Window.h"
#include "InputHandler.h"
#include "SFMLLoaders.hpp"

#include "PolicySelect.h"
#include "CommonUtilities.hpp"
#include "VectorUtilities.hpp"

#include "Config.h"

MainState::MainState(Context context) 
	: State(context)
	, m_window(&context.GetWindow())
	, m_camera(&context.GetCamera())
	, m_inputHandler(&context.GetInputHandler())
	, m_boids(Config::Inst().Boids.Count) {}

void MainState::Initialize()
{
	m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	auto loadBoidTex = GetContext().GetTextureHolder().AcquireAsync(TextureID::Boid,
		FromFile<sf::Texture>(RESOURCE_FOLDER + Config::Inst().Boids.Texture));

	auto loadFont = GetContext().GetFontHolder().AcquireAsync(FontID::F8Bit, 
		FromFile<sf::Font>(RESOURCE_FOLDER + std::string("8bit.ttf")));

	loadBoidTex.wait();
	loadFont.wait();

	m_background.Load(GetContext().GetTextureHolder(), sf::Vector2i(m_window->getSize()));
	m_debug.Load(GetContext().GetFontHolder());

#if defined(_WIN32)
	m_audioMeter = std::make_unique<AudioMeterWin>(1.0f);
#else
	m_audioMeter = std::make_unique<AudioMeterEmpty>();
#endif

	m_border = m_window->GetBorder();
	m_minDistance = GetMinDistance();

	m_audioMeter->Initialize();
	m_fluid.Initialize(m_window->getSize());
	m_grid.Initialize(GetGridBorder(), sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);

	m_fluidMousePosPrev = m_fluidMousePos = sf::Vector2i(m_camera->
		GetMouseWorldPosition(*m_window)) / Config::Inst().Fluid.Scale;

	m_grid.Initialize(GetGridBorder(), sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);

	m_boids.Reserve(Config::Inst().Boids.Count);
	for (std::size_t i = 0; i < Config::Inst().Boids.Count; ++i)
	{
		sf::Vector2f pos = sf::Vector2f(
			util::Random(0.0f, m_border.width) - m_border.left,
			util::Random(0.0f, m_border.height) - m_border.top);

		m_boids.Push(pos);
	}

	UpdateVertices();
	UpdatePolicy();

	SetBoidTexture(loadBoidTex.get());
}

bool MainState::HandleEvent(const sf::Event& event)
{
    switch (event.type)
    {
		case sf::Event::Resized:
        {
			m_background.LoadProperties(sf::Vector2i(m_window->getSize()));

			m_fluid.Initialize(m_window->getSize());
			m_border = m_window->GetBorder();

			m_grid.Initialize(GetGridBorder(), sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);
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
		for (Rebuild rebuild : Config::Inst().Refresh(prev))
		{
			switch (rebuild)
			{
				case Rebuild::Grid:
				{
					m_minDistance = GetMinDistance();
					m_grid.Initialize(GetGridBorder(), sf::Vector2f(m_minDistance, m_minDistance) * 2.0f);

					break;
				}
				case Rebuild::Boids:
				{
					if (Config::Inst().Boids.Count > prev.Boids.Count) // new is larger
					{
						m_boids.Reserve(Config::Inst().Boids.Count);

						for (std::size_t i = prev.Boids.Count; i < Config::Inst().Boids.Count; ++i)
						{
							sf::Vector2f pos = sf::Vector2f(
								util::Random(0.0f, m_border.width) - m_border.left,
								util::Random(0.0f, m_border.height) - m_border.top);

							m_boids.Push(pos);
						}
					}
					else
					{
						m_boids.Pop(m_boids.GetSize() - Config::Inst().Boids.Count);
					}

					UpdateVertices();
					UpdatePolicy();

					break;
				}
				case Rebuild::BoidsTex:
				{
					SetBoidTexture(GetContext().GetTextureHolder().Acquire(TextureID::Boid,
						FromFile<sf::Texture>(RESOURCE_FOLDER + Config::Inst().Boids.Texture), res::LoadStrategy::Reload));

					break;
				}
				case Rebuild::BoidsCycle:
				{
					m_boids.ResetCycleTimes();
					break;
				}
				case Rebuild::BackgroundTex:
				{
					m_background.Load(GetContext().GetTextureHolder(), sf::Vector2i(m_window->getSize()));
					break;
				}
				case Rebuild::BackgroundProp:
				{
					m_background.LoadProperties(sf::Vector2i(m_window->getSize()));

					m_window->SetClearColor(sf::Color(
						(std::uint8_t)(Config::Inst().Background.Color.x * 255.999f),
						(std::uint8_t)(Config::Inst().Background.Color.y * 255.999f),
						(std::uint8_t)(Config::Inst().Background.Color.z * 255.999f)));

					break;
				}
				case Rebuild::Audio:
				{
					m_audioMeter->Clear();
					break;
				}
				case Rebuild::Window:
				{
					m_window->SetFramerate(Config::Inst().Misc.MaxFramerate);
					m_window->SetVerticalSync(Config::Inst().Misc.VerticalSync);
					break;
				}
				case Rebuild::Camera:
				{
					m_camera->SetScale(sf::Vector2f(Config::Inst().Misc.CameraZoom, Config::Inst().Misc.CameraZoom));
					break;
				}
				case Rebuild::Fluid:
				{
					m_fluid.Initialize(m_window->getSize());
					break;
				}
			}
		}
	}

    return true;
}

bool MainState::Update(float dt)
{
	m_audioMeter->Update(dt);

	m_mousePosPrev = m_mousePos;
	m_mousePos = sf::Vector2f(m_camera->GetMouseWorldPosition(*m_window));

	if (m_inputHandler->GetKeyHeld(sf::Keyboard::Key::RAlt) && m_inputHandler->GetButtonHeld(sf::Mouse::Button::Middle))
	{
		const sf::Vector2f mouseDelta = vu::Direction(m_mousePosPrev, m_mousePos);
		if (mouseDelta.lengthSq() > Config::Inst().Interaction.BoidAddMouseDiff)
		{
			for (int i = 0; i < Config::Inst().Interaction.BoidAddAmount; ++i)
			{
				m_boids.Push(m_mousePos, vu::RotatePoint(mouseDelta, {}, util::Random(-1.0f, 1.0f)));
			}

			UpdateVertices();
			UpdatePolicy();
		}
	}
	if (m_inputHandler->GetKeyHeld(sf::Keyboard::Key::Delete) && m_boids.GetSize() > Config::Inst().Boids.Count)
	{
		std::size_t removeAmount = std::min(
			m_boids.GetSize() - Config::Inst().Boids.Count, 
			(std::size_t)Config::Inst().Interaction.BoidRemoveAmount);

		m_boids.Pop(removeAmount);

		UpdateVertices();
		UpdatePolicy();
	}

	if (Config::Inst().Impulse.Enabled && m_inputHandler->GetButtonPressed(sf::Mouse::Button::Left))
	{
		m_impulses.emplace_back(m_mousePos, Config::Inst().Impulse.Speed, Config::Inst().Impulse.Size, -Config::Inst().Impulse.Size);
	}

	for (auto i = std::ssize(m_impulses) - 1; i >= 0; --i)
	{
		Impulse& impulse = m_impulses[i];

		impulse.Update(dt);
		if (impulse.GetLength() > Config::Inst().Impulse.FadeDistance)
		{
			m_impulses.erase(m_impulses.begin() + i);
		}
	}

	if ((Config::Inst().Color.Flags & CF_Fluid) == CF_Fluid)
	{
		m_fluidMousePosPrev = m_fluidMousePos;
		m_fluidMousePos = sf::Vector2i(m_mousePos / (float)Config::Inst().Fluid.Scale);

		const sf::Vector2i amount = vu::Abs(m_fluidMousePos - m_fluidMousePosPrev);

		if (std::abs(amount.x) > 0 || std::abs(amount.y) > 0)
		{
			m_fluid.StepLine(
				m_fluidMousePosPrev.x, m_fluidMousePosPrev.y,
				m_fluidMousePos.x, m_fluidMousePos.y,
				amount.x, amount.y, Config::Inst().Fluid.MouseStrength);
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
	m_boids.Interaction(*m_inputHandler, m_mousePos, dt);
	m_boids.Flock(m_grid, m_policy);
	m_boids.Update(m_border, m_impulses, dt);
	m_boids.UpdateColors(m_border, m_fluid, m_audioMeter.get(), m_impulses);

    return true;
}

bool MainState::PostUpdate([[maybe_unused]] float dt, float interp)
{
	m_boids.UpdateVertices(m_vertices, interp, m_policy);

    return true;
}

void MainState::Draw()
{
	sf::RenderStates renderStates;
	renderStates.texture = m_boidTexture;

	m_background.Draw(*m_window);
	m_window->draw(m_vertices, renderStates);
	m_debug.Draw(*m_window);
}

RectFloat MainState::GetGridBorder() const
{
	if (!Config::Inst().Interaction.TurnAtBorder)
		return m_border;

	return m_border + RectFloat(
		-m_minDistance * Config::Inst().Misc.GridExtraCells,
		-m_minDistance * Config::Inst().Misc.GridExtraCells,
		+m_minDistance * Config::Inst().Misc.GridExtraCells * 2.0f,
		+m_minDistance * Config::Inst().Misc.GridExtraCells * 2.0f);
}

float MainState::GetMinDistance() const
{
	return std::sqrtf(std::max({ Config::Inst().Rules.SepDistance, Config::Inst().Rules.AliDistance, Config::Inst().Rules.CohDistance }));
}

void MainState::SetBoidTexture(sf::Texture& texture)
{
	m_boidTexture = &texture;

	const sf::Vector2u texSize = m_boidTexture->getSize();
	for (std::size_t i = 0; i < m_boids.GetSize(); ++i)
	{
		const std::size_t v = i * 6;

		m_vertices[v + 0].texCoords = sf::Vector2f(0.0f,				0.0f);
		m_vertices[v + 1].texCoords = sf::Vector2f((float)texSize.x,	0.0f);
		m_vertices[v + 2].texCoords = sf::Vector2f(0.0f,				(float)texSize.y);
		m_vertices[v + 3].texCoords = sf::Vector2f((float)texSize.x,	0.0f);
		m_vertices[v + 4].texCoords = sf::Vector2f(0.0f,				(float)texSize.y);
		m_vertices[v + 5].texCoords = sf::Vector2f((float)texSize.x,	(float)texSize.y);
	}
}

void MainState::UpdateVertices()
{
	std::size_t oldSize = m_vertices.getVertexCount() / 6;
	std::size_t newSize = m_boids.GetSize();

	m_vertices.resize(newSize * 6);
	 
	if (newSize > oldSize && m_boidTexture != nullptr)
	{
		const sf::Vector2u texSize = m_boidTexture->getSize();
		for (std::size_t i = oldSize; i < newSize; ++i)
		{
			const std::size_t v = i * 6;

			m_vertices[v + 0].texCoords = sf::Vector2f(0.0f,				0.0f);
			m_vertices[v + 1].texCoords = sf::Vector2f((float)texSize.x,	0.0f);
			m_vertices[v + 2].texCoords = sf::Vector2f(0.0f,				(float)texSize.y);
			m_vertices[v + 3].texCoords = sf::Vector2f((float)texSize.x,	0.0f);
			m_vertices[v + 4].texCoords = sf::Vector2f(0.0f,				(float)texSize.y);
			m_vertices[v + 5].texCoords = sf::Vector2f((float)texSize.x,	(float)texSize.y);
		}
	}
}

void MainState::UpdatePolicy()
{
	m_policy = m_boids.GetSize() <= Config::Inst().Misc.PolicyThreshold ? Policy::unseq : Policy::par_unseq;
}
