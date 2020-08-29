#include "Boid.h"

Boid::Boid()
{
	m_Size = sf::Vector2f(6, 3);

	m_Position = sf::Vector2f();
	m_Velocity = sf::Vector2f();
	m_Acceleration = sf::Vector2f();

	m_Rotation = float();
	m_MaxSpeed = 10.0f;
	m_MinDistance = 100.0f;
}

Boid::Boid(sf::Vector2f pos, sf::Vector2f size, sf::Vector3f color, float maxSpeed, float minDistance) :
	m_Position(pos), m_Size(size), m_Color(color), m_MaxSpeed(maxSpeed), m_MinDistance(minDistance)
{
	m_Velocity = sf::Vector2f();
	m_Acceleration = sf::Vector2f();

	m_Rotation = float();
}

Boid::~Boid()
{

}

void Boid::Update(const sf::Window* window, const float& deltaTime, const std::vector<Boid>* boids)
{
	sf::Vector2f sep = Seperate(boids);
	sf::Vector2f ali = Align(boids);
	sf::Vector2f coh = Cohesion(boids);

	ApplyForce(sep + ali + coh);

	m_Velocity += m_Acceleration * deltaTime;
	m_Velocity = Limit(m_Velocity, m_MaxSpeed, m_MaxSpeed);

	OutsideBorder(window);

	m_Position += m_Velocity;
	m_Acceleration *= 0.0f;

	m_Rotation = Angle(m_Velocity) + ToRadians(180.0f);
}

void Boid::OutsideBorder(const sf::Window* window)
{
	if (m_Position.x + m_Velocity.x < 0.0f)
	{
		m_Position.x = 0.0f;
	}
	else if (m_Position.x + m_Velocity.x > window->getSize().x)
	{
		m_Position.x = (float)window->getSize().x;
	}
	else if (m_Position.y + m_Velocity.y < 0.0f)
	{
		m_Position.y = 0.0f;
	}
	else if (m_Position.y + m_Velocity.y > window->getSize().y)
	{
		m_Position.y = (float)window->getSize().y;
	}
}

sf::Vector2f Boid::Seperate(const std::vector<Boid>* boids)
{
	sf::Vector2f sep = sf::Vector2f();
	for (Boid b : (*boids))
	{
		if (&b != this)
		{

		}
	}

	return sep;
}

sf::Vector2f Boid::Align(const std::vector<Boid>* boids)
{
	sf::Vector2f ali = sf::Vector2f();
	for (Boid b : (*boids))
	{
		if (&b != this)
		{

		}
	}

	return ali;
}

sf::Vector2f Boid::Cohesion(const std::vector<Boid>* boids)
{
	sf::Vector2f coh = sf::Vector2f();
	for (Boid b : (*boids))
	{
		if (&b != this)
		{

		}
	}

	return coh;
}
