#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <memory>

#include "ThreadPool.h"

class Fluid final
{
public:
	Fluid() = default;

	Fluid(const Fluid&) = delete;
	Fluid(Fluid&&) = default;

	Fluid& operator=(const Fluid&) = delete;
	Fluid& operator=(Fluid&&) = default;

	void Initialize(const sf::Vector2u& size);

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
	[[nodiscard]] constexpr int IX(int x, int y) const;
	[[nodiscard]] constexpr int SafeIX(int x, int y) const;

	[[nodiscard]] constexpr bool IsWithin(int x, int y) const;

private:
	int W{0}, H{0}, N{0};

	std::unique_ptr<float[]> m_vx;
	std::unique_ptr<float[]> m_vy;
	std::unique_ptr<float[]> m_vxPrev;
	std::unique_ptr<float[]> m_vyPrev;

	std::unique_ptr<float[]> m_density;
	std::unique_ptr<float[]> m_densityPrev;

	static ThreadPool threadPool;
};

constexpr int Fluid::IX(int x, int y) const
{
	return x + y * W;
}

constexpr int Fluid::SafeIX(int x, int y) const
{
	x = std::clamp<int>(x, 0, W - 1);
	y = std::clamp<int>(y, 0, H - 1);

	return IX(x, y);
}

constexpr bool Fluid::IsWithin(int x, int y) const
{
	if (x < 0 || y < 0)
		return false;
	if (x >= W || y >= H)
		return false;

	return true;
}
