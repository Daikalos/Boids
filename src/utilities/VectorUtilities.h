#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>

#include "Utilities.h"

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

namespace vu
{
	template<std::floating_point T = float>
	inline constexpr T PI = static_cast<T>(M_PI);

	template<std::floating_point T = float>
	inline constexpr T PI_2 = static_cast<T>(M_PI_2);

	template<Arithmetic T>
	static constexpr sf::Vector2<T> direction(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return (to - from);
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
		return direction(from, to).length();
	}
	template<Arithmetic T>
	static constexpr float distance_sq(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return direction(from, to).lengthSq();
	}
	template<Arithmetic T>
	static constexpr float distance_opt(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return distance_opt(direction(from, to));
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> normalize(sf::Vector2<T> vector, float length, float radius)
	{
		if (length < FLT_EPSILON || std::abs(length - radius) < FLT_EPSILON)
			return vector;

		float inv = (radius / length);

		vector.x *= inv;
		vector.y *= inv;

		return vector;
	}
	template<Arithmetic T>
	static constexpr sf::Vector2<T> normalize(const sf::Vector2<T>& vector, float radius = 1.0f)
	{
		return normalize(vector, vector.length(), radius);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> limit(const sf::Vector2<T>& vector, float length, float max_length)
	{
		if (length > max_length)
			return normalize(vector, length, max_length);

		return vector;
	}
	template<Arithmetic T>
	static constexpr sf::Vector2<T> limit(const sf::Vector2<T>& vector, float max_length)
	{
		return limit(vector, vector.length(), max_length);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> rotate_point(const sf::Vector2<T>& point, const sf::Vector2<T>& center, float angle)
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
	static constexpr auto lerp(const T a, const T b, float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}

	template<Arithmetic T>
	static constexpr sf::Vector2<T> lerp(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs, float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a));
	}
	template<Arithmetic T>
	static constexpr sf::Vector3<T> lerp(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs, float a)
	{
		return sf::Vector3<T>(
			lerp(lhs.x, rhs.x, a),
			lerp(lhs.y, rhs.y, a),
			lerp(lhs.z, rhs.z, a));
	}

	inline float atan_approximation(float x)
	{
		const float a1 = 0.99997726f;
		const float a3 = -0.33262347f;
		const float a5 = 0.19354346f;
		const float a7 = -0.11643287f;
		const float a9 = 0.05265332f;
		const float a11 = -0.01172120f;

		const float x_sq = x * x;
		return x * fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, a11, a9), a7), a5), a3), a1);
	}

	static float angle(float y, float x)
	{
		const bool swap = std::fabs(x) < std::fabs(y);
		const float atan_input = (swap ? x / y : y / x);

		float res = atan_approximation(atan_input);

		res = swap ? std::copysignf(PI_2<>, atan_input) - res : res;

		if (x < 0.0f)
			res = std::copysignf(PI<>, y) + res;

		return res;
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

