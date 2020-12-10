#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <SFML/Graphics.hpp>

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

static inline double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}
