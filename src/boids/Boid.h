#pragma once

#include <SFML/Graphics.hpp>

#include <span>

#include "Grid.hpp"
#include "Config.h"
#include "AudioMeter.h"
#include "Impulse.hpp"

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"
#include "../utilities/Utilities.h"

class Boid
{
public:
	Boid(Grid& grid, Config& config, const sf::Vector2f& pos);

	void steer_towards(const sf::Vector2f& direction, const float length, const float weight);
	void steer_towards(const sf::Vector2f& point, const float weight);

	void pre_update() noexcept;

	void update_grid_cells(std::span<const Boid> boids, std::span<const int> sorted_boids, const int index) const;
	void flock(std::span<const Boid> boids, std::span<const int> sorted_boids);
	void update(const RectInt& border, const AudioMeter& audio_meter, std::span<const Impulse> impulses, float dt);
	
public: // Properties
	[[nodiscard]] constexpr sf::Vector2f get_position() const noexcept { return _position; }
	[[nodiscard]] constexpr sf::Vector2f get_prev_position() const noexcept { return _prev_position; }
	[[nodiscard]] constexpr sf::Vector2f get_relative_position() const noexcept { return _relative_pos; }
	[[nodiscard]] constexpr sf::Vector2f get_velocity() const noexcept { return _velocity; }
	[[nodiscard]] constexpr sf::Vector2f get_prev_velocity() const noexcept { return _prev_velocity; }
	[[nodiscard]] constexpr sf::Vector3f get_color() const noexcept { return _color; }
	[[nodiscard]] constexpr int get_cell_index() const noexcept { return _cell_index; }

	[[nodiscard]] constexpr sf::Vector2f get_origin() const noexcept
	{
		return _position + sf::Vector2f(
			_config->boid_size_width,
			_config->boid_size_height) / 2.0f;
	}
	[[nodiscard]] constexpr sf::Vector2f get_prev_origin() const noexcept
	{
		return _prev_position + sf::Vector2f(
			_config->boid_size_width,
			_config->boid_size_height) / 2.0f;
	}

	void set_cycle_time(const float val) noexcept
	{ 
		_cycle_time = val;
	}

private:
	sf::Vector2f steer_at(const sf::Vector2f& steer_direction) const;

	bool outside_border(const RectInt& border, float dt);
	bool turn_at_border(const RectInt& border, float dt);
	bool teleport_at_border(const RectInt& border);

	sf::Vector3f position_color(const RectInt& border) const;
	sf::Vector3f cycle_color(float dt);
	sf::Vector3f density_color(float dt);
	sf::Vector3f velocity_color() const;
	sf::Vector3f rotation_color() const;
	sf::Vector3f audio_color(const AudioMeter& audio_meter, float dt) const;
	sf::Vector3f impulse_color(std::span<const Impulse> impulses) const;

private:
	Grid*				_grid						{nullptr};
	Config*				_config						{nullptr};

	sf::Vector2f		_position, _prev_position, _relative_pos;
	sf::Vector2f		_velocity, _prev_velocity;

	sf::Vector3f		_color;
	float				_cycle_time					{0.0f};
	float				_density_time				{0.0f};

	int					_density					{0};
	int					_cell_index					{0};
};

