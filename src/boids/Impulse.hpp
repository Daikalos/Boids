#pragma once

#include <SFML/Graphics.hpp>

class Impulse final
{
public:
	Impulse(sf::Vector2f position, float speed, float size, float length)
		: _position(position), _speed(speed), _size(size), _length(length) { }

	Impulse& operator=(const Impulse& rhs)
	{
		_position = rhs._position;
		_length = rhs._length;
		return *this;
	}

	[[nodiscard]] constexpr sf::Vector2f get_position() const noexcept 
	{ 
		return _position; 
	}
	[[nodiscard]] constexpr float get_length() const noexcept 
	{ 
		return _length; 
	}
	[[nodiscard]] constexpr float get_size() const noexcept 
	{
		return _size; 
	}

	void update(float dt)
	{
		_length += _speed * dt;
	}

private:
	sf::Vector2f	_position	{0.0f, 0.0f};
	const float		_speed		{0.0f};
	const float		_size		{0.0f};
	float			_length		{0.0f};
};
