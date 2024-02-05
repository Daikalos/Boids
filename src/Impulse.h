#pragma once

#include <SFML/System/Vector2.hpp>

class Impulse final
{
public:
	Impulse(sf::Vector2f position, float speed, float size, float length);

public:
	[[nodiscard]] const sf::Vector2f& GetPosition() const noexcept;
	[[nodiscard]] float GetLength() const noexcept;
	[[nodiscard]] float GetSize() const noexcept;

public:
	void Update(float dt);

private:
	sf::Vector2f	m_position;
	float			m_speed		{0.0f};
	float			m_size		{0.0f};
	float			m_length	{0.0f};
};
