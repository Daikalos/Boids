#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
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

static inline float Angle(const sf::Vector2f& vector)
{
	return atan2(vector.y, vector.x);
}

static inline float ToRadians(float degrees)
{
	return degrees * ((float)M_PI / 180.0f);
}

static inline float ToDegrees(float radians)
{
	return radians * (180.0f / (float)M_PI);
}

static inline sf::Vector2f Limit(const sf::Vector2f& vector, const float maxX, const float maxY)
{
	return sf::Vector2f(Clamp(vector.x, vector.x, maxX), Clamp(vector.y, vector.y, maxY));
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

static inline sf::Vector2f RotatePoint(const sf::Vector2f& point, const sf::Vector2f& center, float angle)
{
	sf::Vector2f newPoint = sf::Vector2f();

	float s = sin(angle);
	float c = cos(angle);

	newPoint.x = (((point.x - center.x) * c - (point.y - center.y) * s)) + center.x;
	newPoint.y = (((point.x - center.x) * s + (point.y - center.y) * c)) + center.y;

	return newPoint;
}
