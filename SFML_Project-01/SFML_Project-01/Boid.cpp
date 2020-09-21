#include "Boid.h"

Boid::Boid()
{
	m_Size = sf::Vector2<double>(10.0f, 5.0f);

	m_Position = sf::Vector2<double>();
	m_Velocity = sf::Vector2<double>(-5.0f + (double)(rand() % 10), -5.0f + (double)(rand() % 10));
	m_Acceleration = sf::Vector2<double>();

	m_Rotation = double();
	m_MaxSpeed = 5.0f;
	m_MaxForce = 0.2f;
	m_MinDistance = 50.0f;
}

Boid::Boid(sf::Vector2<double> pos, sf::Vector2<double> size, sf::Vector3<double> color, double maxSpeed, double maxForce, double minDistance) :
	m_Position(pos), m_Size(size), m_Color(color), m_MaxSpeed(maxSpeed), m_MaxForce(maxForce), m_MinDistance(minDistance)
{
	m_Velocity = sf::Vector2<double>(-5.0f + (double)(rand() % 10), -5.0f + (double)(rand() % 10));
	m_Acceleration = sf::Vector2<double>();

	m_Rotation = double();
}

Boid::~Boid()
{

}

void Boid::Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	Flock(boids);

	m_Velocity += m_Acceleration;
	m_Velocity = Limit(m_Velocity, m_MaxSpeed);

	sf::Vector2<double> nextPos = m_Velocity * deltaTime;
	OutsideBorder(window, nextPos);

	m_Position += nextPos;
	m_Acceleration *= 0.0;

	m_Rotation = Angle(m_Velocity) + ToRadians(180.0f);
}

void Boid::Flock(const std::vector<Boid>& boids)
{
	sf::Vector2<double> sep = Seperate(boids);
	sf::Vector2<double> ali = Align(boids);
	sf::Vector2<double> coh = Cohesion(boids);

	sep *= 1.300;
	ali *= 1.400;
	coh *= 1.310;

	ApplyForce(sep + ali + coh);
}

void Boid::OutsideBorder(const sf::Window* window, const sf::Vector2<double>& nextPos)
{
	if (m_Position.x + nextPos.x + m_Size.x < 0)
	{
		m_Position.x = (double)window->getSize().x;
	}
	else if (m_Position.x + nextPos.x - m_Size.x > window->getSize().x)
	{
		m_Position.x = -m_Size.x;
	}
	else if (m_Position.y + nextPos.y + m_Size.y < 0)
	{
		m_Position.y = (double)window->getSize().y;
	}
	else if (m_Position.y + nextPos.y - m_Size.y > window->getSize().y)
	{
		m_Position.y = -m_Size.y;
	}
}

sf::Vector2<double> Boid::Seperate(const std::vector<Boid>& boids)
{
	sf::Vector2<double> sep = sf::Vector2<double>();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			double distance = Distance(b.GetPosition(), m_Position);
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

	sf::Vector2<double> steer = sep - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

sf::Vector2<double> Boid::Align(const std::vector<Boid>& boids)
{
	sf::Vector2<double> ali = sf::Vector2<double>();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			double distance = Distance(b.GetPosition(), m_Position);
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

	sf::Vector2<double> steer = ali - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

sf::Vector2<double> Boid::Cohesion(const std::vector<Boid>& boids)
{
	sf::Vector2<double> coh = sf::Vector2<double>();
	int neighbourCount = 0;

	for (const Boid& b : boids)
	{
		if (&b != this)
		{
			double distance = Distance(b.GetPosition(), m_Position);
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

	sf::Vector2<double> desired = coh - m_Position;
	desired = Normalize(desired, m_MaxSpeed);

	sf::Vector2<double> steer = desired - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}
