#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

#include "Impulse.h"

static const std::string FILE_NAME = "config";

enum Reconstruct
{
	RGrid,
	RBoids,
	RBackgroundTex,
	RBackgroundProp,
	RAudio,
	RWindow,
	RCamera,
	RPhysics,
	RDebug
};

struct Config
{
	sf::Vector3f background_color;
	std::string background_texture;
	int background_position_x;
	int background_position_y;
	bool background_fit_screen;
	bool background_override_size;
	int background_width;
	int background_height;

	int boid_count;
	float boid_size_width;
	float boid_size_height;
	float boid_max_speed;
	float boid_min_speed;
	float boid_max_steer;
	float boid_view_angle;

	float sep_distance;
	float ali_distance;
	float coh_distance;

	float sep_weight;
	float ali_weight;
	float coh_weight;

	int color_option;

	sf::Vector3f boid_color_top_left;
	sf::Vector3f boid_color_top_right;
	sf::Vector3f boid_color_bot_left;
	sf::Vector3f boid_color_bot_right;

	bool boid_cycle_colors_random;
	float boid_cycle_colors_speed;
	std::vector<sf::Vector3f> boid_cycle_colors;

	int boid_density;
	bool boid_density_cycle_enabled;
	float boid_density_cycle_speed;
	std::vector<sf::Vector3f> boid_density_colors;

	std::vector<std::wstring> audio_responsive_apps;
	float audio_responsive_strength;
	float audio_responsive_limit;
	int audio_responsive_density;
	std::vector<sf::Vector3f> audio_responsive_colors;

	bool impulse_enabled;
	float impulse_size;
	float impulse_speed;
	float impulse_fade_distance;
	std::vector<sf::Vector3f> impulse_colors;

	bool steer_enabled;
	float steer_towards_factor;
	float steer_away_factor;

	bool predator_enabled;
	float predator_distance;
	float predator_factor;

	bool turn_at_border;
	float turn_margin_factor;
	float turn_factor;

	int grid_extra_cells;
	bool camera_enabled;
	float camera_zoom;
	bool vertical_sync;
	int max_framerate;
	float physics_update_freq;

	bool debug_enabled;
	float debug_update_freq;
	int debug_toggle_key;

	std::vector<Impulse> impulses;
	float min_distance;
	float volume;

public:
	Config();
	~Config();

	void load();
	void load_var(nlohmann::json& json);
	std::vector<Reconstruct> refresh(Config& prev);

private:
	sf::Vector3f convert(std::string strColor) const
	{
		std::stringstream stream(strColor);
		std::string segment;
		std::vector<std::string> values;

		while (std::getline(stream, segment, ' '))
			values.push_back(segment);

		sf::Vector3f color(1.0f, 1.0f, 1.0f);

		try
		{
			color.x = std::stof(values[0]) / 255.0f;
			color.y = std::stof(values[1]) / 255.0f;
			color.z = std::stof(values[2]) / 255.0f;
		}
		catch (std::exception e) {}

		return color;
	}
};
