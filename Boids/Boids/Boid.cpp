#include "Boid.h"

Boid::Boid(Grid* grid, Boid* boids, sf::Vector2f pos)
	: grid(grid), boids(boids), index(0), cell_index(0), position(pos), rotation(0.0f), duration(0.0f)
{
	velocity = sf::Vector2f(
		util::random(-Config::boid_max_speed, Config::boid_max_speed),
		util::random(-Config::boid_max_speed, Config::boid_max_speed));

	if (Config::boid_cycle_colors_random)
		duration = util::random(0.0f, 1.0f);
}

void Boid::update(float deltaTime, const Rect_i& border)
{
	flock();

	position += velocity * deltaTime;

	outside_border(border, deltaTime);

	origin = sf::Vector2f(
		position.x + (Config::boid_size_width / 2.0f),
		position.y + (Config::boid_size_height / 2.0f));

	// draw-info
	{
		rotation = v2f::angle(velocity);

		pointA = v2f::rotate_point({ position.x + Config::boid_size_width, position.y + (Config::boid_size_height / 2) }, origin, rotation); // middle right tip
		pointB = v2f::rotate_point({ position.x							 , position.y								   }, origin, rotation); // top left corner
		pointC = v2f::rotate_point({ position.x							 , position.y + Config::boid_size_height	   }, origin, rotation); // bot left corner

		if (!Config::boid_cycle_colors_enabled)
		{
			float t = position.x / border.width();
			float s = position.y / border.height();

			color = sf::Vector3f(
				(float)interpolate(Config::boid_color_top_left.x * 255, Config::boid_color_top_right.x * 255, Config::boid_color_bot_left.x * 255, Config::boid_color_bot_right.x * 255, t, s) / 255.0f,
				(float)interpolate(Config::boid_color_top_left.y * 255, Config::boid_color_top_right.y * 255, Config::boid_color_bot_left.y * 255, Config::boid_color_bot_right.y * 255, t, s) / 255.0f,
				(float)interpolate(Config::boid_color_top_left.z * 255, Config::boid_color_top_right.z * 255, Config::boid_color_bot_left.z * 255, Config::boid_color_bot_right.z * 255, t, s) / 255.0f);
		}
		else
		{
			duration = std::fmodf(duration + deltaTime * Config::boid_cycle_colors_speed, 1.0f);
			
			float scaled_time = duration * (float)(Config::boid_cycle_colors.size() - 1);

			int index1 = (int)scaled_time;
			int index2 = ((int)scaled_time + 1) % Config::boid_cycle_colors.size();

			sf::Vector3f color1 = Config::boid_cycle_colors[index1];
			sf::Vector3f color2 = Config::boid_cycle_colors[index2];

			float newT = scaled_time - std::floorf(scaled_time);

			color = v2f::lerp(color1, color2, newT);
		}
	}
}

void Boid::flock()
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	int sepCount = 0;
	int aliCount = 0;
	int cohCount = 0;

	float sepDistance = (Config::boid_min_distance / 2.0f);

	int neighbours = 0;
	int neighbourIndices[4];

	sf::Vector2f gridCellIndicesRaw = grid->relative_pos(position);
	sf::Vector2i gridCellIndices = sf::Vector2i(gridCellIndicesRaw);
	sf::Vector2f gridCellOverflow = gridCellIndicesRaw - sf::Vector2f(gridCellIndices);

	int xNeighbor = gridCellIndices.x + (gridCellOverflow.x > 0.5f ? 1 : -1);
	int yNeighbor = gridCellIndices.y + (gridCellOverflow.y > 0.5f ? 1 : -1);

	if (xNeighbor < 0 || xNeighbor >= grid->width)
		xNeighbor = -1;
	if (yNeighbor < 0 || yNeighbor >= grid->height)
		yNeighbor = -1;

	neighbourIndices[neighbours++] = grid->at_pos(gridCellIndices.x, gridCellIndices.y);

	if (xNeighbor != -1)
		neighbourIndices[neighbours++] = grid->at_pos(xNeighbor, gridCellIndices.y);
	if (yNeighbor != -1)
		neighbourIndices[neighbours++] = grid->at_pos(gridCellIndices.x, yNeighbor);
	if (xNeighbor != -1 && yNeighbor != -1)
		neighbourIndices[neighbours++] = grid->at_pos(xNeighbor, yNeighbor);

	for (int i = 0; i < neighbours; ++i)
	{
		int gridCellIndex = neighbourIndices[i];
		for (int j = grid->cellsStartIndices[gridCellIndex]; j <= grid->cellsEndIndices[gridCellIndex] && j > -1; ++j) // do in one loop
		{
			Boid* b = &boids[j];

			if (b == this)
				continue;

			sf::Vector2f otherOrigin = b->get_origin();

			float distance = v2f::distance(origin, otherOrigin);
			if (distance >= FLT_EPSILON && distance <= Config::boid_min_distance)
			{
				sf::Vector2f dir = v2f::direction(origin, otherOrigin);
				float angle = v2f::angle(velocity, dir);

				if (util::to_degrees(angle) <= (Config::boid_view_angle / 2.0f))
				{
					ali += b->get_velocity(); // Align with every boids velocity
					coh += otherOrigin;		  // Head towards center of boids

					++aliCount;
					++cohCount;
				}

				if (distance <= sepDistance)
				{
					sep += -dir / powf(distance, 2.0f);
					++sepCount;
				}
			}
		}
	}

	if (sepCount > 0) // seperation
	{
		sep = v2f::normalize(sep / (float)sepCount, Config::boid_max_speed);
		apply_force(steer_at(sep) * Config::weight_sep);
	}
	if (aliCount > 0) // alignment
	{
		ali = v2f::normalize(ali / (float)aliCount, Config::boid_max_speed);
		apply_force(steer_at(ali) * Config::weight_ali);
	}
	if (cohCount > 0) // cohesion
	{
		coh = v2f::direction(origin, coh / (float)cohCount);
		coh = v2f::normalize(coh, Config::boid_max_speed);

		apply_force(steer_at(coh) * Config::weight_coh);
	}

	float length = v2f::length(velocity);

	if (length > Config::boid_max_speed)
		velocity = v2f::normalize(velocity, Config::boid_max_speed);
	else if (length < Config::boid_min_speed)
		velocity = v2f::normalize(velocity, Config::boid_min_speed);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = v2f::direction(velocity, steer_direction); // steering direction
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

void Boid::update_grid_cells() const
{
	int index = static_cast<int>(this - boids);

	if (index == 0)
	{
		grid->cellsStartIndices[cell_index] = index;
		return;
	}

	if (index == Config::boid_count - 1)
		grid->cellsEndIndices[cell_index] = index;

	int otherIndex = boids[index - 1].get_cell_index();

	if (otherIndex != cell_index)
	{
		grid->cellsStartIndices[cell_index] = index;
		grid->cellsEndIndices[otherIndex] = index - 1;
	}
}
