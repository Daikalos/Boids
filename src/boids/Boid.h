#pragma once

#include <SFML/Graphics.hpp>

#include <span>

#include "Grid.h"
#include "Config.h"
#include "AudioMeter.h"
#include "Impulse.hpp"
#include "Fluid.h"

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"
#include "../utilities/Utilities.h"

class Boid
{
public:
	Boid(Config& config, const sf::Vector2f& pos);
	Boid(Config& config, const sf::Vector2f& pos, const sf::Vector2f& velocity);

	void steer_towards(const sf::Vector2f& direction, const float length, const float weight);
	void steer_towards(const sf::Vector2f& point, const float weight);

	void pre_update(const Grid& grid) noexcept;
	void update_grid_cells(Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy, const std::uint32_t index) const;
	void flock(const Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy);
	void update(const RectFloat& border, const IAudioMeterInfo* audio_meter, std::span<const Impulse> impulses, float dt);
	
public: // Properties
	[[nodiscard]] const sf::Vector2f& get_position() const noexcept;
	[[nodiscard]] const sf::Vector2f& get_prev_position() const noexcept;
	[[nodiscard]] const sf::Vector2f& get_relative_position() const noexcept;
	[[nodiscard]] const sf::Vector2f& get_velocity() const noexcept;
	[[nodiscard]] const sf::Vector2f& get_prev_velocity() const noexcept;
	[[nodiscard]] const sf::Vector3f& get_color() const noexcept;
	[[nodiscard]] std::uint16_t get_cell_index() const noexcept;

	[[nodiscard]] sf::Vector2f get_origin() const noexcept;
	[[nodiscard]] sf::Vector2f get_prev_origin() const noexcept;

	void set_cycle_time(const float val) noexcept;

private:
	sf::Vector2f steer_at(const sf::Vector2f& steer_direction) const;

	bool outside_border(const RectFloat& border, float dt);
	bool turn_at_border(const RectFloat& border, float dt);
	bool teleport_at_border(const RectFloat& border);

	sf::Vector3f position_color(const RectFloat& border) const;
	sf::Vector3f cycle_color(float dt);
	sf::Vector3f density_color(float dt);
	sf::Vector3f velocity_color() const;
	sf::Vector3f rotation_color() const;
	sf::Vector3f audio_color(const IAudioMeterInfo* audio_meter, float dt) const;
	sf::Vector3f impulse_color(std::span<const Impulse> impulses) const;

private:
	Config*			_config			{nullptr};

	sf::Vector2f	_position, _prev_position, _relative_pos;
	sf::Vector2f	_velocity, _prev_velocity;

	sf::Vector3f	_color;
	float			_cycle_time		{0.0f};
	float			_density_time	{0.0f};

	std::uint16_t	_density		{0};
	std::uint16_t	_cell_index		{0};
};

