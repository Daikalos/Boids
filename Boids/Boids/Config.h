#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include "Rectangle.h"

struct Config
{
	sf::Vector3f background = sf::Vector3f(0.0f, 0.0f, 0.0f);

	size_t boid_count = 3500;

	float boid_size_width = 12.0f;
	float boid_size_height = 6.0f;
	float boid_max_speed = 300.0f;
	float boid_max_steer = 3.0f;
	float boid_view_angle = 260.0f;
	float boid_min_distance = 40;

	float weight_sep = 2.20f;
	float weight_ali = 1.05f;
	float weight_coh = 1.55f;

	bool cursor_enabled = true;
	float cursor_towards = 1.5f;
	float cursor_away = 1.5f;

	bool turn_at_border = true;
	float turn_margin_factor = 0.85f;
	float turn_factor = 120.0f;

	sf::Vector3f boid_color_top_left = sf::Vector3f(0.73f, 0.33f, 0.82f);
	sf::Vector3f boid_color_top_right = sf::Vector3f(1.0f, 0.0f, 1.0f);
	sf::Vector3f boid_color_bot_left = sf::Vector3f(0.85f, 0.75f, 0.85f);
	sf::Vector3f boid_color_bot_right = sf::Vector3f(0.35f, 0.0f, 0.35f);

	bool vertical_sync = true;
	int max_framerate = 144;

	void load()
	{
		std::ifstream project_file("settings.json", std::ifstream::binary);
		std::stringstream buffer;

		buffer << project_file.rdbuf();

		nlohmann::json json;
		if (!buffer.str().empty())
			json = nlohmann::json::parse(buffer.str());

		if (!json.empty())
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

			vertical_sync = json["settings"]["vertical_sync"];
			max_framerate = json["settings"]["max_framerate"];
		}
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
};
