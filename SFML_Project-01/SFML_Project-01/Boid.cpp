#include "Boid.h"

Boid::Boid()
{
	position = vec2d(0.0, 0.0);
	color = vec3d(0.0, 0.0, 0.0);

	size = vec2d(10.0f, 5.0f);

	weight_sep = 1.0;
	weight_ali = 1.0;
	weight_coh = 1.0;

	rotation = 0.0;
	max_speed = 150.0;
	max_steer = 1.5;
	min_distance = 35.0;
	view_angle = 260.0;

	velocity = vec2d(
		util::fRand(-max_speed, max_speed),
		util::fRand(-max_speed, max_speed));
}

Boid::Boid(vec2d pos, vec2d size, double w_sep, double w_ali, double w_coh, double max_speed, double max_steer, double min_distance, double view_angle)
	: position(pos), size(size), weight_sep(w_sep), weight_ali(w_ali), weight_coh(w_coh), max_speed(max_speed), max_steer(max_steer), min_distance(min_distance), view_angle(view_angle)
{
	velocity = vec2d(
		util::fRand(-max_speed, max_speed),
		util::fRand(-max_speed, max_speed));
	color = vec3d(0.0, 0.0, 0.0);

	rotation = 0.0;
}

void Boid::update(const sf::Window* window, const double& deltaTime, const std::vector<Boid>& boids)
{
	flock(boids);

	velocity = Vector2::limit(velocity, max_speed);
	position += velocity * deltaTime;

	rotation = Vector2::angle(velocity) + util::to_radians(180.0);

	outside_border(window);
}

std::vector<Boid> Boid::visible_boids(const std::vector<Boid>& boids)
{
	std::vector<Boid> visBoids; // Filter all near boids based on min distance and view angle

	for (const Boid& b : boids)
	{
		if (&b == this)
			continue;

		double distance = Vector2::distance(b.get_position(), position);
		if (distance > 0 && distance < min_distance)
		{
			vec2d dir = Vector2::direction(position, b.get_position());
			double angle = Vector2::angle(
				Vector2::normalize(velocity), 
				Vector2::normalize(dir));

			if (util::to_degrees(angle) < (view_angle / 2))
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

	sep *= weight_sep;
	ali *= weight_ali;
	coh *= weight_coh;

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
		double distance = Vector2::distance(b.get_position(), position);
		if (distance < (min_distance / 2))
		{
			// Seperate more strongly the closer to the boid
			sep += (position - b.get_position()) / pow(distance, 2);
		}
	}

	sep /= neighbourCount; // Average
	sep = Vector2::normalize(sep, max_speed); // set magnitude to max_speed 

	vec2d steer = sep - velocity; // steering direction
	steer = Vector2::limit(steer, max_steer);

	return steer;
}

vec2d Boid::align(const std::vector<Boid>& boids)
{
	vec2d ali = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return ali;

	for (const Boid& b : boids)
		ali += b.get_velocity(); // Align with every boids velocity

	ali /= neighbourCount;
	ali = Vector2::normalize(ali, max_speed);

	vec2d steer = ali - velocity;
	steer = Vector2::limit(steer, max_steer);

	return steer;
}

vec2d Boid::cohesion(const std::vector<Boid>& boids)
{
	vec2d coh = vec2d(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return coh;

	for (const Boid& b : boids)
		coh += b.get_position(); // Head towards center of boids

	coh /= neighbourCount;

	vec2d desired = Vector2::direction(coh, position);
	desired = Vector2::normalize(desired, max_speed);

	vec2d steer = desired - velocity;
	steer = Vector2::limit(steer, max_steer);

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
