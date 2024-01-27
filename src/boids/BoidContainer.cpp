#include "BoidContainer.h"

#include <ranges>
#include <cassert>

#include "../utilities/VectorUtilities.hpp"
#include "../utilities/CommonUtilities.hpp"

#include "Config.h"

BoidContainer::BoidContainer(std::size_t capacity) : m_capacity(capacity)
{
	m_indices			= std::make_unique<std::uint32_t[]>(m_capacity);
	m_triangles			= std::make_unique<Triangle[]>(m_capacity);
	m_prevTriangles		= std::make_unique<Triangle[]>(m_capacity);

	m_positions			= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_velocities		= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_prevVelocities	= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_relativePositions = std::make_unique<sf::Vector2f[]>(m_capacity);
	m_colors			= std::make_unique<sf::Vector3f[]>(m_capacity);

	m_speeds			= std::make_unique<float[]>(m_capacity);
	m_angles			= std::make_unique<float[]>(m_capacity);
	m_cycleTimes		= std::make_unique<float[]>(m_capacity);
	m_densityTimes		= std::make_unique<float[]>(m_capacity);

	m_densities			= std::make_unique<std::uint32_t[]>(m_capacity);
	m_cellIndices		= std::make_unique<std::uint16_t[]>(m_capacity);

	m_teleported		= std::make_unique<bool[]>(m_capacity);
}

std::size_t BoidContainer::GetSize() const noexcept
{
	return m_size;
}

std::size_t BoidContainer::GetCapacity() const noexcept
{
	return m_capacity;
}

void BoidContainer::Push(const sf::Vector2f& pos)
{
	Push(pos, sf::Vector2f(
		util::Random(-1.0f, 1.0f),
		util::Random(-1.0f, 1.0f)) * Config::Inst().Boids.SpeedMax);
}

void BoidContainer::Push(const sf::Vector2f& pos, const sf::Vector2f& velocity)
{
	if (m_size == m_capacity)
		Reallocate((std::size_t)(1.5 * m_capacity + 1));

	m_indices[m_size] = (std::uint32_t)m_size;

	m_prevVelocities[m_size] = m_velocities[m_size]	= velocity;

	m_positions[m_size] = pos;
	m_angles[m_size] = velocity.angle().asRadians();

	sf::Vector2f ori = GetOrigin(m_positions[m_size]);
	const float angle = m_angles[m_size];

	m_prevTriangles[m_size] = m_triangles[m_size] =
	{ 
		vu::RotatePoint({ ori.x + Config::Inst().BoidHalfSize.x, ori.y									}, ori, angle),
		vu::RotatePoint({ ori.x - Config::Inst().BoidHalfSize.x, ori.y - Config::Inst().BoidHalfSize.y }, ori, angle),
		vu::RotatePoint({ ori.x - Config::Inst().BoidHalfSize.x, ori.y + Config::Inst().BoidHalfSize.y }, ori, angle)
	};

	if (Config::Inst().Cycle.Random)
		m_cycleTimes[m_size] = util::Random(0.0f, 1.0f);

	++m_size;
}

void BoidContainer::Pop(std::size_t count)
{
	assert(count > 0); // pop nothing ???

	std::size_t oldSize = m_size;
	m_size = (count < m_size) ? (m_size - count) : 0;

	(void)std::remove_if(m_indices.get(), m_indices.get() + oldSize,
		[this](std::uint32_t i)
		{
			return i >= m_size; // push all older indices to the end
		});
}

void BoidContainer::Reallocate(std::size_t capacity)
{
	m_capacity = capacity;

	const auto realloc = 
		[this]<typename T>(std::unique_ptr<T[]>& ptr)
		{
			auto newPtr = std::make_unique<T[]>(m_capacity);
			std::move(ptr.get(), ptr.get() + m_size, newPtr.get());
			ptr = std::move(newPtr);
		};

	realloc(m_indices);
	realloc(m_triangles);
	realloc(m_prevTriangles);

	realloc(m_positions);
	realloc(m_velocities);
	realloc(m_prevVelocities);
	realloc(m_relativePositions);
	realloc(m_colors);

	realloc(m_speeds);
	realloc(m_angles);
	realloc(m_cycleTimes);
	realloc(m_densityTimes);

	realloc(m_densities);
	realloc(m_cellIndices);

	realloc(m_teleported);
}

void BoidContainer::Reserve(std::size_t capacity)
{
	if (capacity > m_capacity) // Only reserve size if the given size is larger than current capacity
		Reallocate(capacity);
}

void BoidContainer::PreUpdate(const Grid& grid)
{
	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_prevVelocities[i]	= m_velocities[i];
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_prevTriangles[i] = m_triangles[i];
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		const sf::Vector2f gridCellRaw		= grid.RelativePos(GetOrigin(m_positions[i]));
		const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
		const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

		m_relativePositions[i]	= gridCellOverflow * grid.GetContDims();
		m_cellIndices[i]		= (std::uint16_t)grid.AtPos(gridCell);
	}
}

void BoidContainer::Sort()
{
	std::sort(m_indices.get(), m_indices.get() + m_size,
		[this](std::uint32_t i0, std::uint32_t i1)
		{
			return m_cellIndices[i0] < m_cellIndices[i1];
		});
}

void BoidContainer::UpdateCells(Grid& grid)
{
	if (m_size == 0)
		return;

	grid.SetStartIndex(m_cellIndices[m_indices[0]], 0);

	for (std::size_t i = 1; i < m_size; ++i)
	{
		const std::uint16_t cellIndex	= m_cellIndices[m_indices[i]];
		const std::uint16_t otherIndex	= m_cellIndices[m_indices[i - 1]];

		if (otherIndex != cellIndex)
		{
			grid.SetStartIndex(cellIndex, (int)i);
			grid.SetEndIndex(otherIndex, (int)i - 1);
		}
	}

	grid.SetEndIndex(m_cellIndices[m_indices[m_size - 1]], (int)m_size - 1);
}

void BoidContainer::Interaction(const InputHandler& inputHandler, const sf::Vector2f& mousePos, float dt)
{
	const bool holdLeft		= inputHandler.GetButtonHeld(sf::Mouse::Button::Left);
	const bool holdRight	= inputHandler.GetButtonHeld(sf::Mouse::Button::Right);

	if (Config::Inst().Interaction.SteerEnabled && (holdLeft || holdRight))
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			sf::Vector2f dir = vu::Direction(m_positions[i], mousePos);

			const float factor = holdLeft ? 1.0f :
				(holdRight ? -1.0f : 0.0f);

			const float lengthOpt = vu::DistanceOpt(dir);
			const float weight = 1.0f / (std::sqrtf(lengthOpt) + FLT_EPSILON);

			SteerTowards(m_velocities[i], m_prevVelocities[i], dir, lengthOpt, Config::Inst().Interaction.SteerTowardsFactor * weight * factor * dt);
		}
	}
	else if (Config::Inst().Interaction.PredatorEnabled)
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			sf::Vector2f dir = vu::Direction(m_positions[i], mousePos);

			float lengthSqr = dir.lengthSq();
			if (lengthSqr <= Config::Inst().Interaction.PredatorDistance)
			{
				float weight = 1.0f / (std::sqrtf(lengthSqr / (Config::Inst().Interaction.PredatorDistance + FLT_EPSILON)) + FLT_EPSILON);
				SteerTowards(m_velocities[i], m_prevVelocities[i], dir, -Config::Inst().Interaction.PredatorFactor * weight * dt);
			}
		}
	}
}

void BoidContainer::Flock(const Grid& grid, Policy policy)
{
	PolicySelect([&](auto& pol)
		{
			std::for_each(pol, m_indices.get(), m_indices.get() + m_size,
				[&](std::uint32_t lhs)
				{
					sf::Vector2f sep;
					sf::Vector2f ali;
					sf::Vector2f coh;

					std::uint32_t sepCount = 0;
					std::uint32_t aliCount = 0;
					std::uint32_t cohCount = 0;

					const sf::Vector2f origin = GetOrigin(m_positions[lhs]);
					const sf::Vector2f thisRelative = m_relativePositions[lhs];
					const float thisAngle = m_angles[lhs];

					static constexpr auto neighbourCount = 4; // max 4 neighbours at a time

					int neighIndices[neighbourCount]{};
					sf::Vector2f neighbours[neighbourCount];

					const sf::Vector2f gridCellRaw		= grid.RelativePos(origin);
					const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
					const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

					const int x = (gridCellOverflow.x > 0.5f ? 1 : -1);
					const int y = (gridCellOverflow.y > 0.5f ? 1 : -1);

					const int neighbour_x = gridCell.x + x;
					const int neighbour_y = gridCell.y + y;

					neighbours[0] = grid.GetContDims() * sf::Vector2f(0,		0);
					neighbours[1] = grid.GetContDims() * sf::Vector2f((float)x,	0);
					neighbours[2] = grid.GetContDims() * sf::Vector2f(0,		(float)y);
					neighbours[3] = grid.GetContDims() * sf::Vector2f((float)x,	(float)y);

					neighIndices[0] = grid.AtPos(gridCell.x,	gridCell.y);	// current
					neighIndices[1] = grid.AtPos(neighbour_x,	gridCell.y);	// left or right of current
					neighIndices[2] = grid.AtPos(gridCell.x,	neighbour_y);	// top or bot of current
					neighIndices[3] = grid.AtPos(neighbour_x,	neighbour_y);	// top left/right bot left/right of current

					Config& config = Config::Inst();

					const float cohDistance = config.Rules.CohDistance;
					const float aliDistance = config.Rules.AliDistance;
					const float sepDistance = config.Rules.SepDistance;

					const float negFOV = -config.Boids.ViewAngle;
					const float posFOV =  config.Boids.ViewAngle;

					for (int i = 0; i < neighbourCount; ++i)
					{
						const int gridCellIndex = neighIndices[i];
						const int start = grid.GetStartIndices()[gridCellIndex];

						if (start == -1)
							continue;

						const int end = grid.GetEndIndices()[gridCellIndex];

						const sf::Vector2f neighbourCell = neighbours[i];
						const sf::Vector2f cellRel = neighbourCell - thisRelative;

						for (int j = start; j <= end; ++j) // do in one loop
						{
							const auto rhs = m_indices[j];

							if (lhs == rhs)
								continue;

							const sf::Vector2f dir	= cellRel + m_relativePositions[rhs];
							const float distanceSqr = dir.lengthSq();

							const bool withinCohesion	= distanceSqr < cohDistance;
							const bool withinAlignment	= distanceSqr < aliDistance;

							const std::uint8_t flag = (static_cast<std::uint8_t>(withinCohesion) | static_cast<std::uint8_t>(withinAlignment) << 1);

							switch (flag)
							{
								[[unlikely]] case 1U: // cohesion
								{
									const float angle		= vu::PI<> - std::abs(std::abs(vu::Angle(dir.y, dir.x) - thisAngle) - vu::PI<>);
									const bool withinFOV	= angle > negFOV && angle < posFOV;

									coh += dir * (float)withinFOV; // Head towards center of boids
									cohCount += withinFOV;

									break; 
								}
								[[unlikely]] case 2U: // alignment
								{
									const float angle		= vu::PI<> - std::abs(std::abs(vu::Angle(dir.y, dir.x) - thisAngle) - vu::PI<>);
									const bool withinFOV	= angle > negFOV && angle < posFOV;

									ali += m_prevVelocities[rhs] * (float)withinFOV; // Align with every boids velocity
									aliCount += withinFOV;

									break;
								}
								[[likely]] case 3U: // both
								{
									const float angle		= vu::PI<> - std::abs(std::abs(vu::Angle(dir.y, dir.x) - thisAngle) - vu::PI<>);
									const bool withinFOV	= angle > negFOV && angle < posFOV;

									coh += dir * (float)withinFOV;
									cohCount += withinFOV;

									ali += m_prevVelocities[rhs] * (float)withinFOV;
									aliCount += withinFOV;

									break;
								}
							}

							if (distanceSqr < sepDistance)
							{
								sep += -dir / (distanceSqr ? distanceSqr : FLT_EPSILON);
								++sepCount;
							}
						}
					}

					if (cohCount) m_velocities[lhs] += SteerAt(m_prevVelocities[lhs], vu::Normalize(coh, config.Boids.SpeedMax)) * config.Rules.CohWeight;
					if (aliCount) m_velocities[lhs] += SteerAt(m_prevVelocities[lhs], vu::Normalize(ali / (float)aliCount, config.Boids.SpeedMax)) * config.Rules.AliWeight;
					if (sepCount) m_velocities[lhs] += SteerAt(m_prevVelocities[lhs], vu::Normalize(sep / (float)sepCount, config.Boids.SpeedMax)) * config.Rules.SepWeight;

					m_densities[lhs] = std::max(std::max(cohCount, aliCount), sepCount);
				});
		}, policy);
}

void BoidContainer::Update(const RectFloat& border, const std::vector<Impulse>& impulses, float dt)
{
	for (std::size_t i = 0; i < m_size; ++i)
	{
		auto& velocity	= m_velocities[i];
		auto& speed		= m_speeds[i];

		float lengthSq = m_velocities[i].lengthSq();

		if (lengthSq < Config::Inst().BoidSpeedMinSq)
		{
			velocity = vu::Normalize(velocity, std::sqrt(lengthSq), Config::Inst().Boids.SpeedMin);
			speed = Config::Inst().Boids.SpeedMin;
		}
		else if (lengthSq > Config::Inst().BoidSpeedMaxSq)
		{
			velocity = vu::Normalize(velocity, std::sqrt(lengthSq), Config::Inst().Boids.SpeedMax);
			speed = Config::Inst().Boids.SpeedMax;
		}
		else
		{
			speed = std::sqrt(lengthSq);
		}
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_positions[i] += m_velocities[i] * dt;
	}
	
	if (Config::Inst().Interaction.TurnAtBorder)
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			TurnAtBorder(m_positions[i], m_velocities[i], m_densities[i], border, dt);
		}
	}
	else
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			m_teleported[i] = TeleportAtBorder(m_positions[i], border);
		}
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		auto& velocity = m_velocities[i];
		auto& angle = m_angles[i];

		angle = (velocity.x || velocity.y) ?
			vu::Angle(velocity.y, velocity.x) : 0.0f;
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		auto& tri = m_triangles[i];

		const auto ori = GetOrigin(m_positions[i]);
		const auto angle = m_angles[i];

		tri =
		{
			vu::RotatePoint({ ori.x + Config::Inst().BoidHalfSize.x, ori.y								   }, ori, angle), // middle right tip
			vu::RotatePoint({ ori.x - Config::Inst().BoidHalfSize.x, ori.y - Config::Inst().BoidHalfSize.y }, ori, angle), // top left corner
			vu::RotatePoint({ ori.x - Config::Inst().BoidHalfSize.x, ori.y + Config::Inst().BoidHalfSize.y }, ori, angle)	// bot left corner
		};
	}

	if (!Config::Inst().Interaction.TurnAtBorder)
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			if (m_teleported[i])
				m_prevTriangles[i] = m_triangles[i];
		}
	}

	if ((Config::Inst().Color.Flags & CF_Cycle) == CF_Cycle)
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_cycleTimes[i] = std::fmodf(m_cycleTimes[i] + dt * Config::Inst().Cycle.Speed, 1.0f);
	}

	if ((Config::Inst().Color.Flags & CF_Density) == CF_Density)
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_densityTimes[i] = (Config::Inst().Density.DensityCycleEnabled) ? std::fmodf(m_densityTimes[i] + dt * Config::Inst().Density.DensityCycleSpeed, 1.0f) : 0.0f;
	}

	if (Config::Inst().Impulse.Force != 0.0f)
	{
		for (const Impulse& impulse : impulses)
		{
			for (std::size_t i = 0; i < m_size; ++i)
			{
				const sf::Vector2f impulsePos = impulse.GetPosition();
				const float impulseLength = impulse.GetLength();

				const float length = vu::Distance(m_positions[i], impulsePos);
				const float diff = std::abs(length - impulseLength);

				const float percentage = (impulseLength / Config::Inst().Impulse.FadeDistance);
				const float size = impulse.GetSize() * (1.0f - percentage);

				if (diff <= size)
				{
					SteerTowards(m_velocities[i], m_prevVelocities[i],
						vu::Direction(impulsePos, m_positions[i]), Config::Inst().Impulse.Force * (1.0f - percentage) * dt);
				}
			}
		}
	}
}

void BoidContainer::UpdateColors(const RectFloat& border, const Fluid& fluid, const IAudioMeterInfo* audioMeter, const std::vector<Impulse>& impulses)
{
	Config& config = Config::Inst();
	std::uint32_t flag = config.Color.Flags;

	std::fill_n(m_colors.get(), m_size, sf::Vector3f());

	if ((flag & CF_Positional) == CF_Positional)
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += PositionColor(m_positions[i], border) * config.Color.PositionalWeight;
	}
	if ((flag & CF_Cycle) == CF_Cycle && !config.Cycle.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += CycleColor(m_cycleTimes[i]) * config.Color.CycleWeight;
	}
	if ((flag & CF_Density) == CF_Density && !config.Density.Colors.empty() && config.Density.Density > 0)
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += DensityColor(m_densities[i], m_densityTimes[i]) * config.Color.DensityWeight;
	}
	if ((flag & CF_Velocity) == CF_Velocity && !config.Velocity.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += VelocityColor(m_speeds[i]) * config.Color.VelocityWeight;
	}
	if ((flag & CF_Rotation) == CF_Rotation && !config.Rotation.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += RotationColor(m_angles[i]) * config.Color.RotationWeight;
	}
	if ((flag & CF_Audio) == CF_Audio && !config.Audio.Colors.empty() && audioMeter != nullptr)
	{
		float volume = std::fminf(audioMeter->GetVolume() * Config::Inst().Audio.Strength, Config::Inst().Audio.Limit);

		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += AudioColor(m_densities[i], volume) * config.Color.AudioWeight;
	}
	if ((flag & CF_Fluid) == CF_Fluid && !config.Fluid.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += fluid.GetColor(GetOrigin(m_positions[i]));
	}

	if (!config.Impulse.Colors.empty() && !impulses.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			for (const Impulse& impulse : impulses)
				ImpulseColor(m_positions[i], m_colors[i], impulse);
		}
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_colors[i] =
		{
			std::clamp(m_colors[i].x, 0.0f, 1.0f),
			std::clamp(m_colors[i].y, 0.0f, 1.0f),
			std::clamp(m_colors[i].z, 0.0f, 1.0f)
		};
	}
}

void BoidContainer::UpdateVertices(sf::VertexArray& vertices, float interp, Policy policy)
{
	PolicySelect(
		[this, &vertices, interp](auto& pol)
		{
			std::for_each(pol, m_indices.get(), m_indices.get() + m_size,
				[this, &vertices, interp](std::uint32_t i)
				{
					const auto& tri			= m_triangles[i];
					const auto& prevTri		= m_prevTriangles[i];

					const Triangle drawTri 
					{
						(tri.v0 * interp + prevTri.v0 * (1.0f - interp)),
						(tri.v1 * interp + prevTri.v1 * (1.0f - interp)),
						(tri.v2 * interp + prevTri.v2 * (1.0f - interp))
					};

					const sf::Vector3f& color = m_colors[i];

					const sf::Color c = sf::Color(
						(std::uint8_t)(color.x * 255.999f),
						(std::uint8_t)(color.y * 255.999f),
						(std::uint8_t)(color.z * 255.999f));

					const std::size_t v = (std::size_t)i * 3;

					vertices[v + 0].position = drawTri.v0;
					vertices[v + 1].position = drawTri.v1;
					vertices[v + 2].position = drawTri.v2;

					vertices[v + 0].color = c;
					vertices[v + 1].color = c;
					vertices[v + 2].color = c;
				});
		}, policy);
}

void BoidContainer::TurnAtBorder(const sf::Vector2f& pos, sf::Vector2f& vel, std::uint32_t den, const RectFloat& border, float dt)
{
	const float maxSize = std::max(Config::Inst().Boids.Width, Config::Inst().Boids.Height);

	float widthMargin	= border.width - (border.width * Config::Inst().Interaction.TurnMarginFactor);
	float heightMargin	= border.width - (border.height * Config::Inst().Interaction.TurnMarginFactor);

	const float leftMargin	= border.left + widthMargin;
	const float topMargin	= border.top + heightMargin;
	const float rightMargin = border.Right() - widthMargin;
	const float botMargin	= border.Bottom() - heightMargin;

	widthMargin		= std::max(widthMargin, 1.0f);
	heightMargin	= std::max(heightMargin, 1.0f);

	if (pos.x + maxSize < leftMargin)
		vel.x += Config::Inst().Interaction.TurnFactor * std::powf(std::abs(pos.x - leftMargin) / widthMargin, 2.0f) * (1.0f / (den + 1.0f)) * dt;

	if (pos.x > rightMargin)
		vel.x -= Config::Inst().Interaction.TurnFactor * std::powf(std::abs(pos.x - rightMargin) / widthMargin, 2.0f) * (1.0f / (den + 1.0f)) * dt;

	if (pos.y + maxSize < topMargin)
		vel.y += Config::Inst().Interaction.TurnFactor * std::powf(std::abs(pos.y - topMargin) / heightMargin, 2.0f) * (1.0f / (den + 1.0f)) * dt;

	if (pos.y > botMargin)
		vel.y -= Config::Inst().Interaction.TurnFactor * std::powf(std::abs(pos.y - botMargin) / heightMargin, 2.0f) * (1.0f / (den + 1.0f)) * dt;
}

bool BoidContainer::TeleportAtBorder(sf::Vector2f& pos, const RectFloat& border)
{
	const float maxSize = std::max(
		Config::Inst().Boids.Width, 
		Config::Inst().Boids.Height);

	const sf::Vector2f previous = pos;

	if (pos.x + maxSize < border.left)
		pos.x = (float)border.Right();

	if (pos.x > border.Right())
		pos.x = border.left - maxSize;

	if (pos.y + maxSize < border.top)
		pos.y = (float)border.Bottom();

	if (pos.y > border.Bottom())
		pos.y = border.top - maxSize;

	return (previous != pos);
}

sf::Vector2f BoidContainer::SteerAt(const sf::Vector2f& prevVel, const sf::Vector2f& steerDir)
{
	return vu::Limit(vu::Direction(prevVel, steerDir), Config::Inst().Boids.SteerMax);
}

void BoidContainer::SteerTowards(sf::Vector2f& vel, const sf::Vector2f& prevVel, const sf::Vector2f& direction, float length, float weight)
{
	if (std::abs(weight) < FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::Normalize(direction, length, Config::Inst().Boids.SpeedMax);

	vel += SteerAt(prevVel, steer) * weight;
}

void BoidContainer::SteerTowards(sf::Vector2f& vel, const sf::Vector2f& prevVel, const sf::Vector2f& point, float weight)
{
	SteerTowards(vel, prevVel, point, point.length(), weight);
}

void BoidContainer::ResetCycleTimes()
{
	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_cycleTimes[i] = Config::Inst().Cycle.Random ?
			util::Random(0.0f, 1.0f) : 0.0f;
	}
}

sf::Vector2f BoidContainer::GetOrigin(const sf::Vector2f& pos)
{
	return pos + Config::Inst().BoidHalfSize;
}

sf::Vector3f BoidContainer::PositionColor(const sf::Vector2f& pos, const RectFloat& border)
{
	const float t = pos.x / border.width;
	const float s = pos.y / border.height;

	return sf::Vector3f(
		util::Interpolate(Config::Inst().Positional.TopLeft.x * 255.999f, Config::Inst().Positional.TopRight.x * 255.999f, Config::Inst().Positional.BotLeft.x * 255.999f, Config::Inst().Positional.BotRight.x * 255.999f, t, s) / 255.999f,
		util::Interpolate(Config::Inst().Positional.TopLeft.y * 255.999f, Config::Inst().Positional.TopRight.y * 255.999f, Config::Inst().Positional.BotLeft.y * 255.999f, Config::Inst().Positional.BotRight.y * 255.999f, t, s) / 255.999f,
		util::Interpolate(Config::Inst().Positional.TopLeft.z * 255.999f, Config::Inst().Positional.TopRight.z * 255.999f, Config::Inst().Positional.BotLeft.z * 255.999f, Config::Inst().Positional.BotRight.z * 255.999f, t, s) / 255.999f);
}

sf::Vector3f BoidContainer::CycleColor(float cycleTime)
{
	float scaledTime = cycleTime * (float)(Config::Inst().Cycle.Colors.size() - 1);

	const auto i1 = (int)scaledTime;
	const auto i2 = (i1 == (int)Config::Inst().Cycle.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Cycle.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Cycle.Colors[i2];

	const float newT = scaledTime - std::floorf(scaledTime);

	return vu::Lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::DensityColor(std::uint32_t density, float densityTime)
{
	const float densityPercentage = (density / (float)Config::Inst().Density.Density);

	const float scaledDensity = std::fmodf(densityPercentage + densityTime, 1.0f) * (float)(Config::Inst().Density.Colors.size() - 1);

	const auto i1 = (int)scaledDensity;
	const auto i2 = (i1 == (int)Config::Inst().Density.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Density.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Density.Colors[i2];

	const float newT = scaledDensity - std::floorf(scaledDensity);

	return vu::Lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::VelocityColor(float speed)
{
	const float velocity_percentage = std::clamp((speed - Config::Inst().Boids.SpeedMin) * Config::Inst().BoidSpeedInv, 0.0f, 1.0f);

	const float scaledVlocity = velocity_percentage * (float)(Config::Inst().Velocity.Colors.size() - 1);

	const auto i1 = (int)scaledVlocity;
	const auto i2 = (i1 == (int)Config::Inst().Velocity.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Velocity.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Velocity.Colors[i2];

	const float newT = scaledVlocity - std::floorf(scaledVlocity);

	return vu::Lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::RotationColor(float angle)
{
	const float rotationPercentage = (angle + float(M_PI)) / (2.0f * float(M_PI));

	const float scaledRotation = rotationPercentage * (float)(Config::Inst().Rotation.Colors.size() - 1);

	const auto i1 = (int)scaledRotation;
	const auto i2 = (i1 == (int)Config::Inst().Rotation.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Rotation.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Rotation.Colors[i2];

	const float newT = scaledRotation - std::floorf(scaledRotation);

	return vu::Lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::AudioColor(std::uint32_t density, float volume)
{
	const float densityPercentage = (density / (float)Config::Inst().Audio.Density);

	const float scaledVolume = std::fminf(volume * densityPercentage, 1.0f) * (float)(Config::Inst().Audio.Colors.size() - 1);

	const auto i1 = (int)scaledVolume;
	const auto i2 = (i1 == (int)Config::Inst().Audio.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Audio.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Audio.Colors[i2];

	const float newT = scaledVolume - std::floorf(scaledVolume);

	return vu::Lerp(color1, color2, newT);
}

void BoidContainer::ImpulseColor(const sf::Vector2f& pos, sf::Vector3f& color, const Impulse& impulse)
{
	const sf::Vector2f impulsePos = impulse.GetPosition();
	const float impulseLength = impulse.GetLength();

	const float length = vu::Distance(pos, impulsePos);
	const float diff = std::abs(length - impulseLength);

	const float percentage = (impulseLength / Config::Inst().Impulse.FadeDistance);
	const float size = impulse.GetSize() * (1.0f - percentage);

	if (diff <= size)
	{
		const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::Inst().Impulse.Colors.size() - 1);

		const auto i1 = (int)scaled_length;
		const auto i2 = (i1 == (int)Config::Inst().Impulse.Colors.size() - 1) ? 0 : i1 + 1;

		const sf::Vector3f color1 = Config::Inst().Impulse.Colors[i1];
		const sf::Vector3f color2 = Config::Inst().Impulse.Colors[i2];

		const float newT = scaled_length - std::floorf(scaled_length);

		color = vu::Lerp(color1, color2, newT);
	}
}
