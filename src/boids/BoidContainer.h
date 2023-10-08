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
private:
	struct Triangle
	{
		sf::Vector2f v0;
		sf::Vector2f v1;
		sf::Vector2f v2;
	};

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
		float interp, Policy policy);

public:
	static void TurnAtBorder(const sf::Vector2f& pos, sf::Vector2f& vel, std::uint32_t den, const RectFloat& border, float dt);
	static bool TeleportAtBorder(sf::Vector2f& pos, const RectFloat& border);

public:
	static sf::Vector2f SteerAt(const sf::Vector2f& prevVel, const sf::Vector2f& steerDir);

	static void SteerTowards(sf::Vector2f& vel, const sf::Vector2f& prevVel, const sf::Vector2f& direction, float length, float weight);
	static void SteerTowards(sf::Vector2f& vel, const sf::Vector2f& prevVel, const sf::Vector2f& point, float weight);

	void ResetCycleTimes();

private:
	static sf::Vector2f GetOrigin(const sf::Vector2f& pos);

	static sf::Vector3f PositionColor(const sf::Vector2f& pos, const RectFloat& border);
	static sf::Vector3f CycleColor(float cycleTime);
	static sf::Vector3f DensityColor(std::uint32_t density, float densityTime);
	static sf::Vector3f VelocityColor(float speed);
	static sf::Vector3f RotationColor(float angle);
	static sf::Vector3f AudioColor(std::uint32_t density, float volume);
	static void ImpulseColor(const sf::Vector2f& pos, sf::Vector3f& color, const Impulse& impulse);

private:
	std::unique_ptr<std::uint32_t[]>	m_indices;
	std::unique_ptr<Triangle[]>			m_triangles;
	std::unique_ptr<Triangle[]>			m_prevTriangles;

	std::unique_ptr<sf::Vector2f[]>		m_positions;
	std::unique_ptr<sf::Vector2f[]>		m_velocities;
	std::unique_ptr<sf::Vector2f[]>		m_prevVelocities;
	std::unique_ptr<sf::Vector2f[]>		m_relativePositions;
	std::unique_ptr<sf::Vector3f[]>		m_colors;

	std::unique_ptr<float[]>			m_speeds;
	std::unique_ptr<float[]>			m_angles;
	std::unique_ptr<float[]>			m_cycleTimes;
	std::unique_ptr<float[]>			m_densityTimes;

	std::unique_ptr<std::uint32_t[]>	m_densities;
	std::unique_ptr<std::uint16_t[]>	m_cellIndices;

	std::unique_ptr<bool[]>				m_teleported;

	std::size_t	m_size		{0};
	std::size_t	m_capacity	{0};
};
