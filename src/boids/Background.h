#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "../window/ResourceHolder.hpp"

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