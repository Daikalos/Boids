#include "Fluid.h"

#include <future>
#include <algorithm>

#include "../utilities/CommonUtilities.hpp"
#include "../utilities/VectorUtilities.hpp"

#include "Config.h"

ThreadPool Fluid::threadPool(3);

Fluid::Fluid(const sf::Vector2u& size)
	: W(size.x / Config::Inst().Fluid.Scale)
	, H(size.y / Config::Inst().Fluid.Scale)
	, N(W * H)
	, m_vx(std::make_unique<float[]>(N))
	, m_vy(std::make_unique<float[]>(N))
	, m_vxPrev(std::make_unique<float[]>(N))
	, m_vyPrev(std::make_unique<float[]>(N))
	, m_density(std::make_unique<float[]>(N))
	, m_densityPrev(std::make_unique<float[]>(N))
{

}

sf::Vector3f Fluid::GetColor(const sf::Vector2f& origin) const
{
	if (Config::Inst().Fluid.Colors.empty())
		return sf::Vector3f();

	const int x = (int)(origin.x / Config::Inst().Fluid.Scale);
	const int y = (int)(origin.y / Config::Inst().Fluid.Scale);

	if (!IsWithin(x, y))
		return sf::Vector3f();

	const float vx = util::MapToRange(m_vx[IX(x, y)],
		-Config::Inst().Fluid.ColorVel, Config::Inst().Fluid.ColorVel, -1.0f, 1.0f);
	const float vy = util::MapToRange(m_vy[IX(x, y)],
		-Config::Inst().Fluid.ColorVel, Config::Inst().Fluid.ColorVel, -1.0f, 1.0f);

	const float bnd = (float)Config::Inst().Fluid.Colors.size() - 1.0f;

	float scaledSpeed = sf::Vector2f(vx, vy).length() * bnd;
	scaledSpeed = std::clamp(scaledSpeed, 0.0f, bnd);

	const int i1 = (int)scaledSpeed;
	const int i2 = (i1 == (int)Config::Inst().Fluid.Colors.size() - 1) ? 0 : i1 + 1;

	const sf::Vector3f color1 = Config::Inst().Fluid.Colors[i1];
	const sf::Vector3f color2 = Config::Inst().Fluid.Colors[i2];

	const float newT = scaledSpeed - std::floorf(scaledSpeed);

	return vu::Lerp(color1, color2, newT) * Config::Inst().Color.FluidWeight;
}

void Fluid::AddDensity(int x, int y, float amount)
{
	m_density[SafeIX(x, y)] += amount;
}

void Fluid::AddVelocity(int x, int y, float vx, float vy)
{
	int index = SafeIX(x, y);

	m_vx[index] += vx;
	m_vy[index] += vy;
}

void Fluid::LinSolve(float* x, const float* x0, float a, int b, float c)
{
	if (c == 0.0f)
		return;

	c = (1.0f / c);

	for (int k = 0; k < 2; ++k)
	{
		for (int i = 1; i < H - 1; ++i)
		{
			for (int j = 1; j < W - 1; ++j)
			{
				x[IX(j, i)] = (x0[IX(j, i)] + a *
					(x[IX(j - 1, i)] +
					 x[IX(j + 1, i)] +
					 x[IX(j, i - 1)] +
					 x[IX(j, i + 1)])) * c;
			}
		}

		SetBnd(x, b);
	}
}

void Fluid::SetBnd(float* x, int b)
{
	switch (b)
	{
		case 0:
		{
			for (int i = 1; i < H - 1; ++i)
			{
				x[IX(0,		i)] = x[IX(1,	  i)];
				x[IX(W - 1, i)] = x[IX(W - 2, i)];
			}

			for (int i = 1; i < W - 1; ++i)
			{
				x[IX(i, 0	 )] = x[IX(i, 1	   )];
				x[IX(i, H - 1)] = x[IX(i, H - 2)];
			}

			break;
		}
		case 1:
		{
			for (int i = 1; i < H - 1; ++i)
			{
				x[IX(0,		i)] = -x[IX(1,		i)];
				x[IX(W - 1, i)] = -x[IX(W - 2,	i)];
			}

			for (int i = 1; i < W - 1; ++i)
			{
				x[IX(i, 0	 )] = x[IX(i, 1	   )];
				x[IX(i, H - 1)] = x[IX(i, H - 2)];
			}

			break;
		}
		case 2:
		{
			for (int i = 1; i < H - 1; ++i)
			{
				x[IX(0,		i)] = x[IX(1,	  i)];
				x[IX(W - 1, i)] = x[IX(W - 2, i)];
			}

			for (int i = 1; i < W - 1; ++i)
			{
				x[IX(i, 0	 )] = -x[IX(i, 1	)];
				x[IX(i, H - 1)] = -x[IX(i, H - 2)];
			}

			break;
		}
	}

	x[IX(0,		0	 )] = 0.5f * (x[IX(1,	  0	   )] + x[IX(0,		1	 )]);
	x[IX(0,		H - 1)] = 0.5f * (x[IX(1,	  H - 1)] + x[IX(0,		H - 2)]);
	x[IX(W - 1, 0	 )] = 0.5f * (x[IX(W - 2, 0	   )] + x[IX(W - 1, 1	 )]);
	x[IX(W - 1, H - 1)] = 0.5f * (x[IX(W - 2, H - 1)] + x[IX(W - 1, H - 2)]);

}

void Fluid::Diffuse(float* x, const float* x0, float diff, int b, float dt)
{
	float a = dt * diff * (W - 2) * (H - 2);
	LinSolve(x, x0, a, b, 1 + 6 * a);
}

void Fluid::Advect(float* d, const float* d0, const float* vx, const float* vy, int b, float dt)
{
	float i0, j0, i1, j1;
	float x, y, s0, t0, s1, t1;

	float dtx = dt * float(W - 2);
	float dty = dt * float(H - 2);

	float wf = float(W);
	float hf = float(H);

	for (int i = 1; i < H - 1; ++i) 
	{
		for (int j = 1; j < W - 1; ++j) 
		{
			x = float(j) - dtx * vx[IX(j, i)]; 
			y = float(i) - dty * vy[IX(j, i)];

			x = std::clamp(x, 0.5f, wf - 1.5f);

			i0 = std::floorf(x); 
			i1 = i0 + 1;

			y = std::clamp(y, 0.5f, hf - 1.5f);
			
			j0 = std::floorf(y);
			j1 = j0 + 1;

			s1 = x - i0; 
			s0 = 1.0f - s1; 
			t1 = y - j0; 
			t0 = 1.0f - t1;

			const int i0i = (int)i0;
			const int i1i = (int)i1;
			const int j0i = (int)j0;
			const int j1i = (int)j1;

			assert(IsWithin(i0i, j0i) && IsWithin(i1i, j1i));

			d[IX(j, i)] =
				s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
				s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);
		}
	}

	SetBnd(d, b);
}

void Fluid::Project(float* vx, float* vy, float* p, float* div)
{
	const float inv = -1.0f / (W + H);

	for (int y = 1; y < H - 1; ++y)
	{
		for (int x = 1; x < W - 1; ++x) 
		{
			div[IX(x, y)] = inv * (
				vx[IX(x + 1, y	  )] - 
				vx[IX(x - 1, y	  )] +
				vy[IX(x,	 y + 1)] - 
				vy[IX(x,	 y - 1)]);

			p[IX(x, y)] = 0;
		}
	}

	SetBnd(div, 0);
	SetBnd(p, 0);

	LinSolve(p, div, 1, 0, 6);

	for (int y = 1; y < H - 1; ++y)
	{
		for (int x = 1; x < W - 1; ++x) 
		{
			vx[IX(x, y)] -= 0.5f * (p[IX(x + 1, y	 )] - p[IX(x - 1, y	   )]) * W;
			vy[IX(x, y)] -= 0.5f * (p[IX(x,	    y + 1)] - p[IX(x,	  y - 1)]) * H;
		}
	}

	SetBnd(vx, 1);
	SetBnd(vy, 2);
}

void Fluid::StepLine(int x0, int y0, int x1, int y1, int dx, int dy, float a)
{
	if (x0 == x1 && y0 == y1)
		return;

	if (dx > dy)
	{
		int pk = 2 * dy - dx;
		for (int i = 0; i < dx; ++i)
		{
			AddVelocity(x0, y0, (x1 - x0) * a, (y1 - y0) * a);

			x0 < x1 ? ++x0 : --x0;

			if (pk < 0) pk += 2 * dy;
			else
			{
				y0 < y1 ? ++y0 : --y0;
				pk += 2 * dy - 2 * dx;
			}
		}
	}
	else
	{
		int pk = 2 * dx - dy;
		for (int i = 0; i < dy; ++i)
		{
			AddVelocity(x0, y0, (x1 - x0) * a, (y1 - y0) * a);

			y0 < y1 ? ++y0 : --y0;

			if (pk < 0) pk += 2 * dx;
			else
			{
				x0 < x1 ? ++x0 : --x0;
				pk += 2 * dx - 2 * dy;
			}
		}
	}
}

void Fluid::Update(float dt)
{
	{
		auto thread1 = threadPool.Enqueue(&Fluid::Diffuse, this, m_vxPrev.get(), m_vx.get(), Config::Inst().Fluid.Viscosity, 1, dt);
		auto thread2 = threadPool.Enqueue(&Fluid::Diffuse, this, m_vyPrev.get(), m_vy.get(), Config::Inst().Fluid.Viscosity, 2, dt);

		thread1.wait();
		thread2.wait();
	}
	
	{
		auto thread1 = threadPool.Enqueue(&Fluid::Project, this, m_vxPrev.get(), m_vyPrev.get(), m_vx.get(), m_vy.get());

		thread1.wait();
		 
		auto thread2 = threadPool.Enqueue(&Fluid::Advect, this, m_vx.get(), m_vxPrev.get(), m_vxPrev.get(), m_vyPrev.get(), 1, dt);
		auto thread3 = threadPool.Enqueue(&Fluid::Advect, this, m_vy.get(), m_vyPrev.get(), m_vxPrev.get(), m_vyPrev.get(), 2, dt);

		thread2.wait();
		thread3.wait();
	}

	{
		auto thread1 = threadPool.Enqueue(&Fluid::Project, this, m_vx.get(), m_vy.get(), m_vxPrev.get(), m_vyPrev.get());
		auto thread2 = threadPool.Enqueue(&Fluid::Diffuse, this, m_densityPrev.get(), m_density.get(), Config::Inst().Fluid.Diffusion, 0, dt);

		thread1.wait();

		auto thread3 = threadPool.Enqueue(&Fluid::Advect, this, m_density.get(), m_densityPrev.get(), m_vx.get(), m_vy.get(), 0, dt);

		thread2.wait();
		thread3.wait();
	}
}
