#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>

#include "Utilities.h"

template<typename T> struct v2
{
public:
	static inline const sf::Vector2<T> direction(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return sf::Vector2<T>(to.x - from.x, to.y - from.y);
	}

	static inline float length(const sf::Vector2<T>& vector)
	{
		return std::sqrtf(vector.x * vector.x + vector.y * vector.y);
	}
	static inline float length_sq(const sf::Vector2<T>& vector)
	{
		return vector.x * vector.x + vector.y * vector.y;
	}
	static inline float length_opt(const sf::Vector2<T>& vector)
	{
		T dx = std::fabsf(vector.x);
		T dy = std::fabsf(vector.y);

		if (dy > dx)
			std::swap(dx, dy);

		return 1007.f / 1024.f * dx + 441.f / 1024.f * dy;
	}

	static inline float length(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return std::sqrtf(direction(from, to));
	}
	static inline float length_sq(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return length_sq(direction(from, to));
	}
	static inline float length_opt(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return length_opt(direction(from, to));
	}

	static inline float angle(const sf::Vector2<T>& vector)
	{
		return atan2f(vector.y, vector.x);
	}
	static inline float angle(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
	{
		return acosf(dot(lhs, rhs) / (length(lhs) * length(rhs)));
	}
	static inline float angle(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs, float lhs_length, float rhs_length)
	{
		return acosf(dot(lhs, rhs) / (lhs_length * rhs_length));
	}

	static inline float dot(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	static inline float cross(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	static inline sf::Vector2<T> normalize(sf::Vector2<T> vector, float radius = 1.0f)
	{
		float len = length(vector);

		if (len < FLT_EPSILON)
			return vector;

		vector.x *= (radius / len);
		vector.y *= (radius / len);

		return vector;
	}

	static inline sf::Vector2<T> max(const sf::Vector2<T>& vector, float max_length)
	{
		if (length(vector) > max_length)
			return normalize(vector, max_length);

		return vector;
	}

	static inline sf::Vector2<T> clamp(const sf::Vector2<T>& vector, float max_length, float min_length)
	{
		float l = length(vector);

		if (l > max_length)
			return normalize(vector, max_length);

		if (l < min_length)
			return normalize(vector, min_length);

		return vector;
	}

	static inline sf::Vector2<T> rotate_point(const sf::Vector2<T>& point, const sf::Vector2<T>& center, float angle)
	{
		sf::Vector2<T> dir = direction(center, point);

		float s = sinf(angle);
		float c = cosf(angle);

		return sf::Vector2<T>(
			(dir.x * c - dir.y * s) + center.x,
			(dir.x * s + dir.y * c) + center.y);
	}

	static inline float lerp(float a, float b, float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}
	static inline const sf::Vector2<T> lerp(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs, float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a));
	}
	static inline sf::Vector3<T> lerp(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs, float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a),
			lerp(lhs.z, rhs.z, a));
	}

private:
	v2() = delete;
};

typedef v2<int> v2i;
typedef v2<float> v2f;
typedef v2<double> v2d;

template <typename T> 
static inline sf::Vector2<T> operator /=(sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}
template <typename T>
static inline sf::Vector2<T> operator /(float lhs, const sf::Vector2<T>& rhs)
{
	sf::Vector2<T> result = { lhs, lhs };
	result.x /= rhs.x;
	result.y /= rhs.y;
	return result;
}
template <typename T>
static inline sf::Vector2<T> operator /(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}

template <typename T>
static inline sf::Vector2<T> operator *(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	return lhs;
}

template <typename T>
static inline sf::Vector3<T> operator *(sf::Vector3<T> lhs, const sf::Vector3<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

