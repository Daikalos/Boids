#include "Boid.h"

Boid::Boid()
{
	m_Size = sf::Vector2f(10.0f, 5.0f);

	m_Position = sf::Vector2f();
	m_Velocity = sf::Vector2f(-5.0f + (float)(rand() % 10), -5.0f + (float)(rand() % 10));
	m_Acceleration = sf::Vector2f();

	m_Rotation = float();
	m_MaxSpeed = 5.0f;
	m_MaxForce = 0.2f;
	m_MinDistance = 50.0f;
}

Boid::Boid(sf::Vector2f pos, sf::Vector2f size, sf::Vector3f color, float maxSpeed, float maxForce, float minDistance) :
	m_Position(pos), m_Size(size), m_Color(color), m_MaxSpeed(maxSpeed), m_MaxForce(maxForce), m_MinDistance(minDistance)
{
	m_Velocity = sf::Vector2f(-5.0f + (float)(rand() % 10), -5.0f + (float)(rand() % 10));
	m_Acceleration = sf::Vector2f();

	m_Rotation = float();
}

Boid::~Boid()
{

}

void Boid::Update(const sf::Window* window, const float& deltaTime, const std::vector<Boid>& boids)
{
	Flock(boids);

	m_Velocity += m_Acceleration * deltaTime;
	m_Velocity = Limit(m_Velocity, m_MaxSpeed);

	OutsideBorder(window);

	m_Position += m_Velocity;
	m_Acceleration *= 0.0f;

	m_Rotation = Angle(m_Velocity) + ToRadians(180.0f);
}

void Boid::Flock(const std::vector<Boid>& boids)
{
	sf::Vector2f sep = Seperate(boids);
	sf::Vector2f ali = Align(boids);
	sf::Vector2f coh = Cohesion(boids);

	sep *= 1.5f;
	ali *= 1.1f;
	coh *= 1.7f;

	ApplyForce(sep + ali + coh);
}

void Boid::OutsideBorder(const sf::Window* window)
{
	if (m_Position.x + m_Velocity.x + m_Size.x < 0)
	{
		m_Position.x = (float)window->getSize().x;
	}
	else if (m_Position.x + m_Velocity.x - m_Size.x > window->getSize().x)
	{
		m_Position.x = -m_Size.x;
	}
	else if (m_Position.y + m_Velocity.y + m_Size.y < 0)
	{
		m_Position.y = (float)window->getSize().y;
	}
	else if (m_Position.y + m_Velocity.y - m_Size.y > window->getSize().y)
	{
		m_Position.y = -m_Size.y;
	}
}

sf::Vector2f Boid::Seperate(const std::vector<Boid>& boids)
{
	sf::Vector2f sep = sf::Vector2f();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			float distance = Distance(b.GetPosition(), m_Position);
			if (distance > 0 && distance < (m_MinDistance / 2))
			{
				sep += (m_Position - b.GetPosition()) / pow(distance, 2);
				neighbourCount++;
			}
		}
	}

	if (neighbourCount == 0)
		return sep;

	sep.x /= neighbourCount;
	sep.y /= neighbourCount;
	sep = Normalize(sep, m_MaxSpeed);

	sf::Vector2f steer = sep - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

sf::Vector2f Boid::Align(const std::vector<Boid>& boids)
{
	sf::Vector2f ali = sf::Vector2f();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			float distance = Distance(b.GetPosition(), m_Position);
			if (distance > 0 && distance < m_MinDistance)
			{
				ali += b.GetVelocity();
				neighbourCount++;
			}
		}
	}

	if (neighbourCount == 0)
		return ali;

	ali.x /= neighbourCount;
	ali.y /= neighbourCount;
	ali = Normalize(ali, m_MaxSpeed);

	sf::Vector2f steer = ali - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

sf::Vector2f Boid::Cohesion(const std::vector<Boid>& boids)
{
	sf::Vector2f coh = sf::Vector2f();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			float distance = Distance(b.GetPosition(), m_Position);
			if (distance > 0 && distance < m_MinDistance)
			{
				coh += b.GetPosition();
				neighbourCount++;
			}
		}
	}

	if (neighbourCount == 0)
		return coh;

	coh.x /= neighbourCount;
	coh.y /= neighbourCount;

	sf::Vector2f desired = coh - m_Position;
	desired = Normalize(desired, m_MaxSpeed);

	sf::Vector2f steer = desired - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}
