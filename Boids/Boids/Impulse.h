#pragma once

#include <SFML/Graphics.hpp>

struct Impulse
{
	Impulse(sf::Vector2f position, float length)
		: position(position), length(length)
	{

	}

	sf::Vector2f position{0.0f, 0.0f};
	float length{0.0f};
};
