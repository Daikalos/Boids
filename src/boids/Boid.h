#pragma once

#include <SFML/Graphics.hpp>

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
	Boid(Grid& grid, Config& config, const AudioMeter& audio_meter, const RectInt& border, const sf::Vector2f& pos);

	void steer_towards(const sf::Vector2f& direction, const float length, const float weight);
	void steer_towards(const sf::Vector2f& point, const float weight);

	void update_grid_cells(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids, const int index) const;
	void update(const std::vector<Impulse>& impulses, float dt);
	void flock(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids);
	
public: // Properties
	constexpr sf::Vector2f get_position() const noexcept { return _position; }
	constexpr sf::Vector2f get_prev_position() const noexcept { return _prev_position; }
	constexpr sf::Vector2f get_velocity() const noexcept { return _velocity; }
	constexpr sf::Vector2f get_prev_velocity() const noexcept { return _prev_velocity; }
	constexpr sf::Vector3f get_color() const noexcept { return _color; }
	constexpr int get_cell_index() const noexcept { return _cell_index; }

	constexpr sf::Vector2f get_origin() const
	{
		return _position + sf::Vector2f(
			_config->boid_size_width,
			_config->boid_size_height) / 2.0f;
	}
	constexpr sf::Vector2f get_prev_origin() const
	{
		return _prev_position + sf::Vector2f(
			_config->boid_size_width,
			_config->boid_size_height) / 2.0f;
	}

	void set_cell_index() 
	{ 
		_prev_position = _position;
		_prev_velocity = _velocity;

		_cell_index = _grid->at_pos(get_origin());
	}
	void set_cycle_time(const float val) 
	{ 
		_cycle_time = val;
	}

private:
	sf::Vector2f steer_at(const sf::Vector2f& steer_direction) const;

	bool outside_border(float dt);
	bool turn_at_border(float dt);
	bool teleport_at_border();

	sf::Vector3f position_color() const;
	sf::Vector3f cycle_color(float dt);
	sf::Vector3f density_color(float dt);
	sf::Vector3f velocity_color() const;
	sf::Vector3f rotation_color() const;
	sf::Vector3f audio_color(float dt) const;
	sf::Vector3f impulse_color(const std::vector<Impulse>& impulses) const;

private:
	Grid*				_grid;
	Config*				_config;
	const AudioMeter*	_audio_meter;
	const RectInt*		_border;

	sf::Vector2f		_position, _prev_position;
	sf::Vector2f		_velocity, _prev_velocity;
	sf::Vector3f		_color;

	float				_cycle_time{0.0f};
	float				_density_time{0.0f};

	int					_density{0};
	int					_cell_index{0};
};

