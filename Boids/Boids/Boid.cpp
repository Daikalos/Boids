#include "Boid.h"

Boid::Boid(sf::Vector2f pos, Config* cfg) 
	: position(pos), rotation(0.0f), cfg(cfg)
{
	velocity = sf::Vector2f(
		util::fRand(-cfg->boid_max_speed, cfg->boid_max_speed),
		util::fRand(-cfg->boid_max_speed, cfg->boid_max_speed));
}

void Boid::update(float deltaTime, const Rect_i& border, const std::vector<const Container<Boid>*>& containers)
{
	flock(containers);
	position += velocity * deltaTime;
	outside_border(border, deltaTime);

	rotation = v2f::angle(velocity);

	// draw-info
	{
		sf::Vector2f origin = get_origin();

		pointA = v2f::rotate_point({ position.x + cfg->boid_size_width, position.y + (cfg->boid_size_height / 2) }, origin, rotation); // middle right tip
		pointB = v2f::rotate_point({ position.x						  , position.y								 }, origin, rotation); // top left corner
		pointC = v2f::rotate_point({ position.x						  , position.y + cfg->boid_size_height		 }, origin, rotation); // bot left corner

		float t = position.x / border.width();
		float s = position.y / border.height();

		color =
		{
			(float)interpolate(cfg->boid_color_top_left.x * 255, cfg->boid_color_top_right.x * 255, cfg->boid_color_bot_left.x * 255, cfg->boid_color_bot_right.x * 255, t, s) / 255.0f,
			(float)interpolate(cfg->boid_color_top_left.y * 255, cfg->boid_color_top_right.y * 255, cfg->boid_color_bot_left.y * 255, cfg->boid_color_bot_right.y * 255, t, s) / 255.0f,
			(float)interpolate(cfg->boid_color_top_left.z * 255, cfg->boid_color_top_right.z * 255, cfg->boid_color_bot_left.z * 255, cfg->boid_color_bot_right.z * 255, t, s) / 255.0f
		};
	}
}

void Boid::flock(const std::vector<const Container<Boid>*>& containers)
{
	if (containers.size() == 0)
		return;

	sf::Vector2f sep(0, 0);
	sf::Vector2f ali(0, 0);
	sf::Vector2f coh(0, 0);

	int sepCount = 0;
	int aliCount = 0;
	int cohCount = 0;

	for (const Container<Boid>* c : containers)
		for (const Boid* b : c->items) // do in one loop
		{
			if (b == this || b == nullptr)
				continue;

			float distance = v2f::distance(get_origin(), b->get_origin());
			if (distance <= cfg->boid_min_distance)
			{
				sf::Vector2f dir = v2f::direction(get_origin(), b->get_origin());
				float angle = v2f::angle(velocity, dir);

				if (util::to_degrees(angle) <= (cfg->boid_view_angle / 2))
				{
					ali += b->get_velocity(); // Align with every boids velocity
					coh += b->get_origin();   // Head towards center of boids

					++aliCount;
					++cohCount;
				}

				if (distance <= (cfg->boid_min_distance / 2.0f))
				{
					sep += (get_origin() - b->get_origin()) / (float)pow(distance, 2);
					++sepCount;
				}
			}
		}

	if (sepCount > 0) // seperation
	{
		sep /= (float)sepCount;
		sep = v2f::normalize(sep, cfg->boid_max_speed);

		apply_force(steer_at(sep) * cfg->weight_sep);
	}
	if (aliCount > 0) // alignment
	{
		ali /= (float)aliCount;
		ali = v2f::normalize(ali, cfg->boid_max_speed);

		apply_force(steer_at(ali) * cfg->weight_ali);
	}
	if (cohCount > 0) // cohesion
	{
		coh /= (float)cohCount;
		coh = v2f::direction(get_origin(), coh);
		coh = v2f::normalize(coh, cfg->boid_max_speed);

		apply_force(steer_at(coh) * cfg->weight_coh);
	}

	velocity = v2f::limit(velocity, cfg->boid_max_speed);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = steer_direction - velocity; // steering direction
	steer = v2f::limit(steer, cfg->boid_max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	sf::Vector2f steer = v2f::direction(velocity, v2f::normalize(v2f::direction(get_origin(), point), v2f::length(velocity)));
	steer = v2f::normalize(steer, cfg->boid_max_steer * weight);

	apply_force(steer);
}

void Boid::steer_away(sf::Vector2f point, float weight)
{
	sf::Vector2f steer = v2f::direction(v2f::normalize(v2f::direction(get_origin(), point), v2f::length(velocity)), velocity);
	steer = v2f::normalize(steer, cfg->boid_max_steer * weight);

	apply_force(steer);
}

void Boid::outside_border(const Rect_i& border, float deltaTime)
{
	if (!cfg->turn_at_border)
	{
		if (position.x + cfg->boid_size_width < border.left)
			position.x = (float)border.right;

		if (position.x - cfg->boid_size_width > border.right)
			position.x = border.left - cfg->boid_size_width;

		if (position.y + cfg->boid_size_height < border.top)
			position.y = (float)border.bot;

		if (position.y - cfg->boid_size_height > border.bot)
			position.y = border.top - cfg->boid_size_height;
	}
	else
	{
		float width_margin = border.width() - border.width() * cfg->turn_margin_factor;
		float height_margin = border.height() - border.height() * cfg->turn_margin_factor;

		if (position.x + cfg->boid_size_width < width_margin)
			velocity.x += cfg->turn_factor * deltaTime;

		if (position.x - cfg->boid_size_width > border.right - width_margin)
			velocity.x -= cfg->turn_factor * deltaTime;

		if (position.y + cfg->boid_size_height < height_margin)
			velocity.y += cfg->turn_factor * deltaTime;

		if (position.y - cfg->boid_size_height > border.bot - height_margin)
			velocity.y -= cfg->turn_factor * deltaTime;
	}
}
