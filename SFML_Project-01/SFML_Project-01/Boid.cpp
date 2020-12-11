#include "Boid.h"

Boid::Boid()
{
	position = vec2d(0, 0);
	velocity = vec2d(fRand(-5.0, 5.0), fRand(-5.0, 5.0));

	size = vec2d(10.0f, 5.0f);

	rotation = 0.0;
	maxSpeed = 150.0;
	maxSteer = 1.5;
	minDistance = 35.0;
	viewAngle = 260.0;
}

Boid::Boid(vec2d pos, vec2d size, sf::Vector3<double> color, double maxSpeed, double maxSteer, double minDistance, double viewAngle) :
	position(pos), size(size), color(color), maxSpeed(maxSpeed), maxSteer(maxSteer), minDistance(minDistance), viewAngle(viewAngle)
{
	velocity = vec2d(
		fRand(-maxSpeed, maxSpeed), 
		fRand(-maxSpeed, maxSpeed));

	rotation = 0.0;
}

void Boid::Update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	Flock(boids);

	velocity = Vector2::Limit(velocity, maxSpeed);
	position += velocity * deltaTime;

	rotation = Vector2::Angle(velocity) + ToRadians(180.0);

	OutsideBorder(window);
}

std::vector<Boid> Boid::VisibleBoids(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids; // Filter all near boids based on min distance and view angle

	for (const Boid& b : boids)
	{
		if (&b == this)
			continue;

		double distance = Vector2::Distance(b.GetPosition(), position);
		if (distance > 0 && distance < minDistance)
		{
			vec2d dir = Vector2::Direction(position, b.GetPosition());
			double angle = Vector2::Angle(velocity, dir);

			if (ToDegrees(angle) < (viewAngle / 2))
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
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return sep;

	for (const Boid& b : boids)
	{
		double distance = Vector2::Distance(b.GetPosition(), position);
		if (distance < (minDistance / 2))
		{
			sep += (position - b.GetPosition()) / pow(distance, 2);
		}
	}

	sep /= neighbourCount;
	sep = Vector2::Normalize(sep, maxSpeed);

	vec2d steer = sep - velocity;
	steer = Vector2::Limit(steer, maxSteer);

	return steer;
}

vec2d Boid::Align(const std::vector<Boid>& boids)
{
	vec2d ali = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return ali;

	for (const Boid& b : boids)
		ali += b.GetVelocity();

	ali /= neighbourCount;
	ali = Vector2::Normalize(ali, maxSpeed);

	vec2d steer = ali - velocity;
	steer = Vector2::Limit(steer, maxSteer);

	return steer;
}

vec2d Boid::Cohesion(const std::vector<Boid>& boids)
{
	vec2d coh = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return coh;

	for (const Boid& b : boids)
		coh += b.GetPosition();

	coh /= neighbourCount;

	vec2d desired = coh - position;
	desired = Vector2::Normalize(desired, maxSpeed);

	vec2d steer = desired - velocity;
	steer = Vector2::Limit(steer, maxSteer);

	return steer;
}

void Boid::OutsideBorder(const sf::Window* window)
{
	if (position.x + size.x < 0)
	{
		position.x = (double)window->getSize().x;
	}
	else if (position.x - size.x > window->getSize().x)
	{
		position.x = -size.x;
	}
	else if (position.y + size.y < 0)
	{
		position.y = (double)window->getSize().y;
	}
	else if (position.y - size.y > window->getSize().y)
	{
		position.y = -size.y;
	}
}
