#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "ResourceHolder.hpp"

class Background
{
public:
	Background();

public:
	void Load(TextureHolder& textureHolder, const sf::Vector2i& size);
	void LoadProperties(const sf::Vector2i& size);

public:
	void Draw(sf::RenderWindow& window) const;

private:
	void LoadTexture(const TextureHolder& textureHolder);

private:
	sf::Sprite m_background;
	std::future<void> m_loadAsync;
};