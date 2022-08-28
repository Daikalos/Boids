#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <random>
#include <chrono>
#include <string_view>
#include <concepts>

namespace util
{
	template<typename T>
	static constexpr T to_radians(const T degrees)
	{
		return T(degrees * (float(M_PI) / 180.0f));
	}

	template<typename T>
	static constexpr T to_degrees(const T radians)
	{
		return T(radians * (180.0f / float(M_PI)));
	}

	template<std::integral T>
	static constexpr auto wrap(T val, const T min, const T max) noexcept
	{
		if (val > min && val < max)
			return val;

		const auto range_size = max - min;

		if (val < min)
			return max - ((min - val) % range_size);

		return min + ((val - min) % range_size);
	}
	template<std::floating_point T>
	static constexpr auto wrap(T val, const T min, const T max)
	{
		if (val > min && val < max)
			return val;

		const auto range_size = max - min;

		if (val < min)
			return max - std::fmod(min - val, max - min);

		return min + std::fmod(val - min, max - min);
	}

	template<typename T>
	static constexpr T map_to_range(const T val, const T minIn, const T maxIn, const T minOut, const T maxOut)
	{
		const float x = (val - minIn) / (maxIn - minIn);
		return minOut + (maxOut - minOut) * x;
	}

	template<typename T>
	static constexpr T interpolate(T a, T b, T c, T d, T t, T s)
	{
		return T(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

	template<typename T>
	static constexpr T set_precision(const T val, const int places)
	{
		const float n = std::powf(10.0f, (float)places);
		return std::roundf(val * n) / n;
	}

	static constexpr std::string remove_trailing_zeroes(const std::string_view str)
	{
		std::string result = std::string(str.data());

		if (result.find('.') != std::string::npos)
		{
			result = result.substr(0, result.find_last_not_of('0') + 1);

			if (result.find('.') == result.size() - 1)
				result = result.substr(0, result.size() - 1);
		}

		return result;
	}

	static thread_local std::mt19937_64 dre(std::chrono::steady_clock::now().time_since_epoch().count());

	template<std::floating_point T>
	static constexpr T random(const T min, const T max)
	{
		std::uniform_real_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
	template<std::integral T>
	static constexpr T random(const T min, const T max)
	{
		std::uniform_int_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
}
