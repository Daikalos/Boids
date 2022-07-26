#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>

#include "json.hpp"
#include "Impulse.h"

static const std::string FILE_NAME = "config";

// ugly but does not matter in small project
//
struct Config
{
	static sf::Vector3f background;

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
					background = convert(json[FILE_NAME]["background"]);

					boid_count = json[FILE_NAME]["boid_count"];
					boid_size_width = json[FILE_NAME]["boid_size_width"];
					boid_size_height = json[FILE_NAME]["boid_size_height"];
					boid_max_speed = json[FILE_NAME]["boid_max_speed"];
					boid_min_speed = json[FILE_NAME]["boid_min_speed"];
					boid_max_steer = json[FILE_NAME]["boid_max_steer"];
					boid_view_angle = json[FILE_NAME]["boid_view_angle"];

					sep_distance = json[FILE_NAME]["sep_distance"];
					ali_distance = json[FILE_NAME]["ali_distance"];
					coh_distance = json[FILE_NAME]["coh_distance"];

					sep_weight = json[FILE_NAME]["sep_weight"];
					ali_weight = json[FILE_NAME]["ali_weight"];
					coh_weight = json[FILE_NAME]["coh_weight"];

					color_option = json[FILE_NAME]["color_option"];

					switch (Config::color_option)
					{
					case 0:
					{
						boid_color_top_left = convert(json[FILE_NAME]["boid_color_top_left"]);
						boid_color_top_right = convert(json[FILE_NAME]["boid_color_top_right"]);
						boid_color_bot_left = convert(json[FILE_NAME]["boid_color_bot_left"]);
						boid_color_bot_right = convert(json[FILE_NAME]["boid_color_bot_right"]);
					}
					break;
					case 2:
					{
						boid_density = json[FILE_NAME]["boid_density"];
						boid_density_cycle_enabled = json[FILE_NAME]["boid_density_cycle_enabled"];
						boid_density_cycle_speed = json[FILE_NAME]["boid_density_cycle_speed"];

						std::vector<std::string> temp_colors = json[FILE_NAME]["boid_density_colors"];
						boid_density_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_density_colors[i] = convert(temp_colors[i]);
					}
					break;
					case 3:
					{
						std::vector<std::string> temp_processes = json[FILE_NAME]["audio_responsive_apps"];
						audio_responsive_apps = std::vector<std::wstring>(temp_processes.size());

						for (int i = 0; i < audio_responsive_apps.size(); ++i)
						{
							std::string process = temp_processes[i];
							audio_responsive_apps[i] = std::wstring(process.begin(), process.end());
						}

						audio_responsive_strength = json[FILE_NAME]["audio_responsive_strength"];
						audio_responsive_limit = json[FILE_NAME]["audio_responsive_limit"];
						audio_responsive_density = json[FILE_NAME]["audio_responsive_density"];

						std::vector<std::string> temp_colors = json[FILE_NAME]["audio_responsive_colors"];
						audio_responsive_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							audio_responsive_colors[i] = convert(temp_colors[i]);
					}
					break;
					default:
					{
						boid_cycle_colors_random = json[FILE_NAME]["boid_cycle_colors_random"];
						boid_cycle_colors_speed = json[FILE_NAME]["boid_cycle_colors_speed"];

						std::vector<std::string> temp_colors = json[FILE_NAME]["boid_cycle_colors"];
						boid_cycle_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_cycle_colors[i] = convert(temp_colors[i]);
					}
					break;
					}

					impulse_enabled = json[FILE_NAME]["impulse_enabled"];
					impulse_size = json[FILE_NAME]["impulse_size"];
					impulse_speed = json[FILE_NAME]["impulse_speed"];
					impulse_fade_distance = json[FILE_NAME]["impulse_fade_distance"];

					{
						std::vector<std::string> temp_colors = json[FILE_NAME]["impulse_colors"];
						impulse_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							impulse_colors[i] = convert(temp_colors[i]);
					}

					steer_enabled = json[FILE_NAME]["steer_enabled"];
					steer_towards_factor = json[FILE_NAME]["steer_towards_factor"];
					steer_away_factor = json[FILE_NAME]["steer_away_factor"];

					predator_enabled = json[FILE_NAME]["predator_enabled"];
					predator_distance = json[FILE_NAME]["predator_distance"];
					predator_factor = json[FILE_NAME]["predator_factor"];

					turn_at_border = json[FILE_NAME]["turn_at_border"];
					turn_margin_factor = json[FILE_NAME]["turn_margin_factor"];
					turn_factor = json[FILE_NAME]["turn_factor"];

					grid_extra_cells = json[FILE_NAME]["grid_extra_cells"];
					camera_enabled = json[FILE_NAME]["camera_enabled"];
					camera_zoom = json[FILE_NAME]["camera_zoom"];
					vertical_sync = json[FILE_NAME]["vertical_sync"];
					max_framerate = json[FILE_NAME]["max_framerate"];
					physics_update_freq = json[FILE_NAME]["physics_update_freq"];
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
