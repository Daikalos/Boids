#include "Impulse.h"

Impulse::Impulse(sf::Vector2f position, float speed, float size, float length)
	: m_position(position), m_speed(speed), m_size(size), m_length(length) { }

const sf::Vector2f& Impulse::GetPosition() const noexcept	{ return m_position; }
float Impulse::GetLength() const noexcept					{ return m_length; }
float Impulse::GetSize() const noexcept						{ return m_size; }

void Impulse::Update(float dt)
{
	m_length += m_speed * dt;
}