#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <future>
#include <thread>
#include <execution>
#include <memory>
#include <algorithm>

#include "Config.h"
#include "../utilities/Utilities.h"
#include "../utilities/VectorUtilities.h"

class Fluid final
{
public:
	Fluid(Config& config, const sf::Vector2u& size);

	[[nodiscard]] sf::Vector3f get_color(const sf::Vector2f& origin) const;

	[[nodiscard]] constexpr int IX(const int x, const int y) const noexcept;
	[[nodiscard]] constexpr int IX(const int i) const noexcept;

	[[nodiscard]] constexpr int safe_IX(int x, int y) const noexcept;

public:
	void add_density(int x, int y, float amount);
	void add_velocity(int x, int y, float vx, float vy);

	void step_line(int x0, int y0, int x1, int y1, int dx, int dy, float a);

	void update(const float dt);

private:
	void lin_solve(float* x, const float* x0, const float a, const int b, const float c);

	void set_bnd(float* x, const int b);

	void diffuse(float* x, const float* x0, const float diff, const int b, const float dt);
	void advect(float* d, const float* d0, const float* vx, const float* vy, const int b, const float dt);
	void project(float* u, float* v, float* p, float* div);

private:
	Config* _config;

	size_t W, H, N;

	std::unique_ptr<float[]> vx;
	std::unique_ptr<float[]> vy;
	std::unique_ptr<float[]> vx_prev;
	std::unique_ptr<float[]> vy_prev;

	std::unique_ptr<float[]> density;
	std::unique_ptr<float[]> density_prev;
};

