#pragma once

#include <SFML/Graphics.hpp>

#include "ResourceHolder.hpp"

class Background
{
public:
	Background() { }
	~Background() { }

	void load_texture(const TextureHolder& texture_holder)
	{
		if (texture_holder.exists(TextureID::Background))
			background.setTexture(texture_holder.get(TextureID::Background), true);
		else
			background.setTexture(sf::Texture(), true);
	}
	void load_prop(Config& config, const sf::VideoMode& video_mode)
	{
		background.setPosition(sf::Vector2f(
			config.background_position_x,
			config.background_position_y));

		sf::Vector2f desired_scale = sf::Vector2f(1.0f, 1.0f);

		if (config.background_fit_screen)
			desired_scale = sf::Vector2f(
				video_mode.size.x / background.getLocalBounds().width,
				video_mode.size.y / background.getLocalBounds().height);
		else if (config.background_override_size)
			desired_scale = sf::Vector2f(
				config.background_width / background.getLocalBounds().width,
				config.background_height / background.getLocalBounds().height);

		background.setScale(desired_scale);

		if (config.background_color.x > FLT_EPSILON || config.background_color.y > FLT_EPSILON || config.background_color.z > FLT_EPSILON)
			background.setColor(sf::Color(
				config.background_color.x * 255,
				config.background_color.y * 255,
				config.background_color.z * 255));
	}

	void draw(sf::RenderWindow& window) const
	{
		if (!background.getTexture()->getMaximumSize())
			return;

		window.pushGLStates();
		window.draw(background);
		window.popGLStates();
	}

private:
	sf::Sprite background;
};