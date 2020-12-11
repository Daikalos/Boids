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

void Boid::update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	flock(boids);

	velocity = Vector2::limit(velocity, maxSpeed);
	position += velocity * deltaTime;

	rotation = Vector2::angle(velocity) + to_radians(180.0);

	outside_border(window);
}

std::vector<Boid> Boid::visible_boids(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids; // Filter all near boids based on min distance and view angle

	for (const Boid& b : boids)
	{
		if (&b == this)
			continue;

		double distance = Vector2::distance(b.getPosition(), position);
		if (distance > 0 && distance < minDistance)
		{
			vec2d dir = Vector2::direction(position, b.getPosition());
			double angle = Vector2::angle(velocity, dir);

			if (to_degrees(angle) < (viewAngle / 2))
			{
				visBoids.push_back(b);
			}
		}
	}

	return visBoids;
}

void Boid::flock(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids = visible_boids(boids);

	vec2d sep = seperate(visBoids);
	vec2d ali = align(visBoids);
	vec2d coh = cohesion(visBoids);

	sep *= 1.320;
	ali *= 1.400;
	coh *= 1.310;

	apply_force(sep + ali + coh);
}

vec2d Boid::seperate(const std::vector<Boid>& boids)
{
	vec2d sep = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return sep;

	for (const Boid& b : boids)
	{
		double distance = Vector2::distance(b.getPosition(), position);
		if (distance < (minDistance / 2))
		{
			sep += (position - b.getPosition()) / pow(distance, 2);
		}
	}

	sep /= neighbourCount;
	sep = Vector2::normalize(sep, maxSpeed);

	vec2d steer = sep - velocity;
	steer = Vector2::limit(steer, maxSteer);

	return steer;
}

vec2d Boid::align(const std::vector<Boid>& boids)
{
	vec2d ali = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return ali;

	for (const Boid& b : boids)
		ali += b.getVelocity();

	ali /= neighbourCount;
	ali = Vector2::normalize(ali, maxSpeed);

	vec2d steer = ali - velocity;
	steer = Vector2::limit(steer, maxSteer);

	return steer;
}

vec2d Boid::cohesion(const std::vector<Boid>& boids)
{
	vec2d coh = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return coh;

	for (const Boid& b : boids)
		coh += b.getPosition();

	coh /= neighbourCount;

	vec2d desired = coh - position;
	desired = Vector2::normalize(desired, maxSpeed);

	vec2d steer = desired - velocity;
	steer = Vector2::limit(steer, maxSteer);

	return steer;
}

void Boid::outside_border(const sf::Window* window)
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
