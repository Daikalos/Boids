#pragma once

#define _USE_MATH_DEFINES

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include "CommonUtilities.hpp"

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

namespace vu
{
	template<std::floating_point T = float>
	constexpr T PI = static_cast<T>(M_PI);

	template<std::floating_point T = float>
	constexpr T PI_2 = static_cast<T>(M_PI_2);

	template<Arithmetic T>
	constexpr sf::Vector2<T> Direction(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return (to - from);
	}

	template<Arithmetic T>
	inline float DistanceOpt(const sf::Vector2<T>& vector)
	{
		T dx = std::abs(vector.x);
		T dy = std::abs(vector.y);

		if (dy > dx)
			std::swap(dx, dy);

		return 1007.f / 1024.f * dx + 441.f / 1024.f * dy;
	}

	template<Arithmetic T>
	constexpr float Distance(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return Direction(from, to).length();
	}
	template<Arithmetic T>
	constexpr float DistanceSq(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return Direction(from, to).lengthSq();
	}
	template<Arithmetic T>
	inline float DistanceOpt(const sf::Vector2<T>& from, const sf::Vector2<T>& to)
	{
		return DistanceOpt(Direction(from, to));
	}

	template<Arithmetic T>
	inline sf::Vector2<T> Normalize(sf::Vector2<T> vector, float length, float radius)
	{
		if (length < FLT_EPSILON)
			return vector;

		const float inv = (radius / length);

		vector.x *= inv;
		vector.y *= inv;

		return vector;
	}
	template<Arithmetic T>
	inline sf::Vector2<T> Normalize(const sf::Vector2<T>& vector, float radius = 1.0f)
	{
		return Normalize(vector, vector.length(), radius);
	}

	template<Arithmetic T>
	inline sf::Vector2<T> Limit(const sf::Vector2<T>& vector, float length, float max_length)
	{
		if (length > max_length)
			return Normalize(vector, length, max_length);

		return vector;
	}
	template<Arithmetic T>
	inline sf::Vector2<T> Limit(const sf::Vector2<T>& vector, float max_length)
	{
		return Limit(vector, vector.length(), max_length);
	}

	template<Arithmetic T>
	inline sf::Vector2<T> RotatePoint(const sf::Vector2<T>& point, const sf::Vector2<T>& center, float angle)
	{
		const sf::Vector2<T> dir = Direction(center, point);

		float s = std::sinf(angle);
		float c = std::cosf(angle);

		return sf::Vector2<T>(
			(dir.x * c - dir.y * s) + center.x,
			(dir.x * s + dir.y * c) + center.y);
	}

	template<Arithmetic T>
	inline sf::Vector2<T> Abs(sf::Vector2<T> vector)
	{
		vector.x = std::abs(vector.x);
		vector.y = std::abs(vector.y);

		return vector;
	}

	template<Arithmetic T>
	inline sf::Vector2<T> Floor(sf::Vector2<T> vector)
	{
		vector.x = std::floorf(vector.x);
		vector.y = std::floorf(vector.y);

		return vector;
	}

	template<Arithmetic T>
	constexpr sf::Vector2<T> Lerp(const sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs, float a)
	{
		return sf::Vector2<T>(
			util::Lerp(lhs.x, rhs.x, a),
			util::Lerp(lhs.y, rhs.y, a));
	}
	template<Arithmetic T>
	constexpr sf::Vector3<T> Lerp(const sf::Vector3<T>& lhs, const sf::Vector3<T>& rhs, float a)
	{
		return sf::Vector3<T>(
			util::Lerp(lhs.x, rhs.x, a),
			util::Lerp(lhs.y, rhs.y, a),
			util::Lerp(lhs.z, rhs.z, a));
	}

	__forceinline float AtanApproximation(float x)
	{
		const float a1 = 0.99997726f;
		const float a3 = -0.33262347f;
		const float a5 = 0.19354346f;
		const float a7 = -0.11643287f;
		const float a9 = 0.05265332f;
		const float a11 = -0.01172120f;

		const float xSq = x * x;

		return x * fmaf(xSq, fmaf(xSq, fmaf(xSq, fmaf(xSq, fmaf(xSq, a11, a9), a7), a5), a3), a1);
	}

	__forceinline float Angle(float y, float x)
	{
		const bool swap = std::fabs(x) < std::fabs(y);
		const float atan_input = (swap ? x / y : y / x);

		float res = AtanApproximation(atan_input);

		res = swap ? std::copysignf(PI_2<>, atan_input) - res : res;

		if (x < 0.0f)
			res = std::copysignf(PI<>, y) + res;

		return res;
	}
}

// operator overloads that SFML is missing
// makes certain assumptions on how these operands affect each other

template <Arithmetic T>
constexpr sf::Vector2<T>& operator /=(sf::Vector2<T>& lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}
template <Arithmetic T>
constexpr sf::Vector2<T> operator/(float lhs, const sf::Vector2<T>& rhs)
{
	sf::Vector2<T> result = { lhs, lhs };
	result.x /= rhs.x;
	result.y /= rhs.y;
	return result;
}
template <Arithmetic T>
constexpr sf::Vector2<T> operator/(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	return lhs;
}

template <Arithmetic T>
constexpr sf::Vector2<T> operator*(sf::Vector2<T> lhs, const sf::Vector2<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	return lhs;
}

template <Arithmetic T>
constexpr sf::Vector3<T> operator*(sf::Vector3<T> lhs, const sf::Vector3<T>& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

