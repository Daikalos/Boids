#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <random>
#include <chrono>

namespace util
{
	template<typename T>
	static inline T to_radians(T degrees)
	{
		return T(degrees * (float(M_PI) / 180.0f));
	}

	template<typename T>
	static inline T to_degrees(T radians)
	{
		return T(radians * (180.0f / float(M_PI)));
	}

	template<typename T>
	static inline T clamp(T val, T min, T max)
	{
		if (val < min)
			return min;
		if (val > max)
			return max;

		return val;
	}

	template<typename T>
	static inline T wrap(T val, T min, T max)
	{
		if (val > min && val < max)
			return val;

		return (val < min) ? 
			max - std::fmodf(min - val, max - min) : 
			min + std::fmodf(val - min, max - min);
	}

	template<typename T>
	static inline T map_to_range(T val, T minIn, T maxIn, T minOut, T maxOut)
	{
		float x = (val - minIn) / (maxIn - minIn);
		return minOut + (maxOut - minOut) * x;
	}

	template<typename T>
	static inline T interpolate(T a, T b, T c, T d, T t, T s)
	{
		return T(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

	template<typename T>
	static inline T set_precision(T val, const int& places)
	{
		float n = std::powf(10.0f, places);
		return std::roundf(val * n) / n;
	}

	static inline std::string remove_trailing_zeroes(const std::string& str)
	{
		std::string result = str;

		if (result.find('.') != std::string::npos)
		{
			result = result.substr(0, result.find_last_not_of('0') + 1);

			if (result.find('.') == result.size() - 1)
				result = result.substr(0, result.size() - 1);
		}

		return result;
	}

	static thread_local std::mt19937_64 dre(std::chrono::steady_clock::now().time_since_epoch().count());

	template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
	static inline T random(T min, T max)
	{
		std::uniform_real_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
	template<typename T, typename std::enable_if_t<!std::is_floating_point_v<T>>* = nullptr>
	static inline T random(T min, T max)
	{
		std::uniform_int_distribution<T> uid(min, max);
		return (T)uid(dre);
	}

	template<typename T, typename std::enable_if_t<std::is_integral_v<T>>* = nullptr>
	static inline std::vector<T> random(T size)
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
