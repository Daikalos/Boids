#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <SFML/Graphics.hpp>

#include "Utilities.h"

struct Vector2
{
public:
	static inline double Length(const sf::Vector2<double>& vector)
	{
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}
	static inline double Angle(const sf::Vector2<double>& vector)
	{
		return atan2(vector.y, vector.x);
	}
	static inline double Angle(const sf::Vector2<double> lhs, const sf::Vector2<double> rhs)
	{
		return acos(Dot(lhs, rhs) / (Length(lhs) * Length(rhs)));
	}
	static inline double Dot(const sf::Vector2<double> lhs, const sf::Vector2<double> rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	static inline double Distance(const sf::Vector2<double>& fromVector, const sf::Vector2<double>& toVector)
	{
		return Length(Direction(fromVector, toVector));
	}

	static inline sf::Vector2<double> Normalize(sf::Vector2<double> vector, double radius = 1.0)
	{
		if (Length(vector) == 0)
			return vector;

		double length = Length(vector);

		vector.x *= (radius / length);
		vector.y *= (radius / length);

		return vector;
	}
	static inline sf::Vector2<double> Limit(const sf::Vector2<double>& vector, double maxLength)
	{
		if (Length(vector) > maxLength)
		{
			return Normalize(vector, maxLength);
		}
		return vector;
	}
	static inline sf::Vector2<double> Direction(const sf::Vector2<double>& from, const sf::Vector2<double>& to)
	{
		return sf::Vector2<double>(from.x - to.x, from.y - to.y);
	}
	static inline sf::Vector2<double> RotatePoint(const sf::Vector2<double>& point, const sf::Vector2<double>& center, double angle)
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

