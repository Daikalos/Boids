#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include "json.hpp"

struct Config
{
	static sf::Vector3f background;

	static size_t boid_count;

	static float boid_size_width;
	static float boid_size_height;
	static float boid_max_speed;
	static float boid_max_steer;
	static float boid_view_angle;
	static float boid_min_distance;

	static sf::Vector3f boid_color_top_left;
	static sf::Vector3f boid_color_top_right;
	static sf::Vector3f boid_color_bot_left;
	static sf::Vector3f boid_color_bot_right;

	static float weight_sep;
	static float weight_ali;
	static float weight_coh;

	static bool cursor_enabled;
	static float cursor_towards;
	static float cursor_away;

	static bool turn_at_border;
	static float turn_margin_factor;
	static float turn_factor;

	static int grid_cell_max_boids;
	static int grid_extra_cells;

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
				boid_max_steer = json["settings"]["boid_max_steer"];
				boid_view_angle = json["settings"]["boid_view_angle"];
				boid_min_distance = json["settings"]["boid_min_distance"];

				boid_color_top_left = convert(json["settings"]["boid_color_top_left"]);
				boid_color_top_right = convert(json["settings"]["boid_color_top_right"]);
				boid_color_bot_left = convert(json["settings"]["boid_color_bot_left"]);
				boid_color_bot_right = convert(json["settings"]["boid_color_bot_right"]);

				weight_sep = json["settings"]["separation"];
				weight_ali = json["settings"]["alignment"];
				weight_coh = json["settings"]["cohesion"];

				cursor_enabled = json["settings"]["cursor_enabled"];
				cursor_towards = json["settings"]["cursor_towards"];
				cursor_away = json["settings"]["cursor_away"];

				turn_at_border = json["settings"]["turn_at_border"];
				turn_margin_factor = json["settings"]["turn_margin_factor"];
				turn_factor = json["settings"]["turn_factor"];

				grid_cell_max_boids = json["settings"]["grid_cell_max_boids"];

				vertical_sync = json["settings"]["vertical_sync"];
				max_framerate = json["settings"]["max_framerate"];
			}
			catch (std::exception e)
			{
				onError();
			}
		}
		else
			onError();
	}

	static inline sf::Vector3f convert(std::string strColor)
	{
		std::stringstream stream(strColor);
		std::string segment;
		std::vector<std::string> values;

		while (std::getline(stream, segment, ' '))
		{
			values.push_back(segment);
		}

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

private:
	static void onError()
	{
		background = sf::Vector3f(0.0f, 0.0f, 0.0f);

		boid_count = 2500;

		boid_size_width = 20.0f;
		boid_size_height = 10.0f;
		boid_max_speed = 340.0f;
		boid_max_steer = 3.0f;
		boid_view_angle = 280.0f;
		boid_min_distance = 52;

		boid_color_top_left = sf::Vector3f(0.73f, 0.33f, 0.82f);
		boid_color_top_right = sf::Vector3f(1.0f, 0.0f, 1.0f);
		boid_color_bot_left = sf::Vector3f(0.85f, 0.75f, 0.85f);
		boid_color_bot_right = sf::Vector3f(0.35f, 0.0f, 0.35f);

		weight_sep = 2.25f;
		weight_ali = 1.20f;
		weight_coh = 1.60f;

		cursor_enabled = true;
		cursor_towards = 1.2f;
		cursor_away = 1.2f;

		turn_at_border = true;
		turn_margin_factor = 0.85f;
		turn_factor = 120.0f;

		grid_cell_max_boids = 8;
		grid_extra_cells = 24;

		vertical_sync = true;
		max_framerate = 144;
	}
};
