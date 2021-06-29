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

void Boid::update(float deltaTime, const Rect_i& border, const std::vector<const Boid*>& boids)
{
	flock(boids);

	velocity = v2f::limit(velocity, max_speed);

	position += velocity * deltaTime;

	rotation = v2f::angle(velocity);

	outside_border(border);

	// draw-info
	{
		sf::Vector2f origin = get_origin();

		pointA = v2f::rotate_point({ position.x + size.x, position.y + (size.y / 2) }, origin, rotation); // middle right tip
		pointB = v2f::rotate_point({ position.x			, position.y }, origin, rotation); // top left corner
		pointC = v2f::rotate_point({ position.x			, position.y + size.y }, origin, rotation); // bot left corner

		color =
		{
			0.5f + ((origin.x) / border.width()),
			(origin.x * origin.y) / (border.width() * border.height()),
			0.5f + ((origin.y) / border.bot)
		};
	}
}

void Boid::flock(const std::vector<const Boid*>& boids)
{
	if (boids.size() == 0)
		return;

	sf::Vector2f sep(0, 0);
	sf::Vector2f ali(0, 0);
	sf::Vector2f coh(0, 0);

	int sepCount = 0;
	int aliCount = 0;
	int cohCount = 0;

	for (const Boid* b : boids) // do in one loop
	{
		if (b == this)
			continue;

		double distance = v2f::distance(get_origin(), b->get_origin());
		if (distance <= min_distance)
		{
			sf::Vector2f dir = v2f::direction(get_origin(), b->get_origin());
			double angle = v2f::angle(velocity, dir);

			if (util::to_degrees(angle) <= (view_angle / 2))
			{
				ali += b->get_velocity(); // Align with every boids velocity
				coh += b->get_origin();   // Head towards center of boids

				++aliCount;
				++cohCount;
			}

			if (distance <= (min_distance / 2.0f))
			{
				sep += (get_origin() - b->get_origin()) / (float)pow(distance, 2);
				++sepCount;
			}
		}
	}

	if (sepCount != 0) // seperation
	{
		sep /= (float)sepCount;
		sep = v2f::normalize(sep, max_speed);

		apply_force(steer_at(sep) * weight_sep);
	}
	if (aliCount != 0) // alignment
	{
		ali /= (float)aliCount;
		ali = v2f::normalize(ali, max_speed);

		apply_force(steer_at(ali) * weight_ali);
	}
	if (cohCount != 0) // cohesion
	{
		coh /= (float)cohCount;
		coh = v2f::direction(get_origin(), coh);
		coh = v2f::normalize(coh, max_speed);

		apply_force(steer_at(coh) * weight_coh);
	}
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = steer_direction - velocity; // steering direction
	steer = v2f::limit(steer, max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	sf::Vector2f steer = v2f::direction(velocity, v2f::normalize(v2f::direction(get_origin(), point), v2f::length(velocity)));
	steer = v2f::normalize(steer, max_steer * weight);

	apply_force(steer);
}

void Boid::steer_away(sf::Vector2f point, float weight)
{
	sf::Vector2f steer = v2f::direction(v2f::normalize(v2f::direction(get_origin(), point), v2f::length(velocity)), velocity);
	steer = v2f::normalize(steer, max_steer * weight);

	apply_force(steer);
}

void Boid::outside_border(const Rect_i& border)
{
	if (position.x + size.x < border.left)
	{
		position.x = (float)border.right;
	}
	if (position.x - size.x > border.right)
	{
		position.x = border.left - size.x;
	}
	if (position.y + size.y < border.top)
	{
		position.y = (float)border.bot;
	}
	if (position.y - size.y > border.bot)
	{
		position.y = border.top - size.y;
	}
}
