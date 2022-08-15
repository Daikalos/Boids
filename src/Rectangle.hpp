#pragma once

#include <SFML/Graphics.hpp>

template<typename T> 
struct Rect
{
	Rect() = default;

	Rect(T left, T top, T right, T bot)
		: top_left({ left, top }), bot_right({ right, bot }) { };
	Rect(sf::Vector2<T> top_left, sf::Vector2<T> bot_right)
		: top_left(top_left), bot_right(bot_right) { };

	template<typename U>
	explicit Rect(Rect<U> rect) :	
		top_left(sf::Vector2<T>(rect.top_left)), 
		bot_right(sf::Vector2<T>(rect.bot_right)) { };

	inline T width() const { return (right - left); }
	inline T height() const { return (bot - top); }

	inline sf::Vector2<T> size() { return sf::Vector2<T>(width(), height()); }
	inline T count() { return width() * height(); }

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