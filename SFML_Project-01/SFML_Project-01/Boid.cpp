#include "Boid.h"

Boid::Boid()
{
	m_Size = vec2d(10.0f, 5.0f);

	m_Position = vec2d(0, 0);
	m_Velocity = vec2d(fRand(-5.0, 5.0), fRand(-5.0, 5.0));

	m_Rotation = 0.0;
	m_MaxSpeed = 5.0;
	m_MaxSteer = 0.09;
	m_MinDistance = 50.0;
	m_ViewAngle = 300.0;
}

Boid::Boid(vec2d pos, vec2d size, sf::Vector3<double> color, double maxSpeed, double maxSteer, double minDistance, double viewAngle) :
	m_Position(pos), m_Size(size), m_Color(color), m_MaxSpeed(maxSpeed), m_MaxSteer(maxSteer), m_MinDistance(minDistance), m_ViewAngle(viewAngle)
{
	m_Rotation = 0.0;

	m_Velocity = vec2d(
		fRand(-maxSpeed, maxSpeed), 
		fRand(-maxSpeed, maxSpeed));
}

void Boid::Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	Flock(boids);

	m_Velocity = Vector2::Limit(m_Velocity, m_MaxSpeed);
	m_Position += m_Velocity * deltaTime;

	m_Rotation = Vector2::Angle(m_Velocity) + ToRadians(180.0);

	OutsideBorder(window);
}

std::vector<Boid> Boid::VisibleBoids(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids; // Filter all near boids based on min distance and view angle

	for (const Boid& b : boids)
	{
		if (&b == this)
			continue;

		double distance = Vector2::Distance(b.GetPosition(), m_Position);
		if (distance > 0 && distance < m_MinDistance)
		{
			vec2d dir = Vector2::Direction(m_Position, b.GetPosition());
			double angle = Vector2::Angle(m_Velocity, dir);

			if (ToDegrees(angle) < (m_ViewAngle / 2))
			{
				visBoids.push_back(b);
			}
		}
	}

	return visBoids;
}

void Boid::Flock(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids = VisibleBoids(boids);

	vec2d sep = Seperate(visBoids);
	vec2d ali = Align(visBoids);
	vec2d coh = Cohesion(visBoids);

	sep *= 1.320;
	ali *= 1.400;
	coh *= 1.310;

	ApplyForce(sep + ali + coh);
}

vec2d Boid::Seperate(const std::vector<Boid>& boids)
{
	vec2d sep = vec2d(0, 0);
	int neighbourCount = boids.size();

	if (neighbourCount == 0)
		return sep;

	for (const Boid& b : boids)
	{
		double distance = Vector2::Distance(b.GetPosition(), m_Position);
		if (distance < (m_MinDistance / 2))
		{
			sep += (m_Position - b.GetPosition()) / pow(distance, 2);
		}
	}

	sep /= neighbourCount;
	sep = Vector2::Normalize(sep, m_MaxSpeed);

	vec2d steer = sep - m_Velocity;
	steer = Vector2::Limit(steer, m_MaxSteer);

	return steer;
}

vec2d Boid::Align(const std::vector<Boid>& boids)
{
	vec2d ali = vec2d(0, 0);
	int neighbourCount = boids.size();

	if (neighbourCount == 0)
		return ali;

	for (const Boid& b : boids)
		ali += b.GetVelocity();

	ali /= neighbourCount;
	ali = Vector2::Normalize(ali, m_MaxSpeed);

	vec2d steer = ali - m_Velocity;
	steer = Vector2::Limit(steer, m_MaxSteer);

	return steer;
}

vec2d Boid::Cohesion(const std::vector<Boid>& boids)
{
	vec2d coh = vec2d(0, 0);
	int neighbourCount = boids.size();

	if (neighbourCount == 0)
		return coh;

	for (const Boid& b : boids)
		coh += b.GetPosition();

	coh /= neighbourCount;

	vec2d desired = coh - m_Position;
	desired = Vector2::Normalize(desired, m_MaxSpeed);

	vec2d steer = desired - m_Velocity;
	steer = Vector2::Limit(steer, m_MaxSteer);

	return steer;
}

void Boid::OutsideBorder(const sf::Window* window)
{
	if (m_Position.x + m_Size.x < 0)
	{
		m_Position.x = (double)window->getSize().x;
	}
	else if (m_Position.x - m_Size.x > window->getSize().x)
	{
		m_Position.x = -m_Size.x;
	}
	else if (m_Position.y + m_Size.y < 0)
	{
		m_Position.y = (double)window->getSize().y;
	}
	else if (m_Position.y - m_Size.y > window->getSize().y)
	{
		m_Position.y = -m_Size.y;
	}
}
