#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <random>
#include <string_view>
#include <concepts>

namespace util
{
	template<typename T>
	constexpr T ToRadians(T degrees)
	{
		return static_cast<T>(degrees * (float(M_PI) / 180.0f));
	}

	template<typename T>
	constexpr T ToDegrees(T radians)
	{
		return static_cast<T>(radians * (180.0f / float(M_PI)));
	}

	template<std::integral T>
	constexpr auto Wrap(T val, T min, T max) noexcept
	{
		if (val > min && val < max)
			return val;

		const auto range_size = max - min;

		if (val < min)
			return max - ((min - val) % range_size);

		return min + ((val - min) % range_size);
	}
	template<std::floating_point T>
	inline auto Wrap(T val, T min, T max)
	{
		if (val > min && val < max)
			return val;

		const auto range_size = max - min;

		if (val < min)
			return max - std::fmod(min - val, range_size);

		return min + std::fmod(val - min, range_size);
	}

	template<typename T>
	constexpr T Pow(T base, int exponent)
	{
		if (exponent < 0)
			return Pow(1 / base, -exponent);

		if (exponent == 0)
			return 1;

		if (exponent % 2 == 0)
			return Pow(base, exponent / 2) * Pow(base, exponent / 2);

		return base * Pow(base, (exponent - 1) / 2) * Pow(base, (exponent - 1) / 2);
	}

	template<typename T>
	constexpr T MapToRange(T val, T minIn, T maxIn, T minOut, T maxOut)
	{
		const float x = (val - minIn) / (maxIn - minIn);
		return minOut + (maxOut - minOut) * x;
	}

	template<typename T>
	constexpr T Interpolate(T a, T b, T c, T d, T t, T s)
	{
		return T(a * (1 - t) * (1 - s) + b * t * (1 - s) + c * (1 - t) * s + d * t * s);
	}

	template<typename T>
	inline T SetPrecision(T val, int places)
	{
		const int n = Pow(10, places);
		return std::roundf(val * n) / n;
	}

	template<typename T>
	constexpr auto Lerp(T a, T b, float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}

	constexpr std::string RemoveTrailingZeroes(const std::string_view str)
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

	static thread_local std::mt19937_64 dre(std::random_device{}());

	template<std::floating_point T>
	inline T Random(T min, T max)
	{
		std::uniform_real_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
	template<std::integral T>
	inline T Random(T min, T max)
	{
		std::uniform_int_distribution<T> uid(min, max);
		return (T)uid(dre);
	}
}
