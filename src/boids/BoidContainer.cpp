#include "BoidContainer.h"

BoidContainer::BoidContainer(std::size_t capacity) : m_capacity(capacity)
{
	m_indices			= std::make_unique<std::uint32_t[]>(m_capacity);

	m_positions			= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_prevPositions		= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_velocities		= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_prevVelocities	= std::make_unique<sf::Vector2f[]>(m_capacity);
	m_relativePositions = std::make_unique<sf::Vector2f[]>(m_capacity);

	m_speeds			= std::make_unique<float[]>(m_capacity);
	m_angles			= std::make_unique<float[]>(m_capacity);
	m_cycleTimes		= std::make_unique<float[]>(m_capacity);
	m_densityTimes		= std::make_unique<float[]>(m_capacity);

	m_densities			= std::make_unique<std::uint16_t[]>(m_capacity);
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
	if (m_size == m_capacity)
		Reallocate(1.5 * m_capacity + 1);

	m_indices[m_size] = m_size;

	m_positions[m_size] = pos;
	m_prevVelocities[m_size] = m_velocities[m_size] = vu::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), Config::Inst().BoidSpeedMax);

	if (Config::Inst().BoidCycleColorsRandom)
		m_cycleTimes[m_size] = util::random(0.0f, 1.0f);

	++m_size;
}

void BoidContainer::Push(const sf::Vector2f& pos, const sf::Vector2f& velocity)
{
	if (m_size == m_capacity)
		Reallocate(1.5 * m_capacity + 1);

	m_indices[m_size] = m_size;

	m_positions[m_size] = pos;
	m_prevVelocities[m_size] = m_velocities[m_size] = velocity;

	if (Config::Inst().BoidCycleColorsRandom)
		m_cycleTimes[m_size] = util::random(0.0f, 1.0f);

	++m_size;
}

void BoidContainer::Pop(std::size_t count)
{
	assert(count > 0); // pop nothing ???

	std::size_t oldSize = m_size;
	m_size = (count < m_size) ? (m_size - count) : 0;

	std::remove_if(m_indices.get(), m_indices.get() + oldSize,
		[this](std::uint32_t i)
		{
			return i >= m_size; // push all older indices to the end
		});
}

void BoidContainer::Reallocate(std::size_t capacity)
{
	const auto realloc = 
		[this, &capacity]<typename T>(std::unique_ptr<T[]>& ptr)
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

	realloc(m_speeds);
	realloc(m_angles);
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

		const sf::Vector2f gridCellRaw		= grid.RelativePos(GetOrigin(m_positions[i]));
		const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
		const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

		m_relativePositions[i]	= gridCellOverflow * grid.contDims;
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

	grid.startIndices[m_cellIndices[m_indices[0]]] = 0;

	for (std::size_t i = 1; i < (m_size - 1); ++i)
	{
		const std::uint16_t cellIndex	= m_cellIndices[m_indices[i]];
		const std::uint16_t otherIndex	= m_cellIndices[m_indices[i - 1]];

		if (otherIndex != cellIndex)
		{
			grid.startIndices[cellIndex] = i;
			grid.endIndices[otherIndex] = i - 1;
		}
	}

	grid.endIndices[m_cellIndices[m_indices[m_size - 1]]] = m_size - 1;
}

void BoidContainer::Interaction(const InputHandler& inputHandler, const sf::Vector2f& mousePos)
{
	const bool holdLeft		= inputHandler.GetButtonHeld(sf::Mouse::Button::Left);
	const bool holdRight	= inputHandler.GetButtonHeld(sf::Mouse::Button::Right);

	for (std::size_t i = 0; i < m_size; ++i)
	{
		if (Config::Inst().SteerEnabled && (holdLeft || holdRight))
		{
			sf::Vector2f dir = vu::direction(m_positions[i], mousePos);

			const float factor = holdLeft ? 1.0f :
				(holdRight ? -1.0f : 0.0f);

			const float lengthOpt = vu::distance_opt(dir);
			const float weight = 15.0f / (std::sqrtf(lengthOpt) + FLT_EPSILON);

			SteerTowards(i, dir, lengthOpt, Config::Inst().SteerTowardsFactor * weight * factor);
		}
		else if (Config::Inst().PredatorEnabled)
		{
			sf::Vector2f dir = vu::direction(m_positions[i], mousePos);

			float lengthSqr = dir.lengthSq();
			if (lengthSqr <= Config::Inst().PredatorDistance)
			{
				float weight = std::sqrtf(lengthSqr / Config::Inst().PredatorDistance);
				SteerTowards(i, dir, -Config::Inst().PredatorFactor / (weight + FLT_EPSILON));
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

					std::uint16_t sepCount = 0;
					std::uint16_t aliCount = 0;
					std::uint16_t cohCount = 0;

					const sf::Vector2f origin = GetOrigin(m_positions[lhs]);

					constexpr auto neighbourCount = 4; // max 4 neighbours at a time

					int neighIndices[neighbourCount]{};
					sf::Vector2f neighbours[neighbourCount];

					const sf::Vector2f gridCellRaw		= grid.RelativePos(origin);
					const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
					const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

					const int x = (gridCellOverflow.x > 0.5f ? 1 : -1);
					const int y = (gridCellOverflow.y > 0.5f ? 1 : -1);

					const int neighbour_x = gridCell.x + x;
					const int neighbour_y = gridCell.y + y;

					neighbours[0] = grid.contDims * sf::Vector2f(0,			0);
					neighbours[1] = grid.contDims * sf::Vector2f((float)x,	0);
					neighbours[2] = grid.contDims * sf::Vector2f(0,			(float)y);
					neighbours[3] = grid.contDims * sf::Vector2f((float)x,	(float)y);

					neighIndices[0] = grid.AtPos(gridCell.x,	gridCell.y);	// current
					neighIndices[1] = grid.AtPos(neighbour_x,	gridCell.y);	// left or right of current
					neighIndices[2] = grid.AtPos(gridCell.x,	neighbour_y);	// top or bot of current
					neighIndices[3] = grid.AtPos(neighbour_x,	neighbour_y);	// top left/right bot left/right of current

					Config& config = Config::Inst();

					for (std::uint8_t i = 0; i < neighbourCount; ++i)
					{
						const int gridCellIndex = neighIndices[i];
						const int start = grid.startIndices[gridCellIndex];

						if (start == -1)
							continue;

						const int end = grid.endIndices[gridCellIndex];
						const sf::Vector2f neighbourCell = neighbours[i];

						for (int j = start; j <= end; ++j) // do in one loop
						{
							const std::uint32_t rhs = m_indices[j];

							if (lhs == rhs)
								continue;

							const sf::Vector2f otherRelativePos =
								neighbourCell + m_relativePositions[rhs]; // need to get relative to this Boid

							const sf::Vector2f dir = otherRelativePos - m_relativePositions[lhs];
							const float distanceSqr = dir.lengthSq();

							if (distanceSqr == 0.0f)
								continue;

							const bool withinCohesion	= distanceSqr <= config.CohDistance;
							const bool withinAlignment	= distanceSqr <= config.AliDistance;

							if (withinCohesion || withinAlignment)
							{
								const float angle = m_prevVelocities[lhs].angleTo(dir).asRadians();
								if (angle >= -config.BoidViewAngle && angle <= config.BoidViewAngle)
								{
									if (withinCohesion)
									{
										coh += origin + dir; // Head towards center of boids
										++cohCount;
									}
									if (withinAlignment)
									{
										ali += m_prevVelocities[rhs]; // Align with every boids velocity
										++aliCount;
									}
								}
							}

							if (distanceSqr <= config.SepDistance)
							{
								sep += -dir / distanceSqr;
								++sepCount;
							}
						}
					}

					if (cohCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(vu::direction(origin, coh / static_cast<float>(cohCount)), config.BoidSpeedMax)) * config.CohWeight;
					if (aliCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(ali / static_cast<float>(aliCount), config.BoidSpeedMax)) * config.AliWeight;
					if (sepCount) m_velocities[lhs] += SteerAt(lhs, vu::normalize(sep / static_cast<float>(sepCount), config.BoidSpeedMax)) * config.SepWeight;

					m_densities[lhs] = std::max(std::max(cohCount, aliCount), sepCount);
				});
		}, policy);
}

void BoidContainer::Update(const RectFloat& border, float dt)
{
	for (std::size_t i = 0; i < m_size; ++i)
	{
		m_speeds[i] = std::clamp(m_velocities[i].length(), Config::Inst().BoidSpeedMin, Config::Inst().BoidSpeedMax);
		m_velocities[i] = vu::normalize(m_velocities[i], m_speeds[i]);

		m_positions[i] += m_velocities[i] * dt;

		if (OutsideBorder(i, border, dt))
			m_prevPositions[i] = m_positions[i];

		if ((Config::Inst().ColorFlag & CF_Cycle) == CF_Cycle)
			m_cycleTimes[i] = std::fmodf(m_cycleTimes[i] + dt * Config::Inst().BoidCycleColorsSpeed, 1.0f);
		if ((Config::Inst().ColorFlag & CF_Density) == CF_Density)
			m_densityTimes[i] = (Config::Inst().BoidDensityCycleEnabled) ? std::fmodf(m_densityTimes[i] + dt * Config::Inst().BoidDensityCycleSpeed, 1.0f) : 0.0f;
	}
}

void BoidContainer::UpdateVertices(sf::VertexArray& vertices, const RectFloat& border, const Fluid& fluid, const IAudioMeterInfo* audioMeter, std::span<const Impulse> impulses, Policy policy, float interp)
{
	PolicySelect([&](auto& pol)
		{
			std::for_each(pol, m_indices.get(), m_indices.get() + m_size,
				[&](std::uint32_t i)
				{
					Config& config = Config::Inst();

					const sf::Vector2f ori		= GetOrigin(m_positions[i]);
					const sf::Vector2f prevOri	= GetOrigin(m_prevPositions[i]);

					const float rot			= m_velocities[i].angle().asRadians();
					const float prev_rot	= m_prevVelocities[i].angle().asRadians();

					m_angles[i] = rot;

					const sf::Vector2f pointA = vu::rotate_point({ ori.x + (config.BoidWidth / 2), ori.y }, ori, rot); // middle right tip
					const sf::Vector2f pointB = vu::rotate_point({ ori.x - (config.BoidWidth / 2), ori.y - (config.BoidHeight / 2) }, ori, rot); // top left corner
					const sf::Vector2f pointC = vu::rotate_point({ ori.x - (config.BoidWidth / 2), ori.y + (config.BoidHeight / 2) }, ori, rot); // bot left corner

					const sf::Vector2f prevPointA = vu::rotate_point({ prevOri.x + (config.BoidWidth / 2), prevOri.y }, prevOri, prev_rot); // middle right tip
					const sf::Vector2f prevPointB = vu::rotate_point({ prevOri.x - (config.BoidWidth / 2), prevOri.y - (config.BoidHeight / 2) }, prevOri, prev_rot); // top left corner
					const sf::Vector2f prevPointC = vu::rotate_point({ prevOri.x - (config.BoidWidth / 2), prevOri.y + (config.BoidHeight / 2) }, prevOri, prev_rot); // bot left corner

					const sf::Vector2f p0 = pointA * interp + prevPointA * (1.0f - interp);
					const sf::Vector2f p1 = pointB * interp + prevPointB * (1.0f - interp);
					const sf::Vector2f p2 = pointC * interp + prevPointC * (1.0f - interp);

					sf::Vector3f bc = GetColor(i, border, audioMeter, impulses);

					if ((config.ColorFlag & CF_Fluid) == CF_Fluid)
					{
						bc += fluid.GetColor((p0 + p1 + p2) / 3.0f);
					}

					bc.x = std::clamp(bc.x, 0.0f, 1.0f);
					bc.y = std::clamp(bc.y, 0.0f, 1.0f);
					bc.z = std::clamp(bc.z, 0.0f, 1.0f);

					const sf::Color c = sf::Color(
						(sf::Uint8)(bc.x * 255.0f), 
						(sf::Uint8)(bc.y * 255.0f), 
						(sf::Uint8)(bc.z * 255.0f));

					const std::size_t v = std::size_t(i) * 3;

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
	return Config::Inst().TurnAtBorder ? TurnAtBorder(i, border, dt) : TeleportAtBorder(i, border);
}

bool BoidContainer::TurnAtBorder(std::uint32_t i, const RectFloat& border, float dt)
{
	const float maxSize = std::max(Config::Inst().BoidWidth, Config::Inst().BoidHeight);

	float widthMargin = border.width() - border.width() * Config::Inst().TurnMarginFactor;
	float heightMargin = border.height() - border.height() * Config::Inst().TurnMarginFactor;

	const float leftMargin = border.left + widthMargin;
	const float topMargin = border.top + heightMargin;
	const float rightMargin = border.right - widthMargin;
	const float botMargin = border.bot - heightMargin;

	if (widthMargin == 0.0f)	widthMargin = 1.0f;
	if (heightMargin == 0.0f)	heightMargin = 1.0f;

	if (m_positions[i].x + maxSize < leftMargin)
		m_velocities[i].x += Config::Inst().TurnFactor * std::powf(std::abs(m_positions[i].x - leftMargin) / widthMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].x > rightMargin)
		m_velocities[i].x -= Config::Inst().TurnFactor * std::powf(std::abs(m_positions[i].x - rightMargin) / widthMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].y + maxSize < topMargin)
		m_velocities[i].y += Config::Inst().TurnFactor * std::powf(std::abs(m_positions[i].y - topMargin) / heightMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	if (m_positions[i].y > botMargin)
		m_velocities[i].y -= Config::Inst().TurnFactor * std::powf(std::abs(m_positions[i].y - botMargin) / heightMargin, 2.0f) * (1.0f / (m_densities[i] + 1.0f)) * dt;

	return false;
}

bool BoidContainer::TeleportAtBorder(std::uint32_t i, const RectFloat& border)
{
	const float maxSize = std::max(Config::Inst().BoidWidth, Config::Inst().BoidHeight);
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
	return vu::limit(vu::direction(m_prevVelocities[i], steerDir), Config::Inst().BoidSteerMax);
}

void BoidContainer::SteerTowards(std::uint32_t i, const sf::Vector2f& direction, float length, float weight)
{
	if (std::abs(weight) < FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::normalize(direction, length, Config::Inst().BoidSpeedMax);

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
		m_cycleTimes[i] = Config::Inst().BoidCycleColorsRandom ?
			util::random(0.0f, 1.0f) : 0.0f;
	}
}

sf::Vector2f BoidContainer::GetOrigin(const sf::Vector2f& pos) const
{
	return pos + (sf::Vector2f(Config::Inst().BoidWidth, Config::Inst().BoidHeight) / 2.0f);
}

sf::Vector3f BoidContainer::GetColor(std::uint32_t i, const RectFloat& border, const IAudioMeterInfo* audioMeter, std::span<const Impulse> impulses)
{
	sf::Vector3f color = sf::Vector3f();

	std::uint32_t flag = Config::Inst().ColorFlag;

	if ((flag & CF_Positional) == CF_Positional)
		color += PositionColor(i, border) * Config::Inst().ColorPositionalWeight;
	if ((flag & CF_Cycle) == CF_Cycle)
		color += CycleColor(i) * Config::Inst().ColorCycleWeight;
	if ((flag & CF_Density) == CF_Density)
		color += DensityColor(i) * Config::Inst().ColorDensityWeight;
	if ((flag & CF_Velocity) == CF_Velocity)
		color += VelocityColor(i) * Config::Inst().ColorVelocityWeight;
	if ((flag & CF_Rotation) == CF_Rotation)
		color += RotationColor(i) * Config::Inst().ColorRotationWeight;
	if ((flag & CF_Audio) == CF_Audio && audioMeter != nullptr)
		color += AudioColor(i, audioMeter) * Config::Inst().ColorAudioWeight;

	ImpulseColor(i, color, impulses);

	return color;
}

sf::Vector3f BoidContainer::PositionColor(std::uint32_t i, const RectFloat& border) const
{
	const float t = m_positions[i].x / border.width();
	const float s = m_positions[i].y / border.height();

	return sf::Vector3f(
		util::interpolate(Config::Inst().BoidColorTopLeft.x * 255.0f, Config::Inst().BoidColorTopRight.x * 255.0f, Config::Inst().BoidColorBotLeft.x * 255.0f, Config::Inst().BoidColorBotRight.x * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().BoidColorTopLeft.y * 255.0f, Config::Inst().BoidColorTopRight.y * 255.0f, Config::Inst().BoidColorBotLeft.y * 255.0f, Config::Inst().BoidColorBotRight.y * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().BoidColorTopLeft.z * 255.0f, Config::Inst().BoidColorTopRight.z * 255.0f, Config::Inst().BoidColorBotLeft.z * 255.0f, Config::Inst().BoidColorBotRight.z * 255.0f, t, s) / 255.0f);
}

sf::Vector3f BoidContainer::CycleColor(std::uint32_t i) const
{
	if (Config::Inst().BoidCycleColors.empty()) [[unlikely]]
		return sf::Vector3f();

	float scaledTime = m_cycleTimes[i] * (float)(Config::Inst().BoidCycleColors.size() - 1);

	const auto i1 = (int)scaledTime;
	const auto i2 = (i1 == Config::Inst().BoidCycleColors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().BoidCycleColors[i1];
	const sf::Vector3f color2 = Config::Inst().BoidCycleColors[i2];

	const float newT = scaledTime - std::floorf(scaledTime);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::DensityColor(std::uint32_t i) const
{
	if (Config::Inst().BoidDensityColors.empty() || Config::Inst().BoidDensity <= 0) [[unlikely]]
		return sf::Vector3f();

	const float densityPercentage = (m_densities[i] / (float)Config::Inst().BoidDensity);

	const float scaledDensity = std::fmodf(densityPercentage + m_densityTimes[i], 1.0f) * (float)(Config::Inst().BoidDensityColors.size() - 1);

	const auto i1 = (int)scaledDensity;
	const auto i2 = (i1 == Config::Inst().BoidDensityColors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().BoidDensityColors[i1];
	const sf::Vector3f color2 = Config::Inst().BoidDensityColors[i2];

	const float newT = scaledDensity - std::floorf(scaledDensity);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::VelocityColor(std::uint32_t i) const
{
	if (Config::Inst().BoidVelocityColors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float inv = (Config::Inst().BoidSpeedMax == Config::Inst().BoidSpeedMin) ? 1.0f
		: (1.0f / (Config::Inst().BoidSpeedMax - Config::Inst().BoidSpeedMin));

	const float velocity_percentage = std::clamp((m_speeds[i] - Config::Inst().BoidSpeedMin) * inv, 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(Config::Inst().BoidVelocityColors.size() - 1);

	const auto i1 = (int)scaled_velocity;
	const auto i2 = (i1 == Config::Inst().BoidVelocityColors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().BoidVelocityColors[i1];
	const sf::Vector3f color2 = Config::Inst().BoidVelocityColors[i2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::RotationColor(std::uint32_t i) const
{
	if (Config::Inst().BoidRotationColors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float rotationPercentage = (m_angles[i] + float(M_PI)) / (2.0f * float(M_PI));

	const float scaledRotation = rotationPercentage * (float)(Config::Inst().BoidRotationColors.size() - 1);

	const auto i1 = (int)scaledRotation;
	const auto i2 = (i1 == Config::Inst().BoidRotationColors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().BoidRotationColors[i1];
	const sf::Vector3f color2 = Config::Inst().BoidRotationColors[i2];

	const float newT = scaledRotation - std::floorf(scaledRotation);

	return vu::lerp(color1, color2, newT);
}

sf::Vector3f BoidContainer::AudioColor(std::uint32_t i, const IAudioMeterInfo* audioMeter) const
{
	if (Config::Inst().AudioResponsiveColors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float densityPercentage = (m_densities[i] / (float)Config::Inst().AudioResponsiveDensity);
	const float maxVolume = std::fminf(audioMeter->GetVolume() * Config::Inst().AudioResponsiveStrength, Config::Inst().AudioResponsiveLimit);

	const float scaledVolume = std::fminf(maxVolume * densityPercentage, 1.0f) * (float)(Config::Inst().AudioResponsiveColors.size() - 1);

	const auto i1 = (int)scaledVolume;
	const auto i2 = (i1 == Config::Inst().AudioResponsiveColors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().AudioResponsiveColors[i1];
	const sf::Vector3f color2 = Config::Inst().AudioResponsiveColors[i2];

	const float newT = scaledVolume - std::floorf(scaledVolume);

	return vu::lerp(color1, color2, newT);
}

void BoidContainer::ImpulseColor(std::uint32_t i, sf::Vector3f& color, std::span<const Impulse> impulses) const
{
	if (Config::Inst().ImpulseColors.empty()) [[unlikely]]
		return;

	for (const Impulse& impulse : impulses)
	{
		const sf::Vector2f impulsePos = impulse.GetPosition();
		const float impulseLength = impulse.GetLength();

		const float length = vu::distance(m_positions[i], impulsePos);
		const float diff = std::abs(length - impulseLength);

		const float percentage = (impulseLength / Config::Inst().ImpulseFadeDistance);
		const float size = impulse.GetSize() * (1.0f - percentage);

		if (diff <= size)
		{
			const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::Inst().ImpulseColors.size() - 1);

			const auto i1 = (int)scaled_length;
			const auto i2 = (i1 == Config::Inst().ImpulseColors.size() - 1) ? 0 : i1 + 1;

			const sf::Vector3f color1 = Config::Inst().ImpulseColors[i1];
			const sf::Vector3f color2 = Config::Inst().ImpulseColors[i2];

			const float newT = scaled_length - std::floorf(scaled_length);

			color = vu::lerp(color1, color2, newT);
		}
	}
}
