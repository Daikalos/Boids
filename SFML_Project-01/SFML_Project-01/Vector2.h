#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <SFML/Graphics.hpp>

#include "Utilities.h"

struct Vector2
{
public:
	static inline double length(const sf::Vector2<double>& vector)
	{
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}
	static inline double angle(const sf::Vector2<double>& vector)
	{
		return atan2(vector.y, vector.x);
	}
	static inline double angle(const sf::Vector2<double> lhs, const sf::Vector2<double> rhs)
	{
		return acos(dot(lhs, rhs) / (length(lhs) * length(rhs)));
	}
	static inline double dot(const sf::Vector2<double> lhs, const sf::Vector2<double> rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	static inline double distance(const sf::Vector2<double>& fromVector, const sf::Vector2<double>& toVector)
	{
		return length(direction(fromVector, toVector));
	}

	static inline sf::Vector2<double> normalize(sf::Vector2<double> vector, double radius = 1.0)
	{
		double len = length(vector);

		if (len == 0)
			return vector;

		vector.x *= (radius / len);
		vector.y *= (radius / len);

		return vector;
	}
	static inline sf::Vector2<double> limit(const sf::Vector2<double>& vector, double maxLength)
	{
		if (length(vector) > maxLength)
		{
			return normalize(vector, maxLength);
		}
		return vector;
	}
	static inline sf::Vector2<double> direction(const sf::Vector2<double>& from, const sf::Vector2<double>& to)
	{
		return sf::Vector2<double>(from.x - to.x, from.y - to.y);
	}
	static inline sf::Vector2<double> rotate_point(const sf::Vector2<double>& point, const sf::Vector2<double>& center, double angle)
	{
		sf::Vector2<double> newPoint = sf::Vector2<double>();

		double s = sin(angle);
		double c = cos(angle);

		newPoint.x = (((point.x - center.x) * c - (point.y - center.y) * s)) + center.x;
		newPoint.y = (((point.x - center.x) * s + (point.y - center.y) * c)) + center.y;

		return newPoint;
	}

private:
	Vector2() = delete;
};

static sf::Vector2<double> operator /=(sf::Vector2<double>& lhs, const sf::Vector2<double>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}

template <typename T>
static sf::Vector2<double> operator /=(sf::Vector2<double>& lhs, const T& rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

