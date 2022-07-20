#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

#include "json.hpp"

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
	static float boid_min_distance;

	static sf::Vector3f boid_color_top_left;
	static sf::Vector3f boid_color_top_right;
	static sf::Vector3f boid_color_bot_left;
	static sf::Vector3f boid_color_bot_right;

	static bool boid_cycle_colors_enabled;
	static bool boid_cycle_colors_random;
	static float boid_cycle_colors_speed;
	static std::vector<sf::Vector3f> boid_cycle_colors;

	static float weight_sep;
	static float weight_ali;
	static float weight_coh;

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
	static bool vertical_sync;
	static int max_framerate;

	static void load()
	{
		std::ifstream project_file("settings.json", std::ifstream::binary);
		std::stringstream buffer;

		buffer << project_file.rdbuf();

		nlohmann::json json;
		if (!buffer.str().empty())
			json = nlohmann::json::parse(buffer.str());

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
				boid_min_distance = json["settings"]["boid_min_distance"];

				boid_color_top_left = convert(json["settings"]["boid_color_top_left"]);
				boid_color_top_right = convert(json["settings"]["boid_color_top_right"]);
				boid_color_bot_left = convert(json["settings"]["boid_color_bot_left"]);
				boid_color_bot_right = convert(json["settings"]["boid_color_bot_right"]);

				boid_cycle_colors_enabled = json["settings"]["boid_cycle_colors_enabled"];
				boid_cycle_colors_random = json["settings"]["boid_cycle_colors_random"];
				boid_cycle_colors_speed = json["settings"]["boid_cycle_colors_speed"];

				std::vector<std::string> temp_colors = json["settings"]["boid_cycle_colors"];
				boid_cycle_colors = std::vector<sf::Vector3f>(temp_colors.size());

				for (int i = 0; i < temp_colors.size(); ++i)
					boid_cycle_colors[i] = convert(temp_colors[i]);

				weight_sep = json["settings"]["separation"];
				weight_ali = json["settings"]["alignment"];
				weight_coh = json["settings"]["cohesion"];

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
				vertical_sync = json["settings"]["vertical_sync"];
				max_framerate = json["settings"]["max_framerate"];
			}
			catch (std::exception e) { }
		}
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
