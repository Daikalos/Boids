#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

#include "Impulse.h"

static const std::string FILE_NAME = "config";

// ugly but does not matter in small project
//
struct Config
{
	static sf::Vector3f background_color;
	static std::string background_texture;
	static int background_position_x;
	static int background_position_y;
	static bool background_fit_screen;
	static bool background_override_size;
	static int background_width;
	static int background_height;

	static int boid_count;

	static float boid_size_width;
	static float boid_size_height;
	static float boid_max_speed;
	static float boid_min_speed;
	static float boid_max_steer;
	static float boid_view_angle;

	static float sep_distance;
	static float ali_distance;
	static float coh_distance;

	static float sep_weight;
	static float ali_weight;
	static float coh_weight;

	static int color_option;

	static sf::Vector3f boid_color_top_left;
	static sf::Vector3f boid_color_top_right;
	static sf::Vector3f boid_color_bot_left;
	static sf::Vector3f boid_color_bot_right;

	static bool boid_cycle_colors_random;
	static float boid_cycle_colors_speed;
	static std::vector<sf::Vector3f> boid_cycle_colors;

	static int boid_density;
	static bool boid_density_cycle_enabled;
	static float boid_density_cycle_speed;
	static std::vector<sf::Vector3f> boid_density_colors;

	static std::vector<std::wstring> audio_responsive_apps;
	static float audio_responsive_strength;
	static float audio_responsive_limit;
	static int audio_responsive_density;
	static std::vector<sf::Vector3f> audio_responsive_colors;

	static bool impulse_enabled;
	static float impulse_size;
	static float impulse_speed;
	static float impulse_fade_distance;
	static std::vector<sf::Vector3f> impulse_colors;

	static bool steer_enabled;
	static float steer_towards_factor;
	static float steer_away_factor;

	static bool predator_enabled;
	static float predator_distance;
	static float predator_factor;

	static bool turn_at_border;
	static float turn_margin_factor;
	static float turn_factor;

	static int grid_extra_cells;

	static bool camera_enabled;
	static float camera_zoom;
	static bool vertical_sync;
	static int max_framerate;
	static float physics_update_freq;

	static bool debug_enabled;
	static int debug_key;

	static std::vector<Impulse> impulses;
	static float min_distance;
	static float volume;

	static void load()
	{
		std::ifstream project_file(FILE_NAME + ".json", std::ifstream::binary);

		if (project_file.good())
		{
			nlohmann::json json = nlohmann::json::parse(project_file);
			if (!json.empty())
			{
				try
				{
					auto config = json[FILE_NAME];

					background_color = convert(config["background_color"]);
					background_texture = config["background_texture"];
					background_position_x = config["background_position_x"];
					background_position_y = config["background_position_y"];
					background_fit_screen = config["background_fit_screen"];
					background_override_size = config["background_override_size"];
					background_width = config["background_width"];
					background_height = config["background_height"];

					boid_count = config["boid_count"];
					boid_size_width = config["boid_size_width"];
					boid_size_height = config["boid_size_height"];
					boid_max_speed = config["boid_max_speed"];
					boid_min_speed = config["boid_min_speed"];
					boid_max_steer = config["boid_max_steer"];
					boid_view_angle = config["boid_view_angle"];

					sep_distance = config["sep_distance"];
					ali_distance = config["ali_distance"];
					coh_distance = config["coh_distance"];

					sep_weight = config["sep_weight"];
					ali_weight = config["ali_weight"];
					coh_weight = config["coh_weight"];

					color_option = config["color_option"];

					switch (Config::color_option)
					{
					case 0:
					{
						boid_color_top_left = convert(config["boid_color_top_left"]);
						boid_color_top_right = convert(config["boid_color_top_right"]);
						boid_color_bot_left = convert(config["boid_color_bot_left"]);
						boid_color_bot_right = convert(config["boid_color_bot_right"]);
					}
					break;
					case 2:
					{
						boid_density = config["boid_density"];
						boid_density_cycle_enabled = config["boid_density_cycle_enabled"];
						boid_density_cycle_speed = config["boid_density_cycle_speed"];

						std::vector<std::string> temp_colors = config["boid_density_colors"];
						boid_density_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_density_colors[i] = convert(temp_colors[i]);
					}
					break;
					case 3:
					{
						std::vector<std::string> temp_processes = config["audio_responsive_apps"];
						audio_responsive_apps = std::vector<std::wstring>(temp_processes.size());

						for (int i = 0; i < audio_responsive_apps.size(); ++i)
						{
							std::string process = temp_processes[i];
							audio_responsive_apps[i] = std::wstring(process.begin(), process.end());
						}

						audio_responsive_strength = config["audio_responsive_strength"];
						audio_responsive_limit = config["audio_responsive_limit"];
						audio_responsive_density = config["audio_responsive_density"];

						std::vector<std::string> temp_colors = config["audio_responsive_colors"];
						audio_responsive_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							audio_responsive_colors[i] = convert(temp_colors[i]);
					}
					break;
					default:
					{
						boid_cycle_colors_random = config["boid_cycle_colors_random"];
						boid_cycle_colors_speed = config["boid_cycle_colors_speed"];

						std::vector<std::string> temp_colors = config["boid_cycle_colors"];
						boid_cycle_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_cycle_colors[i] = convert(temp_colors[i]);
					}
					break;
					}

					impulse_enabled = config["impulse_enabled"];
					impulse_size = config["impulse_size"];
					impulse_speed = config["impulse_speed"];
					impulse_fade_distance = config["impulse_fade_distance"];

					{
						std::vector<std::string> temp_colors = config["impulse_colors"];
						impulse_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							impulse_colors[i] = convert(temp_colors[i]);
					}

					steer_enabled = config["steer_enabled"];
					steer_towards_factor = config["steer_towards_factor"];
					steer_away_factor = config["steer_away_factor"];

					predator_enabled = config["predator_enabled"];
					predator_distance = config["predator_distance"];
					predator_factor = config["predator_factor"];

					turn_at_border = config["turn_at_border"];
					turn_margin_factor = config["turn_margin_factor"];
					turn_factor = config["turn_factor"];

					grid_extra_cells = config["grid_extra_cells"];
					camera_enabled = config["camera_enabled"];
					camera_zoom = config["camera_zoom"];
					vertical_sync = config["vertical_sync"];
					max_framerate = config["max_framerate"];
					physics_update_freq = config["physics_update_freq"];

					debug_enabled = config["debug_enabled"];
					debug_key = config["debug_key"];
				}
				catch (std::exception e) {}
			}
		}

		min_distance = std::fmaxf(std::fmaxf(Config::sep_distance, Config::ali_distance), Config::coh_distance);

		sep_distance *= sep_distance;
		ali_distance *= ali_distance;
		coh_distance *= coh_distance;

		predator_distance *= predator_distance;
	}

	static inline sf::Vector3f convert(std::string strColor)
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
