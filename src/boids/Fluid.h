#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <memory>

class Fluid final
{
public:
	Fluid() = default;
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
	[[nodiscard]] constexpr int IX(int x, int y) const noexcept;
	[[nodiscard]] int SafeIX(int x, int y) const noexcept;

private:
	int W{0}, H{0}, N{0};

	std::unique_ptr<float[]> vx;
	std::unique_ptr<float[]> vy;
	std::unique_ptr<float[]> vx_prev;
	std::unique_ptr<float[]> vy_prev;

	std::unique_ptr<float[]> density;
	std::unique_ptr<float[]> density_prev;
};

constexpr int Fluid::IX(int x, int y) const noexcept
{
	return x + y * W;
}

