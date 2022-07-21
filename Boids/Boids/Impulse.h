#pragma once

#include <SFML/Graphics.hpp>

class Impulse
{
public:
	Impulse(sf::Vector2f position, float speed, float size, float length)
		: position(position), speed(speed), size(size), length(length) { }

	Impulse& operator=(const Impulse& rhs)
	{
		return *this;
	}

	sf::Vector2f get_position() const
	{
		return position;
	}
	float get_speed() const
	{
		return speed;
	}
	float get_size() const
	{
		return size;
	}
	float get_length() const
	{
		return length;
	}

	void update(const float& deltaTime)
	{
		length += speed * deltaTime;
	}

private:
	const sf::Vector2f position{0.0f, 0.0f};
	const float speed{0.0f};
	const float size{ 0.0f };
	float length{0.0f};
};
