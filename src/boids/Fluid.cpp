#include "Fluid.h"

Fluid::Fluid(Config& config, const sf::Vector2u& size)
	: _config(&config), W(size.x / _config->fluid_scale), H(size.y / _config->fluid_scale), N(W * H)
{
	vx = std::make_unique<float[]>(N);
	vy = std::make_unique<float[]>(N);

	vx_prev = std::make_unique<float[]>(N);
	vy_prev = std::make_unique<float[]>(N);

	density = std::make_unique<float[]>(N);
	density_prev = std::make_unique<float[]>(N);
}

sf::Vector3f Fluid::get_color(const sf::Vector2f& origin) const
{
	if (_config->fluid_colors.empty())
		return sf::Vector3f();

	const int x = (int)(origin.x / _config->fluid_scale);
	const int y = (int)(origin.y / _config->fluid_scale);

	const float vel_x = util::map_to_range(vx[safe_IX(x, y)],
		-_config->fluid_color_vel, _config->fluid_color_vel, -1.0f, 1.0f);
	const float vel_y = util::map_to_range(vy[safe_IX(x, y)],
		-_config->fluid_color_vel, _config->fluid_color_vel, -1.0f, 1.0f);

	const float bnd = (float)_config->fluid_colors.size() - 1.0f;

	float scaled_speed = vu::distance(sf::Vector2f(vel_x, vel_y)) * bnd;
	scaled_speed = std::clamp(scaled_speed, 0.0f, bnd);

	const int index1 = (int)scaled_speed;
	const int index2 = ((int)scaled_speed + 1) % (int)_config->fluid_colors.size();

	const sf::Vector3f color1 = _config->fluid_colors[index1];
	const sf::Vector3f color2 = _config->fluid_colors[index2];

	const float newT = scaled_speed - std::floorf(scaled_speed);

	return vu::lerp(color1, color2, newT) * _config->color_fluid_weight;
}

void Fluid::add_density(int x, int y, float amount)
{
	density[safe_IX(x, y)] += amount;
}

void Fluid::add_velocity(int x, int y, float vx, float vy)
{
	int index = safe_IX(x, y);

	this->vx[index] += vx;
	this->vy[index] += vy;
}

constexpr int Fluid::IX(const int x, const int y) const noexcept 
{ 
	return x + y * W; 
}
constexpr int Fluid::IX(const int i) const noexcept 
{ 
	return (i % W) + i; 
}

constexpr int Fluid::safe_IX(int x, int y) const noexcept
{
	x = std::clamp<int>(x, 0, W - 1);
	y = std::clamp<int>(y, 0, H - 1);

	return IX(x, y);
}

void Fluid::lin_solve(float* x, const float* x0, const float a, const int b, const float c)
{
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
					 x[IX(j, i + 1)])) / c;
			}
		}
		set_bnd(x, b);
	}
}

void Fluid::set_bnd(float* x, const int b)
{
	for (int i = 1; i < std::max(H - 1, W - 1); ++i)
	{
		if (i < H - 1) [[likely]]
		{
			x[IX(0,		i)] = b == 1 ? -x[IX(1,		i)] : x[IX(1,	  i)];
			x[IX(W - 1, i)] = b == 1 ? -x[IX(W - 2, i)] : x[IX(W - 2, i)];
		}

		if (i < W - 1) [[likely]]
		{
			x[IX(i, 0	 )] = b == 2 ? -x[IX(i, 1	 )] : x[IX(i, 1	   )];
			x[IX(i, H - 1)] = b == 2 ? -x[IX(i, H - 2)] : x[IX(i, H - 2)];
		}
	}

	x[IX(0,		0	 )] = 0.5f * (x[IX(1,	  0	   )] + x[IX(0,		1	 )]);
	x[IX(0,		H - 1)] = 0.5f * (x[IX(1,	  H - 1)] + x[IX(0,		H - 2)]);
	x[IX(W - 1, 0	 )] = 0.5f * (x[IX(W - 2, 0	   )] + x[IX(W - 1, 1	 )]);
	x[IX(W - 1, H - 1)] = 0.5f * (x[IX(W - 2, H - 1)] + x[IX(W - 1, H - 2)]);

}

void Fluid::diffuse(float* x, const float* x0, const float diff, const int b, const float dt)
{
	float a = dt * diff * (W - 2) * (H - 2);
	lin_solve(x, x0, a, b, 1 + 6 * a);
}

void Fluid::advect(float* d, const float* d0, const float* vx, const float* vy, const int b, const float dt)
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

			if (x < 0.5f) x = 0.5f; 
			if (x > wf + 0.5f) x = wf + 0.5f;

			i0 = std::floorf(x); 
			i1 = i0 + 1;

			if (y < 0.5f) y = 0.5f;
			if (y > hf + 0.5f) y = hf + 0.5f;
			
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

			d[IX(j, i)] =
				s0 * (t0 * d0[safe_IX(i0i, j0i)] + t1 * d0[safe_IX(i0i, j1i)]) +
				s1 * (t0 * d0[safe_IX(i1i, j0i)] + t1 * d0[safe_IX(i1i, j1i)]);
		}
	}

	set_bnd(d, b);
}

void Fluid::project(float* vx, float* vy, float* p, float* div)
{
	for (int y = 1; y < H - 1; ++y)
	{
		for (int x = 1; x < W - 1; ++x) 
		{
			div[IX(x, y)] = -0.5f * (
				vx[IX(x + 1, y	  )] - 
				vx[IX(x - 1, y	  )] +
				vy[IX(x,	 y + 1)] - 
				vy[IX(x,	 y - 1)]) / ((W + H) * 0.5f);

			p[IX(x, y)] = 0;
		}
	}

	set_bnd(div, 0);
	set_bnd(p, 0);

	lin_solve(p, div, 1, 0, 6);

	for (int y = 1; y < H - 1; ++y)
	{
		for (int x = 1; x < W - 1; ++x) 
		{
			vx[IX(x, y)] -= 0.5f * (p[IX(x + 1, y	 )] - p[IX(x - 1, y	   )]) * W;
			vy[IX(x, y)] -= 0.5f * (p[IX(x,	    y + 1)] - p[IX(x,	  y - 1)]) * H;
		}
	}

	set_bnd(vx, 1);
	set_bnd(vy, 2);
}

void Fluid::step_line(int x0, int y0, int x1, int y1, int dx, int dy, float a)
{
	if (x0 == x1 && y0 == y1)
		return;

	if (dx > dy)
	{
		int pk = 2 * dy - dx;
		for (int i = 0; i < dx; ++i)
		{
			add_velocity(x0, y0, (x1 - x0) * a, (y1 - y0) * a);

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
			add_velocity(x0, y0, (x1 - x0) * a, (y1 - y0) * a);

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

void Fluid::update(const float dt)
{
	const auto diffuseFunc = std::bind(&Fluid::diffuse, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, dt);

	const auto advectFunc = std::bind(&Fluid::advect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, dt);

	const auto projectFunc = std::bind(&Fluid::project, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	std::future<void> thread1;
	std::future<void> thread2;
	std::future<void> thread3;

	{
		thread1 = std::async(std::launch::deferred, diffuseFunc, vx_prev.get(), vx.get(), _config->fluid_viscosity, 1);
		thread2 = std::async(std::launch::deferred, diffuseFunc, vy_prev.get(), vy.get(), _config->fluid_viscosity, 2);

		thread1.wait();
		thread2.wait();
	}
	
	{
		thread1 = std::async(std::launch::deferred, projectFunc, vx_prev.get(), vy_prev.get(), vx.get(), vy.get());

		thread1.wait();

		thread2 = std::async(std::launch::deferred, advectFunc, vx.get(), vx_prev.get(), vx_prev.get(), vy_prev.get(), 1);
		thread3 = std::async(std::launch::deferred, advectFunc, vy.get(), vy_prev.get(), vx_prev.get(), vy_prev.get(), 2);

		thread2.wait();
		thread3.wait();
	}

	{
		thread1 = std::async(std::launch::deferred, projectFunc, vx.get(), vy.get(), vx_prev.get(), vy_prev.get());
		thread2 = std::async(std::launch::deferred, diffuseFunc, density_prev.get(), density.get(), _config->fluid_diffusion, 0);

		thread1.wait();

		thread3 = std::async(std::launch::deferred, advectFunc, density.get(), density_prev.get(), vx.get(), vy.get(), 0);

		thread2.wait();
		thread3.wait();
	}
}