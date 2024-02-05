#pragma once

#include <execution>

enum class Policy
{
	seq,
	unseq,
	par,
	par_unseq
};

template<class F>
inline auto PolicySelect(F&& f, Policy p)
{
	switch (p) 
	{
	case Policy::seq:		return f(std::execution::seq);
	case Policy::unseq:		return f(std::execution::unseq);
	case Policy::par:		return f(std::execution::par);
	case Policy::par_unseq:	return f(std::execution::par_unseq);
	}
}