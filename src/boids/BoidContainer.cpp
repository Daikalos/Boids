#include "BoidContainer.h"

#include <ranges>

#include "../utilities/VectorUtilities.h"
#include "../utilities/Utilities.h"

#include "Config.h"

BoidContainer::BoidContainer(std::size_t capacity) : m_capacity(capacity)
{
	m_indices			= std::make_unique<std::uint32_t[]>(m_capacity);

	m_positions			= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_prevPositions		= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_velocities		= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_prevVelocities	= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_relativePositions = std::make_unique<sf::Vector2f[]>(m_capacity);
	m_colors			= std::make_unique<sf::Vector3f[]>(m_capacity);

	m_speeds			= std::make_unique<float[]>(m_capacity);
	m_angles			= std::make_unique<float[]>(m_capacity);
	m_prevAngles		= std::make_unique<float[]>(m_capacity);
	m_cycleTimes		= std::make_unique<float[]>(m_capacity);
	m_densityTimes		= std::make_unique<float[]>(m_capacity);

	m_densities			= std::make_unique<std::uint32_t[]>(m_capacity);
	m_cellIndices		= std::make_unique<std::uint16_t[]>(m_capacity);
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
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)) * Config::Inst().Boids.SpeedMax);
}

void BoidContainer::Push(const sf::Vector2f& pos, const sf::Vector2f& velocity)
{
	if (m_size == m_capacity)
		Reallocate(1.5 * m_capacity + 1);

	m_indices[m_size] = m_size;

	m_prevPositions[m_size]		= m_positions[m_size]	= pos;
	m_prevVelocities[m_size]	= m_velocities[m_size]	= velocity;
	m_prevAngles[m_size]		= m_angles[m_size]		= velocity.angle().asRadians();

	if (Config::Inst().Cycle.Random)
		m_cycleTimes[m_size] = util::random(0.0f, 1.0f);

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
	const auto realloc = 
		[this, capacity]<typename T>(std::unique_ptr<T[]>& ptr)
		{
			auto newPtr = std::make_unique<T[]>(capacity);
			std::move(ptr.get(), ptr.get() + m_size, newPtr.get());
			ptr = std::move(newPtr);
		};

	realloc(m_indices);

	realloc(m_positions);
	realloc(m_prevPositions);
	realloc(m_velocities);
	realloc(m_prevVelocities);
	realloc(m_relativePositions);
	realloc(m_colors);

	realloc(m_speeds);
	realloc(m_angles);
	realloc(m_prevAngles);
	realloc(m_cycleTimes);
	realloc(m_densityTimes);

	realloc(m_densities);
	realloc(m_cellIndices);

	m_capacity = capacity;
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
		m_prevPositions[i]	= m_positions[i];
		m_prevVelocities[i]	= m_velocities[i];
		m_prevAngles[i]		= m_angles[i];

		const sf::Vector2f gridCellRaw		= grid.RelativePos(GetOrigin(m_positions[i]));
		const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
		const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

		m_relativePositions[i]	= gridCellOverflow * grid.GetContDims();
		m_cellIndices[i]		= grid.AtPos(gridCell);
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
			grid.SetStartIndex(cellIndex, i);
			grid.SetEndIndex(otherIndex, i - 1);
		}
	}

	grid.SetEndIndex(m_cellIndices[m_indices[m_size - 1]], m_size - 1);
}

void BoidContainer::Interaction(const InputHandler& inputHandler, const sf::Vector2f& mousePos, float dt)
{
	const bool holdLeft		= inputHandler.GetButtonHeld(sf::Mouse::Button::Left);
	const bool holdRight	= inputHandler.GetButtonHeld(sf::Mouse::Button::Right);

	if (Config::Inst().Interaction.SteerEnabled && (holdLeft || holdRight))
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			sf::Vector2f dir = vu::direction(m_positions[i], mousePos);

			const float factor = holdLeft ? 1.0f :
				(holdRight ? -1.0f : 0.0f);

			const float lengthOpt = vu::distance_opt(dir);
			const float weight = 1.0f / (std::sqrtf(lengthOpt) + FLT_EPSILON);

			SteerTowards(i, dir, lengthOpt, Config::Inst().Interaction.SteerTowardsFactor * weight * factor * dt);
		}
	}
	else if (Config::Inst().Interaction.PredatorEnabled)
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			sf::Vector2f dir = vu::direction(m_positions[i], mousePos);

			float lengthSqr = dir.lengthSq();
			if (lengthSqr <= Config::Inst().Interaction.PredatorDistance)
			{
				float weight = 1.0f / (std::sqrtf(lengthSqr / (Config::Inst().Interaction.PredatorDistance + FLT_EPSILON)) + FLT_EPSILON);
				SteerTowards(i, dir, -Config::Inst().Interaction.PredatorFactor * weight * dt);
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
					const float thisAngle = m_prevAngles[lhs];

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

					for (std::uint8_t i = 0; i < neighbourCount; ++i)
					{
						const int gridCellIndex = neighIndices[i];
						const int start = grid.GetStartIndices()[gridCellIndex];

						if (start == -1)
							continue;

						const int end = grid.GetEndIndices()[gridCellIndex];
						const sf::Vector2f neighbourCell = neighbours[i];

						for (int j = start; j <= end; ++j) // do in one loop
						{
							if (lhs == m_indices[j])
								continue;

							const sf::Vector2f dir = (neighbourCell + m_relativePositions[m_indices[j]]) - thisRelative;
							const float distanceSqr = dir.lengthSq();

							const bool withinCohesion	= distanceSqr < cohDistance;
							const bool withinAlignment	= distanceSqr < aliDistance;

							if (withinCohesion || withinAlignment)
							{
								const float angle = vu::PI<> - std::abs(std::abs(vu::angle(dir.y, dir.x) - thisAngle) - vu::PI<>);
								if (angle > negFOV && angle < posFOV)
								{
									if (withinCohesion) [[likely]]
									{
										coh += dir; // Head towards center of boids
										++cohCount;
									}
									if (withinAlignment) [[likely]]
									{
										ali += m_prevVelocities[m_indices[j]]; // Align with every boids velocity
										++aliCount;
									}
								}
							}

							if (distanceSqr < sepDistance)
							{
								sep += -dir / (distanceSqr ? distanceSqr : FLT_EPSILON);
								++sepCount;
							}
						}
					}

					if (cohCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(coh, config.Boids.SpeedMax)) * config.Rules.CohWeight;
					if (aliCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(ali / static_cast<float>(aliCount), config.Boids.SpeedMax)) * config.Rules.AliWeight;
					if (sepCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(sep / static_cast<float>(sepCount), config.Boids.SpeedMax)) * config.Rules.SepWeight;

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
			velocity = vu::normalize(velocity, std::sqrt(lengthSq), Config::Inst().Boids.SpeedMin);
			speed = Config::Inst().Boids.SpeedMin;
		}
		else if (lengthSq > Config::Inst().BoidSpeedMaxSq)
		{
			velocity = vu::normalize(velocity, std::sqrt(lengthSq), Config::Inst().Boids.SpeedMax);
			speed = Config::Inst().Boids.SpeedMax;
		}
		else
		{
			speed = std::sqrt(lengthSq);
		}
	}

	for (std::size_t i = 0; i < m_size; ++i)
	{
		auto& position		= m_positions[i];
		auto& prevPosition	= m_prevPositions[i];
		auto& velocity		= m_velocities[i];
		auto& angle			= m_angles[i];
		
		position += velocity * dt;

		if (OutsideBorder(i, border, dt))
			prevPosition = position; // prevent interpolation effect

		angle = (velocity.x && velocity.y) ? 
			vu::angle(m_velocities[i].y, m_velocities[i].x) : 0.0f;
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

	if (!impulses.empty() && Config::Inst().Impulse.Force != 0.0f)
	{
		for (std::size_t i = 0; i < m_size; ++i)
		{
			for (const Impulse& impulse : impulses)
			{
				const sf::Vector2f impulsePos = impulse.GetPosition();
				const float impulseLength = impulse.GetLength();

				const float length = vu::distance(m_positions[i], impulsePos);
				const float diff = std::abs(length - impulseLength);

				const float percentage = (impulseLength / Config::Inst().Impulse.FadeDistance);
				const float size = impulse.GetSize() * (1.0f - percentage);

				if (diff <= size)
					SteerTowards(i, vu::direction(impulsePos, m_positions[i]), Config::Inst().Impulse.Force * (1.0f - percentage) * dt);
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
			m_colors[i] += PositionColor(i, border) * config.Color.PositionalWeight;
	}
	if ((flag & CF_Cycle) == CF_Cycle && !config.Cycle.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += CycleColor(i) * config.Color.CycleWeight;
	}
	if ((flag & CF_Density) == CF_Density && !config.Density.Colors.empty() && config.Density.Density > 0)
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += DensityColor(i) * config.Color.DensityWeight;
	}
	if ((flag & CF_Velocity) == CF_Velocity && !config.Velocity.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += VelocityColor(i) * config.Color.VelocityWeight;
	}
	if ((flag & CF_Rotation) == CF_Rotation && !config.Rotation.Colors.empty())
	{
		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += RotationColor(i) * config.Color.RotationWeight;
	}
	if ((flag & CF_Audio) == CF_Audio && !config.Audio.Colors.empty() && audioMeter != nullptr)
	{
		float volume = std::fminf(audioMeter->GetVolume() * Config::Inst().Audio.Strength, Config::Inst().Audio.Limit);

		for (std::size_t i = 0; i < m_size; ++i)
			m_colors[i] += AudioColor(i, volume) * config.Color.AudioWeight;
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
				ImpulseColor(i, m_colors[i], impulse);
		}
	}
}

void BoidContainer::UpdateVertices(sf::VertexArray& vertices, Policy policy, float interp)
{
	PolicySelect([this, &vertices, &interp](auto& pol)
		{
			std::for_each(pol, m_indices.get(), m_indices.get() + m_size,
				[this, &vertices, &interp](std::uint32_t i)
				{
					Config& config = Config::Inst();

					const sf::Vector2f ori		= GetOrigin(m_positions[i]);
					const sf::Vector2f prevOri	= GetOrigin(m_prevPositions[i]);

					const float& rot		= m_angles[i];
					const float& prevRot	= m_prevAngles[i];

					const sf::Vector2f pointA = vu::rotate_point({ ori.x + config.BoidHalfSize.x, ori.y							}, ori, rot); // middle right tip
					const sf::Vector2f pointB = vu::rotate_point({ ori.x - config.BoidHalfSize.x, ori.y - config.BoidHalfSize.y }, ori, rot); // top left corner
					const sf::Vector2f pointC = vu::rotate_point({ ori.x - config.BoidHalfSize.x, ori.y + config.BoidHalfSize.y }, ori, rot); // bot left corner

					const sf::Vector2f prevPointA = vu::rotate_point({ prevOri.x + config.BoidHalfSize.x, prevOri.y							}, prevOri, prevRot); // middle right tip
					const sf::Vector2f prevPointB = vu::rotate_point({ prevOri.x - config.BoidHalfSize.x, prevOri.y - config.BoidHalfSize.y }, prevOri, prevRot); // top left corner
					const sf::Vector2f prevPointC = vu::rotate_point({ prevOri.x - config.BoidHalfSize.x, prevOri.y + config.BoidHalfSize.y }, prevOri, prevRot); // bot left corner

					const sf::Vector2f p0 = pointA * interp + prevPointA * (1.0f - interp);
					const sf::Vector2f p1 = pointB * interp + prevPointB * (1.0f - interp);
					const sf::Vector2f p2 = pointC * interp + prevPointC * (1.0f - interp);

					sf::Vector3f bc = m_colors[i];

					bc.x = std::clamp(bc.x, 0.0f, 1.0f);
					bc.y = std::clamp(bc.y, 0.0f, 1.0f);
					bc.z = std::clamp(bc.z, 0.0f, 1.0f);

					const sf::Color c = sf::Color(
						static_cast<sf::Uint8>(bc.x * 255.0f), 
						static_cast<sf::Uint8>(bc.y * 255.0f),
						static_cast<sf::Uint8>(bc.z * 255.0f));

					const std::size_t v = static_cast<std::size_t>(i) * 3;

					vertices[v + 0].position = p0;
					vertices[v + 1].position = p1;
					vertices[v + 2].position = p2;

					vertices[v + 0].color = c;
					vertices[v + 1].color = c;
					vertices[v + 2].color = c;
				});
		}, policy);
}

bool BoidContainer::OutsideBorder(std::uint32_t i, const RectFloat& border, float dt)
{
	return Config::Inst().Interaction.TurnAtBorder ? TurnAtBorder(i, border, dt) : TeleportAtBorder(i, border);
}

bool BoidContainer::TurnAtBorder(std::uint32_t i, const RectFloat& border, float dt)
{
	const float maxSize = std::max(Config::Inst().Boids.Width, Config::Inst().Boids.Height);

	float widthMargin = border.width() - border.width() * Config::Inst().Interaction.TurnMarginFactor;
	float heightMargin = border.height() - border.height() * Config::Inst().Interaction.TurnMarginFactor;

	const float leftMargin = border.left + widthMargin;
	const float topMargin = border.top + heightMargin;
	const float rightMargin = border.right - widthMargin;
	const float botMargin = border.bot - heightMargin;

	if (widthMargin == 0.0f)	widthMargin = 1.0f;
	if (heightMargin == 0.0f)	heightMargin = 1.0f;

	if (m_positions[i].x + maxSize < leftMargin)
		m_velocities[i].x += Config::Inst().Interaction.TurnFactor * std::powf(std::abs(m_positions[i].x - leftMargin) / widthMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].x > rightMargin)
		m_velocities[i].x -= Config::Inst().Interaction.TurnFactor * std::powf(std::abs(m_positions[i].x - rightMargin) / widthMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].y + maxSize < topMargin)
		m_velocities[i].y += Config::Inst().Interaction.TurnFactor * std::powf(std::abs(m_positions[i].y - topMargin) / heightMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].y > botMargin)
		m_velocities[i].y -= Config::Inst().Interaction.TurnFactor * std::powf(std::abs(m_positions[i].y - botMargin) / heightMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	return false;
}

bool BoidContainer::TeleportAtBorder(std::uint32_t i, const RectFloat& border)
{
	const float maxSize = std::max(Config::Inst().Boids.Width, Config::Inst().Boids.Height);
	const sf::Vector2f previous = m_positions[i];

	if (m_positions[i].x + maxSize < border.left)
		m_positions[i].x = (float)border.right;

	if (m_positions[i].x > border.right)
		m_positions[i].x = border.left - maxSize;

	if (m_positions[i].y + maxSize < border.top)
		m_positions[i].y = (float)border.bot;

	if (m_positions[i].y > border.bot)
		m_positions[i].y = border.top - maxSize;

	return (previous != m_positions[i]);
}

sf::Vector2f BoidContainer::SteerAt(std::uint32_t i, const sf::Vector2f& steerDir) const
{
	return vu::limit(vu::direction(m_prevVelocities[i], steerDir), Config::Inst().Boids.SteerMax);
}

void BoidContainer::SteerTowards(std::uint32_t i, const sf::Vector2f& direction, float length, float weight)
{
	if (std::abs(weight) < FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::normalize(direction, length, Config::Inst().Boids.SpeedMax);

	m_velocities[i] += SteerAt(i, steer) * weight;
}

void BoidContainer::SteerTowards(std::uint32_t i, const sf::Vector2f& point, float weight)
{
	SteerTowards(i, point, point.length(), weight);
}

void BoidContainer::ResetCycleTimes()
{
	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_cycleTimes[i] = Config::Inst().Cycle.Random ?
			util::random(0.0f, 1.0f) : 0.0f;
	}
}

sf::Vector2f BoidContainer::GetOrigin(const sf::Vector2f& pos) const
{
	return pos + Config::Inst().BoidHalfSize;
}

sf::Vector3f BoidContainer::PositionColor(std::uint32_t i, const RectFloat& border) const
{
	const float t = m_positions[i].x / border.width();
	const float s = m_positions[i].y / border.height();

	return sf::Vector3f(
		util::interpolate(Config::Inst().Positional.TopLeft.x * 255.0f, Config::Inst().Positional.TopRight.x * 255.0f, Config::Inst().Positional.BotLeft.x * 255.0f, Config::Inst().Positional.BotRight.x * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().Positional.TopLeft.y * 255.0f, Config::Inst().Positional.TopRight.y * 255.0f, Config::Inst().Positional.BotLeft.y * 255.0f, Config::Inst().Positional.BotRight.y * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().Positional.TopLeft.z * 255.0f, Config::Inst().Positional.TopRight.z * 255.0f, Config::Inst().Positional.BotLeft.z * 255.0f, Config::Inst().Positional.BotRight.z * 255.0f, t, s) / 255.0f);
}

sf::Vector3f BoidContainer::CycleColor(std::uint32_t i) const
{
	float scaledTime = m_cycleTimes[i] * (float)(Config::Inst().Cycle.Colors.size() - 1);

	const auto i1 = (int)scaledTime;
	const auto i2 = (i1 == Config::Inst().Cycle.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Cycle.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Cycle.Colors[i2];

	const float newT = scaledTime - std::floorf(scaledTime);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::DensityColor(std::uint32_t i) const
{
	const float densityPercentage = (m_densities[i] / (float)Config::Inst().Density.Density);

	const float scaledDensity = std::fmodf(densityPercentage + m_densityTimes[i], 1.0f) * (float)(Config::Inst().Density.Colors.size() - 1);

	const auto i1 = (int)scaledDensity;
	const auto i2 = (i1 == Config::Inst().Density.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Density.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Density.Colors[i2];

	const float newT = scaledDensity - std::floorf(scaledDensity);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::VelocityColor(std::uint32_t i) const
{
	const float velocity_percentage = std::clamp((m_speeds[i] - Config::Inst().Boids.SpeedMin) * Config::Inst().BoidSpeedInv, 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(Config::Inst().Velocity.Colors.size() - 1);

	const auto i1 = (int)scaled_velocity;
	const auto i2 = (i1 == Config::Inst().Velocity.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Velocity.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Velocity.Colors[i2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::RotationColor(std::uint32_t i) const
{
	const float rotationPercentage = (m_angles[i] + float(M_PI)) / (2.0f * float(M_PI));

	const float scaledRotation = rotationPercentage * (float)(Config::Inst().Rotation.Colors.size() - 1);

	const auto i1 = (int)scaledRotation;
	const auto i2 = (i1 == Config::Inst().Rotation.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Rotation.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Rotation.Colors[i2];

	const float newT = scaledRotation - std::floorf(scaledRotation);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::AudioColor(std::uint32_t i, float volume) const
{
	const float densityPercentage = (m_densities[i] / (float)Config::Inst().Audio.Density);

	const float scaledVolume = std::fminf(volume * densityPercentage, 1.0f) * (float)(Config::Inst().Audio.Colors.size() - 1);

	const auto i1 = (int)scaledVolume;
	const auto i2 = (i1 == Config::Inst().Audio.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Audio.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Audio.Colors[i2];

	const float newT = scaledVolume - std::floorf(scaledVolume);

	return vu::lerp(color1, color2, newT);
}

void BoidContainer::ImpulseColor(std::uint32_t i, sf::Vector3f& color, const Impulse& impulse) const
{
	const sf::Vector2f impulsePos = impulse.GetPosition();
	const float impulseLength = impulse.GetLength();

	const float length = vu::distance(m_positions[i], impulsePos);
	const float diff = std::abs(length - impulseLength);

	const float percentage = (impulseLength / Config::Inst().Impulse.FadeDistance);
	const float size = impulse.GetSize() * (1.0f - percentage);

	if (diff <= size)
	{
		const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::Inst().Impulse.Colors.size() - 1);

		const auto i1 = (int)scaled_length;
		const auto i2 = (i1 == Config::Inst().Impulse.Colors.size() - 1) ? 0 : i1 + 1;

		const sf::Vector3f color1 = Config::Inst().Impulse.Colors[i1];
		const sf::Vector3f color2 = Config::Inst().Impulse.Colors[i2];

		const float newT = scaled_length - std::floorf(scaled_length);

		color = vu::lerp(color1, color2, newT);
	}
}
