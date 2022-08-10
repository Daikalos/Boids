#pragma once

#include <SFML/Graphics.hpp>

#include "Utilities.h"
#include "VecUtil.h"
#include "Rectangle.h"
#include "Config.h"
#include "Grid.h"
#include "AudioMeter.h"
#include "Impulse.h"

class Boid
{
public:
	Boid(Grid& grid, Config& config, const AudioMeter& audio_meter, const Rect_i& border, const sf::Vector2f& pos);

	void update(const std::vector<Boid>& boids, const std::vector<Impulse>& impulses, const float& physics_dt);
	void steer_towards(sf::Vector2f point, float weight);
	void update_grid_cells(const std::vector<Boid>& boids) const;

public: // Properties
	inline sf::Vector2f get_position() const { return position; }
	inline sf::Vector2f get_prev_position() const { return prev_position; }
	inline sf::Vector2f get_velocity() const { return velocity; }
	inline sf::Vector2f get_prev_velocity() const { return prev_velocity; }
	inline sf::Vector3f get_color() const { return color; }

	inline sf::Vector2f get_origin() const 
	{ 
		return sf::Vector2f(
			position.x + config->boid_size_width / 2.0f, 
			position.y + config->boid_size_height / 2.0f);
	}

	inline int get_cell_index() const { return cell_index; }

	void set_cell_index() { cell_index = grid->at_pos(get_origin()); }
	void set_cycle_time(float val) { cycle_time = val; }

private:
	void flock(const std::vector<Boid>& boids);

	sf::Vector2f steer_at(const sf::Vector2f& steer_direction);

	bool outside_border(const float& dt);
	bool turn_at_border(const float& dt);
	bool teleport_at_border();

	void position_color();
	void cycle_color(const float& dt);
	void density_color(const float& dt);
	void audio_color(const float& dt);
	void impulse_color(const std::vector<Impulse>& impulses);

	inline void apply_force(const sf::Vector2f& force)
	{
		velocity += force;
	}

private:
	Grid* grid;
	Config* config;
	const AudioMeter* audio_meter;
	const Rect_i* border;

	sf::Vector2f position, prev_position;
	sf::Vector2f velocity, prev_velocity;
	sf::Vector3f color;

	float cycle_time{0.0f};
	float density_time{0.0f};

	int density{0};
	int cell_index{0};
};

