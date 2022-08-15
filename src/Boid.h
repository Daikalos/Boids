#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities.h"
#include "VecUtil.h"
#include "Rectangle.hpp"
#include "Config.h"
#include "Grid.hpp"
#include "AudioMeter.h"
#include "Impulse.hpp"

class Boid
{
public:
	Boid(Grid& grid, Config& config, const AudioMeter& audio_meter, const RectInt& border, const sf::Vector2f& pos);

	void steer_towards(sf::Vector2f point, float weight);
	void update_grid_cells(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids, int index) const;
	void update(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids, const std::vector<Impulse>& impulses, float dt);
	
public: // Properties
	inline sf::Vector2f get_position() const noexcept { return position; }
	inline sf::Vector2f get_prev_position() const noexcept { return prev_position; }
	inline sf::Vector2f get_saved_position() const noexcept { return saved_position; }
	inline sf::Vector2f get_velocity() const noexcept { return velocity; }
	inline sf::Vector2f get_prev_velocity() const noexcept { return prev_velocity; }
	inline sf::Vector2f get_saved_velocity() const noexcept { return saved_velocity; }
	inline sf::Vector3f get_color() const noexcept { return color; }
	inline int get_cell_index() const noexcept { return cell_index; }

	inline sf::Vector2f get_origin() const
	{
		return position + sf::Vector2f(
			config->boid_size_width,
			config->boid_size_height) / 2.0f;
	}
	inline sf::Vector2f get_prev_origin() const
	{
		return prev_position + sf::Vector2f(
			config->boid_size_width,
			config->boid_size_height) / 2.0f;
	}
	inline sf::Vector2f get_saved_origin() const
	{
		return saved_position + sf::Vector2f(
			config->boid_size_width,
			config->boid_size_height) / 2.0f;
	}

	void set_cell_index() 
	{ 
		saved_position = position; // usually you swap pos and vel buffers, but this works as well
		saved_velocity = velocity;

		cell_index = grid->at_pos(get_origin());
	}
	void set_cycle_time(float val) 
	{ 
		cycle_time = val; 
	}

private:
	void flock(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids);

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
	Grid* grid;
	Config* config;
	const AudioMeter* audio_meter;
	const RectInt* border;

	sf::Vector2f position, prev_position, saved_position;
	sf::Vector2f velocity, prev_velocity, saved_velocity;
	sf::Vector3f color;

	float cycle_time{0.0f};
	float density_time{0.0f};

	int density{0};
	int cell_index{0};
};

