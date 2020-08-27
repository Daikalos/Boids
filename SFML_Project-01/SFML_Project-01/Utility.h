#pragma once

#include <SFML/Graphics.hpp>

static inline float Length(const sf::Vector2f& vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

static inline float Clamp(float val, const float min, const float max)
{
	val = (val < min) ? min : val;
	val = (val > max) ? max : val;
	return val;
}

static inline sf::Vector2f Normalize(sf::Vector2f vector)
{
	if (Length(vector) == 0) return vector;

	float length = Length(vector);

	vector.x *= (1.0f / length);
	vector.y *= (1.0f / length);

	return vector;
}

static inline sf::Vector2f Direction(const sf::Vector2f& fromVector, const sf::Vector2f& toVector)
{
	return sf::Vector2f(fromVector.x - toVector.x, fromVector.y - toVector.y);
}

static inline sf::Vector2f CrossProduct(const sf::Vector2f& vector, float scalar)
{
	return sf::Vector2f(scalar * vector.y, -scalar * vector.x);
}

static inline sf::Vector2f CrossProduct(float scalar, const sf::Vector2f& vector)
{
	return sf::Vector2f(-scalar * vector.y, scalar * vector.x);
}

static inline float CrossProduct(const sf::Vector2f& vectorA, const sf::Vector2f& vectorB)
{
	return vectorA.x * vectorB.y - vectorA.y * vectorB.x;
}
