#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>

#include "Utilities.h"

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

namespace vu
{
	template<Arithmetic T>
	static constexpr sf::Vector2<T> direction(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return sf::Vector2<T>(to.x - from.x, to.y - from.y);
	}

	template<Arithmetic T>
	static constexpr float dot(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	template<Arithmetic T>
	static constexpr float cross(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	template<Arithmetic T>
	static constexpr float distance(const sf::Vector2<T>& vector)
	{
		return std::sqrtf(vector.x * vector.x + vector.y * vector.y);
	}
	template<Arithmetic T>
	static constexpr float distance_sq(const sf::Vector2<T>& vector)
	{
		return vector.x * vector.x + vector.y * vector.y;
	}
	template<Arithmetic T>
	static constexpr float distance_opt(const sf::Vector2<T>& vector)
	{
		T dx = std::fabsf(vector.x);
		T dy = std::fabsf(vector.y);

		if (dy > dx)
			std::swap(dx, dy);

		return 1007.f / 1024.f * dx + 441.f / 1024.f * dy;
	}

	template<Arithmetic T>
	static constexpr float distance(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return distance(direction(from, to));
	}
	template<Arithmetic T>
	static constexpr float distance_sq(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return distance_sq(direction(from, to));
	}
	template<Arithmetic T>
	static constexpr float distance_opt(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return distance_opt(direction(from, to));
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> normalize(sf::Vector2<T> vector, const float length, const float radius)
	{
		if (length < FLT_EPSILON || std::abs(length - radius) < FLT_EPSILON)
			return vector;

		const float inv = (radius / length);

		vector.x *= inv;
		vector.y *= inv;

		return vector;
	}
	template<Arithmetic T>
	static constexpr sf::Vector2<T> normalize(const sf::Vector2<T>& vector, const float radius = 1.0f)
	{
		return normalize(vector, distance(vector), radius);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> limit(const sf::Vector2<T>& vector, const float length, const float max_length)
	{
		if (length > max_length)
			return normalize(vector, length, max_length);

		return vector;
	}
	template<Arithmetic T>
	static constexpr sf::Vector2<T> limit(const sf::Vector2<T>& vector, const float max_length)
	{
		return limit(vector, distance(vector), max_length);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> clamp(const sf::Vector2<T>& vector, const float length, const float min_length, const float max_length)
	{
		if (length < min_length)
			return normalize(vector, length, min_length);
		if (length > max_length)
			return normalize(vector, length, max_length);

		return vector;
	}
	template<Arithmetic T>
	static constexpr sf::Vector2<T> clamp(const sf::Vector2<T>& vector, const float min_length, const float max_length)
	{
		return clamp(vector, distance(vector), min_length, max_length);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> rotate_point(const sf::Vector2<T>& point, const sf::Vector2<T>& center, const float angle)
	{
		const sf::Vector2<T> dir = direction(center, point);

		float s = std::sinf(angle);
		float c = std::cosf(angle);

		return sf::Vector2<T>(
			(dir.x * c - dir.y * s) + center.x,
			(dir.x * s + dir.y * c) + center.y);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> abs(sf::Vector2<T> vector)
	{
		vector.x = std::abs(vector.x);
		vector.y = std::abs(vector.y);

		return vector;
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> floor(sf::Vector2<T> vector)
	{
		vector.x = std::floorf(vector.x);
		vector.y = std::floorf(vector.y);

		return vector;
	}

	template<Arithmetic T>
	static constexpr auto lerp(const T a, const T b, const float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> lerp(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs, const float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a));
	}
	template<Arithmetic T>
	static constexpr sf::Vector3<T> lerp(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs, const float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a),
			lerp(lhs.z, rhs.z, a));
	}
}

// operator overloads that SFML is missing

template <Arithmetic T>
static constexpr sf::Vector2<T>& operator /=(sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}
template <Arithmetic T>
static constexpr sf::Vector2<T> operator/(float lhs, const sf::Vector2<T>& rhs)
{
	sf::Vector2<T> result = { lhs, lhs };
	result.x /= rhs.x;
	result.y /= rhs.y;
	return result;
}
template <Arithmetic T>
static constexpr sf::Vector2<T> operator/(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}

template <Arithmetic T>
static constexpr sf::Vector2<T> operator*(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	return lhs;
}

template <Arithmetic T>
static constexpr sf::Vector3<T> operator*(sf::Vector3<T> lhs, const sf::Vector3<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

