#pragma once

#include <SFML/Graphics.hpp>

#include "../window/ResourceHolder.hpp"

#include "Config.h"

class Background
{
public:
	Background() = default;

	void load_texture(const TextureHolder& _texture_holder)
	{
		if (_texture_holder.exists(TextureID::Background))
			_background.setTexture(_texture_holder.get(TextureID::Background), true);
		else
			_background.setTexture(sf::Texture(), true);
	}
	void load_prop(Config& config, const sf::Vector2i& size)
	{
		sf::Vector2f desired_scale = sf::Vector2f(1.0f, 1.0f);

		if (config.background_fit_screen)
			desired_scale = sf::Vector2f(
				size.x / _background.getLocalBounds().width,
				size.y / _background.getLocalBounds().height);
		else if (config.background_override_size)
			desired_scale = sf::Vector2f(
				config.background_width / _background.getLocalBounds().width,
				config.background_height / _background.getLocalBounds().height);

		bool set_color = config.background_color.x > FLT_EPSILON || config.background_color.y > FLT_EPSILON || config.background_color.z > FLT_EPSILON;

		_background.setPosition(sf::Vector2f(
			config.background_position_x,
			config.background_position_y));

		_background.setScale(desired_scale);

		_background.setColor(set_color ? sf::Color(
			config.background_color.x * 255,
			config.background_color.y * 255,
			config.background_color.z * 255) : sf::Color(255, 255, 255));
	}

	void draw(sf::RenderWindow& window) const
	{
		if (!_background.getTexture()->getMaximumSize())
			return;

		window.pushGLStates();
		window.draw(_background);
		window.popGLStates();
	}

private:
	sf::Sprite _background;
};