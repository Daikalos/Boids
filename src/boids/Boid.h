#pragma once

#include <SFML/Graphics.hpp>

#include <span>

#include "Grid.h"
#include "Config.h"
#include "AudioMeter.h"
#include "Impulse.h"
#include "Fluid.h"

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"
#include "../utilities/Utilities.h"

class Boid
{
public:
	Boid(const sf::Vector2f& pos);
	Boid(const sf::Vector2f& pos, const sf::Vector2f& velocity);

public: // Properties
	[[nodiscard]] const sf::Vector2f& GetPosition() const noexcept;
	[[nodiscard]] const sf::Vector2f& GetPrevPosition() const noexcept;
	[[nodiscard]] const sf::Vector2f& GetRelativePosition() const noexcept;
	[[nodiscard]] const sf::Vector2f& GetVelocity() const noexcept;
	[[nodiscard]] const sf::Vector2f& GetPrevVelocity() const noexcept;
	[[nodiscard]] std::uint16_t GetCellIndex() const noexcept;

	[[nodiscard]] sf::Vector2f GetOrigin() const noexcept;
	[[nodiscard]] sf::Vector2f GetPrevOrigin() const noexcept;

	void SetCycleTime(const float val) noexcept;

public:
	void SteerTowards(const sf::Vector2f& direction, const float length, const float weight);
	void SteerTowards(const sf::Vector2f& point, const float weight);

public:
	void PreUpdate(const Grid& grid) noexcept;
	void UpdateGridCells(Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy, const std::uint32_t index) const;
	void Flock(const Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy);
	void Update(const RectFloat& border, std::span<const Impulse> impulses, float dt);

	sf::Vector3f GetColor(const RectFloat& border, const IAudioMeterInfo* audioMeter, std::span<const Impulse> impulses) const noexcept;

private:
	sf::Vector2f SteerAt(const sf::Vector2f& steerDir) const;

	bool OutsideBorder(const RectFloat& border, float dt);
	bool TurnAtBorder(const RectFloat& border, float dt);
	bool TeleportAtBorder(const RectFloat& border);

	sf::Vector3f PositionColor(const RectFloat& border) const;
	sf::Vector3f CycleColor() const;
	sf::Vector3f DensityColor() const;
	sf::Vector3f VelocityColor() const;
	sf::Vector3f RotationColor() const;
	sf::Vector3f AudioColor(const IAudioMeterInfo* audioMeter) const;
	void ImpulseColor(sf::Vector3f& color, std::span<const Impulse> impulses) const;

private:
	sf::Vector2f	m_position, m_prevPosition, m_relativePos;
	sf::Vector2f	m_velocity, m_prevVelocity;

	float			m_speed			{0.0f};
	float			m_cycleTime		{0.0f};
	float			m_densityTime	{0.0f};

	std::uint16_t	m_density		{0};
	std::uint16_t	m_cellIndex		{0};
};

