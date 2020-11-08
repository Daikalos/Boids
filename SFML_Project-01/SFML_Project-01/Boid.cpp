#include "Boid.h"

Boid::Boid()
{
	m_Size = vec2d(10.0f, 5.0f);

	m_Position = vec2d(0, 0);
	m_Velocity = vec2d(-5.0f + (double)(rand() % 10), -5.0f + (double)(rand() % 10));
	m_Acceleration = vec2d(0, 0);

	m_Rotation = 0.0;
	m_MaxSpeed = 5.0;
	m_MaxForce = 0.2;
	m_MinDistance = 50.0;
}

Boid::Boid(vec2d pos, vec2d size, sf::Vector3<double> color, double maxSpeed, double maxForce, double minDistance) :
	m_Position(pos), m_Size(size), m_Color(color), m_MaxSpeed(maxSpeed), m_MaxForce(maxForce), m_MinDistance(minDistance)
{
	m_Velocity = vec2d(-5.0 + (double)(rand() % 10), -5.0 + (double)(rand() % 10));
	m_Acceleration = vec2d(0, 0);

	m_Rotation = 0.0;
}

Boid::~Boid()
{

}

void Boid::Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	Flock(boids);

	m_Velocity += m_Acceleration;
	m_Velocity = Limit(m_Velocity, m_MaxSpeed);

	vec2d nextPos = m_Velocity * deltaTime;
	OutsideBorder(window, nextPos);

	m_Position += nextPos;
	m_Acceleration = vec2d(0, 0);

	m_Rotation = Angle(m_Velocity) + ToRadians(180.0f);
}

void Boid::Flock(const std::vector<Boid>& boids)
{
	vec2d sep = Seperate(boids);
	vec2d ali = Align(boids);
	vec2d coh = Cohesion(boids);

	sep *= 1.300;
	ali *= 1.400;
	coh *= 1.310;

	ApplyForce(sep + ali + coh);
}

void Boid::OutsideBorder(const sf::Window* window, const vec2d& nextPos)
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

vec2d Boid::Seperate(const std::vector<Boid>& boids)
{
	vec2d sep = vec2d(0, 0);
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

	vec2d steer = sep - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

vec2d Boid::Align(const std::vector<Boid>& boids)
{
	vec2d ali = vec2d(0, 0);
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

	vec2d steer = ali - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}

vec2d Boid::Cohesion(const std::vector<Boid>& boids)
{
	vec2d coh = vec2d(0, 0);
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

	vec2d desired = coh - m_Position;
	desired = Normalize(desired, m_MaxSpeed);

	vec2d steer = desired - m_Velocity;
	steer = Limit(steer, m_MaxForce);

	return steer;
}
