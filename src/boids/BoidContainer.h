#pragma once

#include <SFML/Graphics.hpp>

#include <span>
#include <ranges>
#include <memory>

#include "Grid.h"
#include "Config.h"
#include "AudioMeter.h"
#include "Impulse.h"
#include "Fluid.h"

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"
#include "../utilities/Utilities.h"
#include "../utilities/PolicySelect.h"

#include "../window/InputHandler.h"

class BoidContainer
{
public:
	BoidContainer(std::size_t capacity);

public:
	std::size_t GetSize() const noexcept;
	std::size_t GetCapacity() const noexcept;

public:
	void Push(const sf::Vector2f& pos);
	void Push(const sf::Vector2f& pos, const sf::Vector2f& velocity);

	void Pop(std::size_t count = 1);

public:
	void Reallocate(std::size_t capacity);
	void Reserve(std::size_t capacity);

public:
	void PreUpdate(const Grid& grid);

	void Sort();

	void UpdateCells(Grid& grid);

	void Interaction(const InputHandler& inputHandler, const sf::Vector2f& mousePos);

	void Flock(const Grid& grid, Policy policy);

	void Update(const RectFloat& border, float dt);

	void UpdateVertices(
		sf::VertexArray& vertices,
		const RectFloat& border, 
		const Fluid& fluid,
		const IAudioMeterInfo* audioMeter,
		std::span<const Impulse> impulses,
		Policy policy, float interp);

public:
	bool OutsideBorder(		std::uint32_t i, const RectFloat& border, float dt);
	bool TurnAtBorder(		std::uint32_t i, const RectFloat& border, float dt);
	bool TeleportAtBorder(	std::uint32_t i, const RectFloat& border);

public:
	sf::Vector2f SteerAt(std::uint32_t i, const sf::Vector2f& steerDir) const;

	void SteerTowards(std::uint32_t i, const sf::Vector2f& direction, float length, float weight);
	void SteerTowards(std::uint32_t i, const sf::Vector2f& point, float weight);

	void ResetCycleTimes();

private:
	sf::Vector2f GetOrigin(const sf::Vector2f& pos) const;

	sf::Vector3f GetColor(std::uint32_t i, const RectFloat& border, const IAudioMeterInfo* audioMeter, std::span<const Impulse> impulses);

	sf::Vector3f PositionColor(	std::uint32_t i, const RectFloat& border) const;
	sf::Vector3f CycleColor(	std::uint32_t i) const;
	sf::Vector3f DensityColor(	std::uint32_t i) const;
	sf::Vector3f VelocityColor(	std::uint32_t i) const;
	sf::Vector3f RotationColor(	std::uint32_t i) const;
	sf::Vector3f AudioColor(	std::uint32_t i, const IAudioMeterInfo* audioMeter) const;
	void ImpulseColor(			std::uint32_t i, sf::Vector3f& color, std::span<const Impulse> impulses) const;

private:
	std::unique_ptr<std::uint32_t[]>	m_indices;

	std::unique_ptr<sf::Vector2f[]>		m_positions;
	std::unique_ptr<sf::Vector2f[]>		m_prevPositions;
	std::unique_ptr<sf::Vector2f[]>		m_velocities;
	std::unique_ptr<sf::Vector2f[]>		m_prevVelocities;
	std::unique_ptr<sf::Vector2f[]>		m_relativePositions;

	std::unique_ptr<float[]>			m_speeds;
	std::unique_ptr<float[]>			m_angles;
	std::unique_ptr<float[]>			m_cycleTimes;
	std::unique_ptr<float[]>			m_densityTimes;

	std::unique_ptr<std::uint16_t[]>	m_densities;
	std::unique_ptr<std::uint16_t[]>	m_cellIndices;

	std::size_t	m_size		{0};
	std::size_t	m_capacity	{0};
};