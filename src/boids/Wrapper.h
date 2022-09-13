#pragma once

#include "Boid.h"

struct Wrapper final
{
	Wrapper() = default;
	Wrapper(Wrapper& wrap) = default;
	Wrapper(Wrapper&& wrap) = default;
	Wrapper& operator=(Wrapper& rhs) = default;
	Wrapper& operator=(Wrapper&& rhs) = default;

	Wrapper(Boid& boid) : boid(&boid) {}

	bool operator<(const Wrapper& rhs) const
	{
		return boid->get_cell_index() < rhs.boid->get_cell_index();
	}
	bool operator>(const Wrapper& rhs) const
	{
		return (rhs < *this);
	}

	Boid* boid{nullptr};
};