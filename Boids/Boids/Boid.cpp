#include "Boid.h"

Boid::Boid()
{
	position = sf::Vector2f(0.0f, 0.0f);
	color = sf::Vector3f(0.0f, 0.0f, 0.0f);

	size = sf::Vector2f(10.0f, 5.0f);

	weight_sep = 1.0;
	weight_ali = 1.0;
	weight_coh = 1.0;

	rotation = 0.0;
	max_speed = 150.0;
	max_steer = 1.5;
	min_distance = 35.0;
	view_angle = 260.0;

	velocity = sf::Vector2f(
		util::fRand(-max_speed, max_speed),
		util::fRand(-max_speed, max_speed));
}

Boid::Boid(
	sf::Vector2f pos, sf::Vector2f size, 
	float w_sep, float w_ali, float w_coh,
	float max_speed, float max_steer, float min_distance, float view_angle)
	: position(pos), size(size), 
	weight_sep(w_sep), weight_ali(w_ali), weight_coh(w_coh), 
	max_speed(max_speed), max_steer(max_steer), min_distance(min_distance), view_angle(view_angle)
{
	velocity = sf::Vector2f(
		util::fRand(-max_speed, max_speed),
		util::fRand(-max_speed, max_speed));
	color = sf::Vector3f(0.0f, 0.0f, 0.0f);

	rotation = 0.0;
}

void Boid::update(const sf::Window* window, float deltaTime, const std::vector<const Boid*>& boids)
{
	flock(boids);

	velocity = v2f::limit(velocity, max_speed);
	position += velocity * deltaTime;

	rotation = v2f::angle(-velocity);

	outside_border(window);
}

std::vector<const Boid*> Boid::visible_boids(const std::vector<const Boid*>& boids)
{
	std::vector<const Boid*> visBoids; // Filter all near boids based on min distance and view angle

	for (const Boid* b : boids)
	{
		if (b == this)
			continue;

		double distance = v2f::distance(b->get_origin(), get_origin());
		if (distance > FLT_EPSILON && distance < min_distance)
		{
			sf::Vector2f dir = v2f::direction(get_origin(), b->get_origin());
			double angle = v2f::angle(
				v2f::normalize(velocity), 
				v2f::normalize(dir));

			if (util::to_degrees(angle) < (view_angle / 2))
			{
				visBoids.push_back(b);
			}
		}
	}

	return visBoids;
}

void Boid::flock(const std::vector<const Boid*>& boids)
{
	std::vector<const Boid*> visBoids = visible_boids(boids);

	sf::Vector2f sep = seperate(visBoids);
	sf::Vector2f ali = align(visBoids);
	sf::Vector2f coh = cohesion(visBoids);

	sep *= weight_sep;
	ali *= weight_ali;
	coh *= weight_coh;

	apply_force(sep + ali + coh);
}

sf::Vector2f Boid::seperate(const std::vector<const Boid*>& boids)
{
	sf::Vector2f sep = sf::Vector2f(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return sep;

	for (const Boid* b : boids)
	{
		float distance = v2f::distance(b->get_origin(), get_origin());
		if (distance < (min_distance / 2))
		{
			// Seperate more strongly the closer to the boid
			sep += (get_origin() - b->get_origin()) / (float)pow(distance, 2);
		}
	}

	sep /= (float)neighbourCount; // Average
	sep = v2f::normalize(sep, max_speed); // set magnitude to max_speed 

	sf::Vector2f steer = sep - velocity; // steering direction
	steer = v2f::limit(steer, max_steer);

	return steer;
}

sf::Vector2f Boid::align(const std::vector<const Boid*>& boids)
{
	sf::Vector2f ali = sf::Vector2f(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return ali;

	for (const Boid* b : boids)
		ali += b->get_velocity(); // Align with every boids velocity

	ali /= (float)neighbourCount;
	ali = v2f::normalize(ali, max_speed);

	sf::Vector2f steer = ali - velocity;
	steer = v2f::limit(steer, max_steer);

	return steer;
}

sf::Vector2f Boid::cohesion(const std::vector<const Boid*>& boids)
{
	sf::Vector2f coh = sf::Vector2f(0, 0);
	size_t neighbourCount = boids.size();

	if (neighbourCount == 0)
		return coh;

	for (const Boid* b : boids)
		coh += b->get_origin(); // Head towards center of boids

	coh /= (float)neighbourCount;

	sf::Vector2f desired = v2f::direction(coh, get_origin());
	desired = v2f::normalize(desired, max_speed);

	sf::Vector2f steer = desired - velocity;
	steer = v2f::limit(steer, max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float force)
{
	sf::Vector2f steer = v2f::direction(point, get_origin()) - velocity;
	steer = v2f::limit(steer, max_steer * force);

	apply_force(steer);
}

void Boid::steer_away(sf::Vector2f point, float force)
{
	sf::Vector2f steer = velocity - v2f::direction(point, get_origin());
	steer = v2f::limit(steer, max_steer * force);

	apply_force(steer);
}

void Boid::outside_border(const sf::Window* window)
{
	if (position.x + size.x < 0)
	{
		position.x = (float)window->getSize().x;
	}
	if (position.x - size.x > window->getSize().x)
	{
		position.x = -size.x;
	}
	if (position.y + size.y < 0)
	{
		position.y = (float)window->getSize().y;
	}
	if (position.y - size.y > window->getSize().y)
	{
		position.y = -size.y;
	}
}
