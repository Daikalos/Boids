#pragma once

#include <SFML/Graphics.hpp>

#include "../window/ResourceHolder.hpp"

#include "Config.h"

class Background
{
public:
	Background() = default;

public:
	void LoadTexture(const TextureHolder& textureHolder);
	void LoadProperties(const sf::Vector2i& size);

public:
	void Draw(sf::RenderWindow& window) const;

private:
	sf::Sprite m_background;
};