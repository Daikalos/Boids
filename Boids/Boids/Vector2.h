#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>

struct Vector2
{
public:
	static inline float length(const sf::Vector2f& vector)
	{
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}
	static inline float angle(const sf::Vector2f& vector)
	{
		return atan2(vector.y, vector.x);
	}
	static inline float angle(const sf::Vector2f lhs, const sf::Vector2f rhs)
	{
		return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
	}
	static inline float dot(const sf::Vector2f lhs, const sf::Vector2f rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	static inline float distance(const sf::Vector2f& from, const sf::Vector2f& to)
	{
		return length(direction(from, to));
	}

	static inline sf::Vector2f normalize(sf::Vector2f vector, float radius = 1.0)
	{
		float len = length(vector);

		if (len == 0)
			return vector;

		vector.x *= (radius / len);
		vector.y *= (radius / len);

		return vector;
	}
	static inline sf::Vector2f limit(const sf::Vector2f& vector, float maxLength)
	{
		if (length(vector) > maxLength)
		{
			return normalize(vector, maxLength);
		}
		return vector;
	}
	static inline sf::Vector2f direction(const sf::Vector2f& from, const sf::Vector2f& to)
	{
		return sf::Vector2f(from.x - to.x, from.y - to.y);
	}
	static inline sf::Vector2f rotate_point(const sf::Vector2f& point, const sf::Vector2f& center, float angle)
	{
		sf::Vector2f newPoint = sf::Vector2f();

		float s = sin(angle);
		float c = cos(angle);

		newPoint.x = (((point.x - center.x) * c - (point.y - center.y) * s)) + center.x;
		newPoint.y = (((point.x - center.x) * s + (point.y - center.y) * c)) + center.y;

		return newPoint;
	}

private:
	Vector2() = delete;
};

static sf::Vector2f operator /=(sf::Vector2f& lhs, const sf::Vector2f& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}

template <typename T>
static sf::Vector2f operator /=(sf::Vector2f& lhs, const T& rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

