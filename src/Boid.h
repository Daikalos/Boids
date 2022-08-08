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

	void set_cell_index()
	{
		this->cell_index = grid->at_pos(get_origin());
	}
	int get_cell_index() const
	{
		return cell_index;
	}

private:
	void flock(const std::vector<Boid>& boids);

	sf::Vector2f steer_at(const sf::Vector2f& steer_direction);

	void update_points();

	sf::Vector2f outside_border(sf::Vector2f pos, const float& dt);
	sf::Vector2f turn_at_border(const sf::Vector2f& pos, const float& dt);
	sf::Vector2f teleport_at_border(sf::Vector2f& pos);

	void position_color();
	void cycle_color(const float& dt);
	void density_color(const float& dt);
	void audio_color(const float& dt);
	void impulse_color(const std::vector<Impulse>& impulses);

	int interpolate(int a, int b, int c, int d, double t, double s) const
	{
		return (int)(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

	inline void apply_force(const sf::Vector2f& force)
	{
		velocity += force;
	}

public: // Properties
	inline sf::Vector2f get_prev_pointA() const { return prev_pointA; }
	inline sf::Vector2f get_prev_pointB() const { return prev_pointB; }
	inline sf::Vector2f get_prev_pointC() const { return prev_pointC; }

	inline sf::Vector2f get_pointA() const { return pointA; }
	inline sf::Vector2f get_pointB() const { return pointB; }
	inline sf::Vector2f get_pointC() const { return pointC; }

	inline sf::Vector3f get_color() const { return color; }

	inline sf::Vector2f get_position() const { return position; }
	inline sf::Vector2f get_velocity() const { return velocity; }
	inline float get_rotation() const { return rotation; }

	inline sf::Vector2f get_origin() const
	{
		return origin;
	}

private:
	Grid* grid;
	Config* config;
	const AudioMeter* audio_meter;
	const Rect_i* border;

	int cell_index{0};

	sf::Vector2f pointA, pointB, pointC;
	sf::Vector2f prev_pointA, prev_pointB, prev_pointC;

	sf::Vector2f position;
	sf::Vector2f velocity;
	sf::Vector2f origin;
	float rotation{0.0f};

	sf::Vector3f color;

	float cycle_time{0.0f};
	float density_time{0.0f};
	int density{0};
};

