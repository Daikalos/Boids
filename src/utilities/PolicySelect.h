#pragma once

#include <execution>

enum Policy
{
	seq,
	unseq,
	par,
	par_unseq
};

template<class F>
static auto policy_select(F f, Policy p)
{
	switch (p) 
	{
		case seq: return f(std::execution::seq);
		case unseq: return f(std::execution::unseq);
		case par: return f(std::execution::par);
		default: return f(std::execution::par_unseq);
	}
}