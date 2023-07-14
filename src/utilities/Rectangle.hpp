#pragma once

#include <SFML/System/Vector2.hpp>

template<typename T> 
struct Rect
{
	Rect() { top_left = bot_right = sf::Vector2<T>(); };

	Rect(T left, T top, T right, T bot)
		: top_left({ left, top }), bot_right({ right, bot }) { };
	Rect(sf::Vector2<T> top_left, sf::Vector2<T> bot_right)
		: top_left(top_left), bot_right(bot_right) { };

	template<typename U>
	explicit Rect(const Rect<U>& rect) :	
		top_left(sf::Vector2<T>(rect.top_left)), 
		bot_right(sf::Vector2<T>(rect.bot_right)) { };

	constexpr T width() const noexcept { return (right - left); }
	constexpr T height() const noexcept { return (bot - top); }

	constexpr sf::Vector2<T> size() const noexcept { return sf::Vector2<T>(width(), height()); }
	constexpr T count() const { return width() * height(); }

	Rect<T>& operator+=(const Rect<T>& rhs)
	{
		top_left += rhs.top_left;
		bot_right += rhs.bot_right;

		return *this;
	}
	Rect<T> operator+(const Rect<T>& rhs) const
	{
		return (Rect<T>(*this) += rhs);
	}

	union
	{
		sf::Vector2<T> top_left;
		struct { T left, top; };
	};
	union
	{
		sf::Vector2<T> bot_right;
		struct { T right, bot; };
	};
};

using RectInt = Rect<int>;
using RectFloat = Rect<float>;