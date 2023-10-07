#pragma once

#include <SFML/System/Vector2.hpp>

template<typename T> 
struct Rect
{
	constexpr Rect()
		: topLeft()
		, botRight()
	{

	};

	constexpr Rect(T l, T t, T r, T b)
		: topLeft({ l, t })
		, botRight({ r, b }) 
	{ 

	};

	constexpr Rect(sf::Vector2<T> tl, sf::Vector2<T> br)
		: topLeft(tl)
		, botRight(br) 
	{ 

	};

	template<typename U>
	explicit Rect(const Rect<U>& rect) :	
		topLeft(sf::Vector2<T>(rect.topLeft)), 
		botRight(sf::Vector2<T>(rect.botRight)) { };

	constexpr T Width() const noexcept { return (right - left); }
	constexpr T Height() const noexcept { return (bot - top); }

	constexpr sf::Vector2<T> Size() const noexcept { return sf::Vector2<T>(Width(), Height()); }
	constexpr T Count() const { return Width() * Height(); }

	Rect<T>& operator+=(const Rect<T>& rhs)
	{
		topLeft += rhs.topLeft;
		botRight += rhs.botRight;

		return *this;
	}
	Rect<T> operator+(const Rect<T>& rhs) const
	{
		return (Rect<T>(*this) += rhs);
	}

	union
	{
		sf::Vector2<T> topLeft;
		struct { T left, top; };
	};
	union
	{
		sf::Vector2<T> botRight;
		struct { T right, bot; };
	};
};

using RectInt = Rect<int>;
using RectFloat = Rect<float>;