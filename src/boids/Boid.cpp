#include "Boid.h"

Boid::Boid(const sf::Vector2f& pos) : m_position(pos), m_prevPosition(pos)
{
	m_prevVelocity = m_velocity = vu::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), Config::Inst().BoidSpeedMax);

	if (Config::Inst().BoidCycleColorsRandom)
		m_cycleTime = util::random(0.0f, 1.0f);
}

Boid::Boid(const sf::Vector2f& pos, const sf::Vector2f& velocity)
	: m_position(pos), m_prevPosition(pos), m_velocity(velocity), m_prevVelocity(velocity)
{
	if (Config::Inst().BoidCycleColorsRandom)
		m_cycleTime = util::random(0.0f, 1.0f);
}

const sf::Vector2f& Boid::GetPosition() const noexcept			{ return m_position; }
const sf::Vector2f& Boid::GetPrevPosition() const noexcept		{ return m_prevPosition; }
const sf::Vector2f& Boid::GetRelativePosition() const noexcept	{ return m_relativePos; }
const sf::Vector2f& Boid::GetVelocity() const noexcept			{ return m_velocity; }
const sf::Vector2f& Boid::GetPrevVelocity() const noexcept		{ return m_prevVelocity; }
std::uint16_t Boid::GetCellIndex() const noexcept				{ return m_cellIndex; }

sf::Vector2f Boid::GetOrigin() const noexcept
{
	return GetPosition() + sf::Vector2f(
		Config::Inst().BoidWidth,
		Config::Inst().BoidHeight) / 2.0f;
}
sf::Vector2f Boid::GetPrevOrigin() const noexcept
{
	return GetPrevPosition() + sf::Vector2f(
		Config::Inst().BoidWidth,
		Config::Inst().BoidHeight) / 2.0f;
}

void Boid::SetCycleTime(const float val) noexcept
{
	m_cycleTime = val;
}

void Boid::PreUpdate(const Grid& grid) noexcept
{
	m_prevPosition = m_position;
	m_prevVelocity = m_velocity;

	const sf::Vector2f gridCellRaw		= grid.RelativePos(GetOrigin());
	const sf::Vector2i gridCell			= sf::Vector2i(gridCellRaw);
	const sf::Vector2f gridCellOverflow = gridCellRaw - sf::Vector2f(gridCell);

	m_relativePos	= gridCellOverflow * grid.contDims;
	m_cellIndex		= grid.AtPos(gridCell);
}

void Boid::UpdateGridCells(Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy, const uint32_t index) const
{
	if (index == 0)
	{
		grid.startIndices[m_cellIndex] = index;
		return;
	}

	if (index == boids.size() - 1)
		grid.endIndices[m_cellIndex] = index;

	const int otherIndex = boids[proxy[index - 1]].GetCellIndex();

	if (otherIndex != m_cellIndex)
	{
		grid.startIndices[m_cellIndex] = index;
		grid.endIndices[otherIndex] = index - 1;
	}
}

void Boid::Flock(const Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy)
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	std::uint16_t sepCount = 0;
	std::uint16_t aliCount = 0;
	std::uint16_t cohCount = 0;

	const sf::Vector2f origin = GetOrigin();

	constexpr auto neighbourCount = 4; // max 4 neighbours at a time

	int neighIndicies[neighbourCount] {};
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

	neighIndicies[0] = grid.AtPos(gridCell.x,  gridCell.y);		// current
	neighIndicies[1] = grid.AtPos(neighbour_x, gridCell.y);		// left or right of current
	neighIndicies[2] = grid.AtPos(gridCell.x,  neighbour_y);	// top or bot of current
	neighIndicies[3] = grid.AtPos(neighbour_x, neighbour_y);	// top left/right bot left/right of current

	Config& config = Config::Inst();

	for (std::uint8_t i = 0; i < neighbourCount; ++i)
	{
		const int gridCellIndex = neighIndicies[i];
		const int start = grid.startIndices[gridCellIndex];

		if (start == -1)
			continue;

		const int end = grid.endIndices[gridCellIndex];
		const sf::Vector2f neighbourCell = neighbours[i];

		for (int j = start; j <= end; ++j) // do in one loop
		{
			const Boid& b = boids[proxy[j]];

			if (&b == this)
				continue;

			const sf::Vector2f otherRelativePos = 
				neighbourCell + b.GetRelativePosition(); // need to get relative to this Boid

			const sf::Vector2f dir	= vu::direction(m_relativePos, otherRelativePos);
			const float distanceSqr = std::max(vu::distance_sq(dir), FLT_EPSILON);
			const float angle		= m_prevVelocity.angleTo(dir).asRadians();

			if (angle >= -config.BoidViewAngle && angle <= config.BoidViewAngle) // angle only effects cohesion and alignment
			{
				if (distanceSqr <= config.CohDistance)
				{
					coh += origin + dir; // Head towards center of boids
					++cohCount;
				}
				if (distanceSqr <= config.AliDistance)
				{
					ali += b.GetPrevVelocity(); // Align with every boids velocity
					++aliCount;
				}
			}

			if (distanceSqr <= config.SepDistance)
			{
				sep += -dir / distanceSqr;
				++sepCount;
			}
		}
	}

	if (cohCount) coh = vu::normalize(vu::direction(origin, coh / (float)cohCount), config.BoidSpeedMax);
	if (aliCount) ali = vu::normalize(ali / (float)aliCount, config.BoidSpeedMax);
	if (sepCount) sep = vu::normalize(sep / (float)sepCount, config.BoidSpeedMax);

	m_velocity +=
		SteerAt(coh) * config.CohWeight +
		SteerAt(ali) * config.AliWeight +
		SteerAt(sep) * config.SepWeight;

	m_density = std::max(std::max(cohCount, aliCount), sepCount);
}

void Boid::Update(const RectFloat& border, std::span<const Impulse> impulses, float dt)
{
	m_velocity = vu::clamp(m_velocity, Config::Inst().BoidSpeedMin, Config::Inst().BoidSpeedMax);

	m_position += m_velocity * dt;

	if (OutsideBorder(border, dt))
		m_prevPosition = m_position;

	if ((Config::Inst().ColorFlag & CF_Cycle) == CF_Cycle)
		m_cycleTime = std::fmodf(m_cycleTime + dt * Config::Inst().BoidCycleColorsSpeed, 1.0f);
	if ((Config::Inst().ColorFlag & CF_Density) == CF_Density)
		m_densityTime = (Config::Inst().BoidDensityCycleEnabled) ? std::fmodf(m_densityTime + dt * Config::Inst().BoidDensityCycleSpeed, 1.0f) : 0.0f;
}

sf::Vector2f Boid::SteerAt(const sf::Vector2f& steer_direction) const
{
	return vu::limit(vu::direction(m_prevVelocity, steer_direction), Config::Inst().BoidSteerMax);
}

void Boid::SteerTowards(const sf::Vector2f& direction, float length, float weight)
{
	if (std::abs(weight) < FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::normalize(direction, length, Config::Inst().BoidSpeedMax);

	m_velocity += SteerAt(steer) * weight;
}
void Boid::SteerTowards(const sf::Vector2f& point, float weight)
{
	SteerTowards(point, vu::distance(point), weight);
}

sf::Vector3f Boid::GetColor(const RectFloat& border, const IAudioMeterInfo* audioMeter, std::span<const Impulse> impulses) const noexcept
{
	sf::Vector3f color = sf::Vector3f();

	std::uint32_t flag = Config::Inst().ColorFlag;

	if ((flag & CF_Positional) == CF_Positional)
		color += PositionColor(border) * Config::Inst().ColorPositionalWeight;
	if ((flag & CF_Cycle) == CF_Cycle)
		color += CycleColor() * Config::Inst().ColorCycleWeight;
	if ((flag & CF_Density) == CF_Density)
		color += DensityColor() * Config::Inst().ColorDensityWeight;
	if ((flag & CF_Velocity) == CF_Velocity)
		color += VelocityColor() * Config::Inst().ColorVelocityWeight;
	if ((flag & CF_Rotation) == CF_Rotation)
		color += RotationColor() * Config::Inst().ColorRotationWeight;
	if ((flag & CF_Audio) == CF_Audio && audioMeter != nullptr)
		color += AudioColor(audioMeter) * Config::Inst().ColorAudioWeight;

	ImpulseColor(color, impulses);

	return color;
}

bool Boid::OutsideBorder(const RectFloat& border, float dt)
{
	return Config::Inst().TurnAtBorder ? TurnAtBorder(border, dt) : TeleportAtBorder(border);
}
bool Boid::TurnAtBorder(const RectFloat& border, float dt)
{
	const float widthMargin = border.width() - border.width() * Config::Inst().TurnMarginFactor;
	const float heightMargin = border.height() - border.height() * Config::Inst().TurnMarginFactor;

	const float leftMargin = border.left + widthMargin;
	const float topMargin = border.top + heightMargin;
	const float rightMargin = border.right - widthMargin;
	const float botMargin = border.bot - heightMargin;

	if (m_position.x + Config::Inst().BoidWidth < leftMargin)
		m_velocity.x += Config::Inst().TurnFactor * dt * (1.0f + std::powf(std::abs(m_position.x - leftMargin) / widthMargin, 2.0f)) * (1.0f / (m_density + 1.0f));

	if (m_position.x > rightMargin)
		m_velocity.x -= Config::Inst().TurnFactor * dt * (1.0f + std::powf(std::abs(m_position.x - rightMargin) / widthMargin, 2.0f)) * (1.0f / (m_density + 1.0f));

	if (m_position.y + Config::Inst().BoidHeight < topMargin)
		m_velocity.y += Config::Inst().TurnFactor * dt * (1.0f + std::powf(std::abs(m_position.y - topMargin) / heightMargin, 2.0f)) * (1.0f / (m_density + 1.0f));

	if (m_position.y > botMargin)
		m_velocity.y -= Config::Inst().TurnFactor * dt * (1.0f + std::powf(std::abs(m_position.y - botMargin) / heightMargin, 2.0f)) * (1.0f / (m_density + 1.0f));

	return false;
}
bool Boid::TeleportAtBorder(const RectFloat& border)
{
	const float maxSize = std::max(Config::Inst().BoidWidth, Config::Inst().BoidHeight);
	const sf::Vector2f current = m_position;

	if (m_position.x + maxSize < border.left)
		m_position.x = (float)border.right;

	if (m_position.x > border.right)
		m_position.x = border.left - maxSize;

	if (m_position.y + maxSize < border.top)
		m_position.y = (float)border.bot;

	if (m_position.y > border.bot)
		m_position.y = border.top - maxSize;

	return (current != m_position);
}

sf::Vector3f Boid::PositionColor(const RectFloat& border) const
{
	const float t = m_position.x / border.width();
	const float s = m_position.y / border.height();

	return sf::Vector3f(
		util::interpolate(Config::Inst().BoidColorTopLeft.x * 255.0f, Config::Inst().BoidColorTopRight.x * 255.0f, Config::Inst().BoidColorBotLeft.x * 255.0f, Config::Inst().BoidColorBotRight.x * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().BoidColorTopLeft.y * 255.0f, Config::Inst().BoidColorTopRight.y * 255.0f, Config::Inst().BoidColorBotLeft.y * 255.0f, Config::Inst().BoidColorBotRight.y * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::Inst().BoidColorTopLeft.z * 255.0f, Config::Inst().BoidColorTopRight.z * 255.0f, Config::Inst().BoidColorBotLeft.z * 255.0f, Config::Inst().BoidColorBotRight.z * 255.0f, t, s) / 255.0f);
}
sf::Vector3f Boid::CycleColor() const
{
	if (Config::Inst().BoidCycleColors.empty()) [[unlikely]]
		return sf::Vector3f();

	float scaledTime = m_cycleTime * (float)(Config::Inst().BoidCycleColors.size() - 1);

	const auto index1 = (int)scaledTime;
	const auto index2 = ((int)scaledTime + 1) % (int)Config::Inst().BoidCycleColors.size();

	const sf::Vector3f color1 = Config::Inst().BoidCycleColors[index1];
	const sf::Vector3f color2 = Config::Inst().BoidCycleColors[index2];

	const float newT = scaledTime - std::floorf(scaledTime);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::DensityColor() const
{
	if (Config::Inst().BoidDensityColors.empty() || !Config::Inst().BoidDensity) [[unlikely]]
		return sf::Vector3f();

	const float densityPercentage = (m_density / (float)Config::Inst().BoidDensity);

	const float scaledDensity = std::fmodf(densityPercentage + m_densityTime, 1.0f) * (float)(Config::Inst().BoidDensityColors.size() - 1);

	const auto index1 = (int)scaledDensity;
	const auto index2 = ((int)scaledDensity + 1) % (int)Config::Inst().BoidDensityColors.size();

	const sf::Vector3f color1 = Config::Inst().BoidDensityColors[index1];
	const sf::Vector3f color2 = Config::Inst().BoidDensityColors[index2];

	const float newT = scaledDensity - std::floorf(scaledDensity);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::VelocityColor() const
{
	if (Config::Inst().BoidVelocityColors.empty() || (Config::Inst().BoidSpeedMax == Config::Inst().BoidSpeedMin)) [[unlikely]]
		return sf::Vector3f();

	const float velocity_percentage = std::clamp((vu::distance(m_velocity) - Config::Inst().BoidSpeedMin) / (Config::Inst().BoidSpeedMax - Config::Inst().BoidSpeedMin), 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(Config::Inst().BoidVelocityColors.size() - 1);

	const auto index1 = (int)scaled_velocity;
	const auto index2 = ((int)scaled_velocity + 1) % (int)Config::Inst().BoidVelocityColors.size();

	const sf::Vector3f color1 = Config::Inst().BoidVelocityColors[index1];
	const sf::Vector3f color2 = Config::Inst().BoidVelocityColors[index2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::RotationColor() const
{
	if (Config::Inst().BoidRotationColors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float rotationPercentage = (m_velocity.angle().asRadians() + float(M_PI)) / (2.0f * float(M_PI));

	const float scaledRotation = rotationPercentage * (float)(Config::Inst().BoidRotationColors.size() - 1);

	const auto index1 = (int)scaledRotation;
	const auto index2 = ((int)scaledRotation + 1) % (int)Config::Inst().BoidRotationColors.size();

	const sf::Vector3f color1 = Config::Inst().BoidRotationColors[index1];
	const sf::Vector3f color2 = Config::Inst().BoidRotationColors[index2];

	const float newT = scaledRotation - std::floorf(scaledRotation);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::AudioColor(const IAudioMeterInfo* audioMeter) const
{
	if (Config::Inst().AudioResponsiveColors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float densityPercentage = (m_density / (float)Config::Inst().AudioResponsiveDensity);
	const float maxVolume = std::fminf(audioMeter->GetVolume() * Config::Inst().AudioResponsiveStrength, Config::Inst().AudioResponsiveLimit);

	const float scaledVolume = std::fminf(maxVolume * densityPercentage, 1.0f) * (float)(Config::Inst().AudioResponsiveColors.size() - 1);

	const auto index1 = (int)scaledVolume;
	const auto index2 = ((int)scaledVolume + 1) % (int)Config::Inst().AudioResponsiveColors.size();

	const sf::Vector3f color1 = Config::Inst().AudioResponsiveColors[index1];
	const sf::Vector3f color2 = Config::Inst().AudioResponsiveColors[index2];

	const float newT = scaledVolume - std::floorf(scaledVolume);

	return vu::lerp(color1, color2, newT);
}
void Boid::ImpulseColor(sf::Vector3f& Color, std::span<const Impulse> impulses) const
{
	if (Config::Inst().ImpulseColors.empty()) [[unlikely]]
		return;

	for (const Impulse& impulse : impulses)
	{
		const sf::Vector2f impulsePos = impulse.GetPosition();
		const float impulseLength = impulse.GetLength();

		const float length = vu::distance(m_position, impulsePos);
		const float diff = std::abs(length - impulseLength);

		const float percentage = (impulseLength / Config::Inst().ImpulseFadeDistance);
		const float size = impulse.GetSize() * (1.0f - percentage);

		if (diff <= size)
		{
			const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::Inst().ImpulseColors.size() - 1);

			const auto index1 = (int)scaled_length;
			const auto index2 = ((int)scaled_length + 1) % (int)Config::Inst().ImpulseColors.size();

			const sf::Vector3f color1 = Config::Inst().ImpulseColors[index1];
			const sf::Vector3f color2 = Config::Inst().ImpulseColors[index2];

			const float newT = scaled_length - std::floorf(scaled_length);

			Color = vu::lerp(color1, color2, newT);
		}
	}
}
