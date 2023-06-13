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
	Fluid();
	Fluid(const sf::Vector2u& size);

public:
	[[nodiscard]] sf::Vector3f GetColor(const sf::Vector2f& origin) const;

public:
	void AddDensity(int x, int y, float amount);
	void AddVelocity(int x, int y, float vx, float vy);

	void StepLine(int x0, int y0, int x1, int y1, int dx, int dy, float a);

	void Update(float dt);

private:
	void LinSolve(float* x, const float* x0, float a, int b, float c);

	void SetBnd(float* x, int b);

	void Diffuse(float* x, const float* x0, float diff, int b, float dt);
	void Advect(float* d, const float* d0, const float* vx, const float* vy, int b, float dt);
	void Project(float* u, float* v, float* p, float* div);

private:
	[[nodiscard]] int IX(int x, int y) const noexcept;
	[[nodiscard]] int SafeIX(int x, int y) const noexcept;

private:
	int W, H, N;

	std::unique_ptr<float[]> vx;
	std::unique_ptr<float[]> vy;
	std::unique_ptr<float[]> vx_prev;
	std::unique_ptr<float[]> vy_prev;

	std::unique_ptr<float[]> density;
	std::unique_ptr<float[]> density_prev;
};

