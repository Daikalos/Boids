#include "Boid.h"

Boid::Boid(sf::Vector2f pos) 
	: position(pos), rotation(0.0f)
{
	velocity = sf::Vector2f(
		util::random(-Config::boid_max_speed, Config::boid_max_speed),
		util::random(-Config::boid_max_speed, Config::boid_max_speed));
}

void Boid::update(float deltaTime, const Rect_i& border)
{
	flock();

	position += velocity * deltaTime;
	outside_border(border, deltaTime);

	// draw-info
	{
		rotation = v2f::angle(velocity);

		sf::Vector2f origin = get_origin();

		pointA = v2f::rotate_point({ position.x + Config::boid_size_width, position.y + (Config::boid_size_height / 2) }, origin, rotation); // middle right tip
		pointB = v2f::rotate_point({ position.x							 , position.y								   }, origin, rotation); // top left corner
		pointC = v2f::rotate_point({ position.x							 , position.y + Config::boid_size_height	   }, origin, rotation); // bot left corner

		float t = position.x / border.width();
		float s = position.y / border.height();

		color =
		{
			(float)interpolate(Config::boid_color_top_left.x * 255, Config::boid_color_top_right.x * 255, Config::boid_color_bot_left.x * 255, Config::boid_color_bot_right.x * 255, t, s) / 255.0f,
			(float)interpolate(Config::boid_color_top_left.y * 255, Config::boid_color_top_right.y * 255, Config::boid_color_bot_left.y * 255, Config::boid_color_bot_right.y * 255, t, s) / 255.0f,
			(float)interpolate(Config::boid_color_top_left.z * 255, Config::boid_color_top_right.z * 255, Config::boid_color_bot_left.z * 255, Config::boid_color_bot_right.z * 255, t, s) / 255.0f
		};
	}
}

void Boid::flock()
{
	if (container == nullptr)
		return;

	sf::Vector2f sep(0, 0);
	sf::Vector2f ali(0, 0);
	sf::Vector2f coh(0, 0);

	float sepCount = 0;
	float aliCount = 0;
	float cohCount = 0;

	float sepDistance = (Config::boid_min_distance / 2.0f);

	for (const Container* c : container->neighbours)
		for (const Boid* b : c->items) // do in one loop
		{
			if (b == this)
				continue;

			float distance = v2f::distance(get_origin(), b->get_origin());
			if (distance >= FLT_EPSILON && distance <= Config::boid_min_distance)
			{
				sf::Vector2f dir = v2f::direction(get_origin(), b->get_origin());
				float angle = v2f::angle(velocity, dir);

				if (util::to_degrees(angle) <= (Config::boid_view_angle / 2))
				{
					ali += b->get_velocity(); // Align with every boids velocity
					coh += b->get_origin();   // Head towards center of boids

					++aliCount;
					++cohCount;
				}

				if (distance <= sepDistance)
				{
					sep += v2f::direction(b->get_origin(), get_origin()) / powf(distance, 2.0f);
					++sepCount;
				}
			}
		}

	if (sepCount > 0) // seperation
	{
		sep = v2f::normalize(sep / sepCount, Config::boid_max_speed);
		apply_force(steer_at(sep) * Config::weight_sep);
	}
	if (aliCount > 0) // alignment
	{
		ali = v2f::normalize(ali / aliCount, Config::boid_max_speed);
		apply_force(steer_at(ali) * Config::weight_ali);
	}
	if (cohCount > 0) // cohesion
	{
		coh = v2f::direction(get_origin(), coh / cohCount);
		coh = v2f::normalize(coh, Config::boid_max_speed);

		apply_force(steer_at(coh) * Config::weight_coh);
	}

	velocity = v2f::limit(velocity, Config::boid_max_speed);
	velocity = v2f::min(velocity, Config::boid_min_speed);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = steer_direction - velocity; // steering direction
	steer = v2f::limit(steer, Config::boid_max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	sf::Vector2f steer = v2f::direction(velocity, v2f::normalize(v2f::direction(get_origin(), point), v2f::length(velocity)));
	steer = v2f::normalize(steer, Config::boid_max_steer * weight);

	apply_force(steer);
}

void Boid::outside_border(const Rect_i& border, float deltaTime)
{
	if (!Config::turn_at_border)
	{
		if (position.x + Config::boid_size_width < border.left)
			position.x = (float)border.right;

		if (position.x > border.right)
			position.x = border.left - Config::boid_size_width;

		if (position.y + Config::boid_size_height < border.top)
			position.y = (float)border.bot;

		if (position.y > border.bot)
			position.y = border.top - Config::boid_size_height;
	}
	else
	{
		float width_margin = border.width() - border.width() * Config::turn_margin_factor;
		float height_margin = border.height() - border.height() * Config::turn_margin_factor;

		float left_margin = border.left + width_margin;
		float top_margin = border.top + height_margin;
		float right_margin = border.right - width_margin;
		float bot_margin = border.bot - height_margin;

		if (position.x + Config::boid_size_width < left_margin)
			velocity.x += Config::turn_factor * deltaTime * (1.0f + std::powf(std::abs(position.x - left_margin) / width_margin, 2.0f));

		if (position.x > right_margin)
			velocity.x -= Config::turn_factor * deltaTime * (1.0f + std::powf(std::abs(position.x - right_margin) / width_margin, 2.0f));

		if (position.y + Config::boid_size_height < top_margin)
			velocity.y += Config::turn_factor * deltaTime * (1.0f + std::powf(std::abs(position.y - top_margin) / height_margin, 2.0f));

		if (position.y > bot_margin)
			velocity.y -= Config::turn_factor * deltaTime * (1.0f + std::powf(std::abs(position.y - bot_margin) / height_margin, 2.0f));
	}
}
