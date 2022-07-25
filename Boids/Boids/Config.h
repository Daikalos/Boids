#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

#include "json.hpp"
#include "Impulse.h"

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

	static std::vector<std::wstring> audio_responsive_processes;
	static float audio_responsive_strength;
	static float audio_responsive_limit;
	static int audio_responsive_density;
	static std::vector<sf::Vector3f> audio_responsive_colors;

	static bool impulse_enabled;
	static float impulse_size;
	static float impulse_speed;
	static float impulse_fade_distance;
	static std::vector<sf::Vector3f> impulse_colors;

	static bool gravity_enabled;
	static float gravity_towards_factor;
	static float gravity_away_factor;

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

	static std::vector<Impulse> impulses;
	static float min_distance;
	static float volume;

	static void load()
	{
		std::ifstream project_file("settings.json", std::ifstream::binary);
		nlohmann::json json = nlohmann::json::parse(project_file);

		if (!json.empty())
		{
			try
			{
				background = convert(json["settings"]["background"]);

				boid_count = json["settings"]["boid_count"];
				boid_size_width = json["settings"]["boid_size_width"];
				boid_size_height = json["settings"]["boid_size_height"];
				boid_max_speed = json["settings"]["boid_max_speed"];
				boid_min_speed = json["settings"]["boid_min_speed"];
				boid_max_steer = json["settings"]["boid_max_steer"];
				boid_view_angle = json["settings"]["boid_view_angle"];

				sep_distance = json["settings"]["sep_distance"];
				ali_distance = json["settings"]["ali_distance"];
				coh_distance = json["settings"]["coh_distance"];

				sep_weight = json["settings"]["sep_weight"];
				ali_weight = json["settings"]["ali_weight"];
				coh_weight = json["settings"]["coh_weight"];

				color_option = json["settings"]["color_option"];

				switch (Config::color_option)
				{
					case 0:
					{
						boid_color_top_left = convert(json["settings"]["boid_color_top_left"]);
						boid_color_top_right = convert(json["settings"]["boid_color_top_right"]);
						boid_color_bot_left = convert(json["settings"]["boid_color_bot_left"]);
						boid_color_bot_right = convert(json["settings"]["boid_color_bot_right"]);
					}
					break;
					case 2:
					{
						boid_density = json["settings"]["boid_density"];
						boid_density_cycle_enabled = json["settings"]["boid_density_cycle_enabled"];
						boid_density_cycle_speed = json["settings"]["boid_density_cycle_speed"];

						std::vector<std::string> temp_colors = json["settings"]["boid_density_colors"];
						boid_density_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_density_colors[i] = convert(temp_colors[i]);
					}
					break;
					default:
					{
						boid_cycle_colors_random = json["settings"]["boid_cycle_colors_random"];
						boid_cycle_colors_speed = json["settings"]["boid_cycle_colors_speed"];

						std::vector<std::string> temp_colors = json["settings"]["boid_cycle_colors"];
						boid_cycle_colors = std::vector<sf::Vector3f>(temp_colors.size());

						for (int i = 0; i < temp_colors.size(); ++i)
							boid_cycle_colors[i] = convert(temp_colors[i]);
					}
					break;
				}

				{
					std::vector<std::string> temp_processes = json["settings"]["audio_responsive_processes"];
					audio_responsive_processes = std::vector<std::wstring>(temp_processes.size());

					for (int i = 0; i < audio_responsive_processes.size(); ++i)
					{
						std::string process = temp_processes[i];
						audio_responsive_processes[i] = std::wstring(process.begin(), process.end());
					}
				}

				audio_responsive_strength = json["settings"]["audio_responsive_strength"];
				audio_responsive_limit = json["settings"]["audio_responsive_limit"];
				audio_responsive_density = json["settings"]["audio_responsive_density"];

				{
					std::vector<std::string> temp_colors = json["settings"]["audio_responsive_colors"];
					audio_responsive_colors = std::vector<sf::Vector3f>(temp_colors.size());

					for (int i = 0; i < temp_colors.size(); ++i)
						audio_responsive_colors[i] = convert(temp_colors[i]);
				}

				impulse_enabled = json["settings"]["impulse_enabled"];
				impulse_size = json["settings"]["impulse_size"];
				impulse_speed = json["settings"]["impulse_speed"];
				impulse_fade_distance = json["settings"]["impulse_fade_distance"];

				{
					std::vector<std::string> temp_colors = json["settings"]["impulse_colors"];
					impulse_colors = std::vector<sf::Vector3f>(temp_colors.size());

					for (int i = 0; i < temp_colors.size(); ++i)
						impulse_colors[i] = convert(temp_colors[i]);
				}

				gravity_enabled = json["settings"]["gravity_enabled"];
				gravity_towards_factor = json["settings"]["gravity_towards_factor"];
				gravity_away_factor = json["settings"]["gravity_away_factor"];

				predator_enabled = json["settings"]["predator_enabled"];
				predator_distance = json["settings"]["predator_distance"];
				predator_factor = json["settings"]["predator_factor"];

				turn_at_border = json["settings"]["turn_at_border"];
				turn_margin_factor = json["settings"]["turn_margin_factor"];
				turn_factor = json["settings"]["turn_factor"];

				grid_extra_cells = json["settings"]["grid_extra_cells"];
				camera_enabled = json["settings"]["camera_enabled"];
				camera_zoom = json["settings"]["camera_zoom"];
				vertical_sync = json["settings"]["vertical_sync"];
				max_framerate = json["settings"]["max_framerate"];
			}
			catch (std::exception e) { }
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
