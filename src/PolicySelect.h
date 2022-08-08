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
auto maybe_parallel(F f, Policy p) 
{
	switch (p) 
	{
		case seq: return f(std::execution::seq);
		case unseq: return f(std::execution::unseq);
		case par: return f(std::execution::par);
		default: return f(std::execution::par_unseq);
	}
}