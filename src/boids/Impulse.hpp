#pragma once

#include <SFML/Graphics.hpp>

class Impulse
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

	constexpr sf::Vector2f get_position() const { return _position; }
	constexpr float get_length() const { return _length; }
	constexpr float get_size() const { return _size; }

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
