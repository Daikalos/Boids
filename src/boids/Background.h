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

	void load_prop(const sf::Vector2i& size)
	{
		sf::Vector2f desired_scale = sf::Vector2f(1.0f, 1.0f);

		if (Config::GetInstance().background_fit_screen)
			desired_scale = sf::Vector2f(
				size.x / _background.getLocalBounds().width,
				size.y / _background.getLocalBounds().height);
		else if (Config::GetInstance().background_override_size)
			desired_scale = sf::Vector2f(
				Config::GetInstance().background_width / _background.getLocalBounds().width,
				Config::GetInstance().background_height / _background.getLocalBounds().height);

		bool set_color = Config::GetInstance().background_color.x > FLT_EPSILON || Config::GetInstance().background_color.y > FLT_EPSILON || Config::GetInstance().background_color.z > FLT_EPSILON;

		_background.setPosition(sf::Vector2f(
			(float)Config::GetInstance().background_position_x,
			(float)Config::GetInstance().background_position_y));

		_background.setScale(desired_scale);

		_background.setColor(set_color ? sf::Color(
			(sf::Uint8)(Config::GetInstance().background_color.x * 255.0f),
			(sf::Uint8)(Config::GetInstance().background_color.y * 255.0f),
			(sf::Uint8)(Config::GetInstance().background_color.z * 255.0f)) : sf::Color(UINT8_MAX, UINT8_MAX, UINT8_MAX));
	}

	void draw(sf::RenderWindow& window) const
	{
		if (_background.getTexture()->getSize().x == 0U ||
			_background.getTexture()->getSize().y == 0U)
			return;

		window.draw(_background);
	}

private:
	sf::Sprite _background;
};