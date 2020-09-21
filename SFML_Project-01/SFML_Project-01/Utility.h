#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <SFML/Graphics.hpp>

static inline double Length(const sf::Vector2<double>& vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

static inline double ToRadians(double degrees)
{
	return degrees * ((double)M_PI / 180.0f);
}

static inline double ToDegrees(double radians)
{
	return radians * (180.0f / (double)M_PI);
}

static inline double Clamp(double val, const double min, const double max)
{
	val = (val < min) ? min : val;
	val = (val > max) ? max : val;
	return val;
}

static inline double Angle(const sf::Vector2<double>& vector)
{
	return atan2(vector.y, vector.x);
}

static inline sf::Vector2<double> Normalize(sf::Vector2<double> vector, double radius)
{
	if (Length(vector) == 0) return vector;

	double length = Length(vector);

	vector.x *= (radius / length);
	vector.y *= (radius / length);

	return vector;
}

static inline sf::Vector2<double> Limit(const sf::Vector2<double>& vector, double maxForce)
{
	if (Length(vector) > maxForce)
	{
		return Normalize(vector, maxForce);
	}
	return vector;
}

static inline sf::Vector2<double> Direction(const sf::Vector2<double>& fromVector, const sf::Vector2<double>& toVector)
{
	return sf::Vector2<double>(fromVector.x - toVector.x, fromVector.y - toVector.y);
}

static inline double Distance(const sf::Vector2<double>& fromVector, const sf::Vector2<double>& toVector)
{
	return Length(Direction(fromVector, toVector));
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
