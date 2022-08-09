#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <random>
#include <chrono>

namespace util
{
	template<typename T>
	static inline T to_radians(const T& degrees)
	{
		return (T)(degrees * ((double)M_PI / 180.0));
	}

	template<typename T>
	static inline T to_degrees(const T& radians)
	{
		return (T)(radians * (180.0 / (double)M_PI));
	}

	template<typename T>
	static inline T clamp(const T& val, const T& min, const T& max)
	{
		if (val < min)
			return min;
		if (val > max)
			return max;

		return val;
	}

	template<typename T>
	static inline T map_to_range(const T& val, const T& minIn, const T& maxIn, const T& minOut, const T& maxOut)
	{
		float x = (val - minIn) / (maxIn - minIn);
		return minOut + (maxOut - minOut) * x;
	}

	template<typename T>
	static inline T interpolate(const T& a, const T& b, const T& c, const T& d, const T& t, const T& s)
	{
		return T(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

	static thread_local std::mt19937_64 dre(std::chrono::steady_clock::now().time_since_epoch().count());

	template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
	static inline T random(const T& min, const T& max)
	{
		std::uniform_real_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
	template<typename T, typename std::enable_if_t<!std::is_floating_point_v<T>>* = nullptr>
	static inline T random(const T& min, const T& max)
	{
		std::uniform_int_distribution<T> uid(min, max);
		return (T)uid(dre);
	}

	template<typename T, typename std::enable_if_t<std::is_integral_v<T>>* = nullptr>
	static inline std::vector<T> random(const T& size)
	{
		std::vector<T> result;
		result.reserve(size);

		for (T i = 0; i < size; ++i)
			result.push_back(i);

		std::shuffle(result.begin(), result.end(), dre);

		return result;
	}

	template<typename T, typename... Args, typename std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr>
	static inline T random_arg(const Args&... args)
	{
		std::vector<T> x{ { args... } };
		return x[util::random<size_t>(0, x.size() - 1)];
	}
}
