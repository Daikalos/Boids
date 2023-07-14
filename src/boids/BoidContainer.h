#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <memory>

#include "Grid.h"
#include "AudioMeter.h"
#include "Impulse.h"
#include "Fluid.h"

#include "../utilities/PolicySelect.h"
#include "../utilities/Rectangle.hpp"
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

	void Interaction(const InputHandler& inputHandler, const sf::Vector2f& mousePos, float dt);

	void Flock(const Grid& grid, Policy policy);

	void Update(const RectFloat& border, const std::vector<Impulse>& impulses, float dt);

	void UpdateColors(
		const RectFloat& border,
		const Fluid& fluid,
		const IAudioMeterInfo* audioMeter,
		const std::vector<Impulse>& impulses);

	void UpdateVertices(
		sf::VertexArray& vertices,
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

	sf::Vector3f PositionColor(	std::uint32_t i, const RectFloat& border) const;
	sf::Vector3f CycleColor(	std::uint32_t i) const;
	sf::Vector3f DensityColor(	std::uint32_t i) const;
	sf::Vector3f VelocityColor(	std::uint32_t i) const;
	sf::Vector3f RotationColor(	std::uint32_t i) const;
	sf::Vector3f AudioColor(	std::uint32_t i, float volume) const;
	void ImpulseColor(			std::uint32_t i, sf::Vector3f& color, const Impulse& impulse) const;

private:
	std::unique_ptr<std::uint32_t[]>	m_indices;

	std::unique_ptr<sf::Vector2f[]>		m_positions;
	std::unique_ptr<sf::Vector2f[]>		m_prevPositions;
	std::unique_ptr<sf::Vector2f[]>		m_velocities;
	std::unique_ptr<sf::Vector2f[]>		m_prevVelocities;
	std::unique_ptr<sf::Vector2f[]>		m_relativePositions;
	std::unique_ptr<sf::Vector3f[]>		m_colors;

	std::unique_ptr<float[]>			m_speeds;
	std::unique_ptr<float[]>			m_angles;
	std::unique_ptr<float[]>			m_prevAngles;
	std::unique_ptr<float[]>			m_cycleTimes;
	std::unique_ptr<float[]>			m_densityTimes;

	std::unique_ptr<std::uint16_t[]>	m_densities;
	std::unique_ptr<std::uint16_t[]>	m_cellIndices;

	std::size_t	m_size		{0};
	std::size_t	m_capacity	{0};
};
