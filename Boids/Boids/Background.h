#pragma once

#include <SFML/Graphics.hpp>

#include "ResourceManager.h"

class Background
{
public:
	Background() { }
	~Background() { }

	void load(const ResourceManager& resourceManager, const sf::VideoMode& video_mode)
	{
		sf::Texture* background_texture = resourceManager.request_texture("background");
		if (background_texture != nullptr)
			background.setTexture(*background_texture);

		background.setPosition(sf::Vector2f(
			Config::background_position_x,
			Config::background_position_y));

		sf::Vector2f desired_scale = sf::Vector2f(1.0f, 1.0f);

		if (Config::background_fit_screen)
			desired_scale = sf::Vector2f(
				video_mode.size.x / background.getLocalBounds().width,
				video_mode.size.y / background.getLocalBounds().height);
		else if (Config::background_override_size)
			desired_scale = sf::Vector2f(
				Config::background_width / background.getLocalBounds().width,
				Config::background_height / background.getLocalBounds().height);

		background.setScale(desired_scale);

		if (Config::background_color.x > FLT_EPSILON || Config::background_color.y > FLT_EPSILON || Config::background_color.z > FLT_EPSILON)
			background.setColor(sf::Color(
				Config::background_color.x * 255,
				Config::background_color.y * 255,
				Config::background_color.z * 255));
	}

	void draw(sf::RenderWindow& renderWindow)
	{
		renderWindow.draw(background);
	}

private:
	sf::Sprite background;
};