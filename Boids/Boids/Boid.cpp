#include "Boid.h"

Boid::Boid(Grid* grid, Boid* boids, sf::Vector2f pos)
	: grid(grid), boids(boids), position(pos)
{
	velocity = v2f::normalize(sf::Vector2f(
		util::random(-Config::boid_max_speed, Config::boid_max_speed),
		util::random(-Config::boid_max_speed, Config::boid_max_speed)), Config::boid_max_speed);

	if (Config::boid_cycle_colors_random)
		cycle_time = util::random(0.0f, 1.0f);

	origin = sf::Vector2f(
		position.x + (Config::boid_size_width / 2.0f),
		position.y + (Config::boid_size_height / 2.0f));
}

void Boid::update(float deltaTime, const Rect_i& border)
{
	flock();

	position += velocity * deltaTime;

	outside_border(deltaTime, border);

	origin = sf::Vector2f(
		position.x + (Config::boid_size_width / 2.0f),
		position.y + (Config::boid_size_height / 2.0f));

	// draw-info
	{
		rotation = v2f::angle(velocity);

		pointA = v2f::rotate_point({ position.x + Config::boid_size_width, position.y + (Config::boid_size_height / 2) }, origin, rotation); // middle right tip
		pointB = v2f::rotate_point({ position.x						  , position.y								    }, origin, rotation); // top left corner
		pointC = v2f::rotate_point({ position.x						  , position.y + Config::boid_size_height	    }, origin, rotation); // bot left corner

		switch (Config::color_option)
		{
		case 0:
			position_color(border);
			break;
		case 2:
			density_color(deltaTime);
			break;
		default:
			cycle_color(deltaTime);
			break;
		}

		if (Config::impulse_enabled)
			impulse_color();
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

	density = 0.0f;

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
			sf::Vector2f dir = v2f::direction(origin, otherOrigin);

			bool use_angle = distance <= std::fminf(Config::coh_distance, Config::ali_distance);

			if (use_angle)
			{
				float angle = v2f::angle(velocity, dir);

				if (util::to_degrees(angle) <= (Config::boid_view_angle / 2.0f))
				{
					if (distance <= Config::coh_distance)
					{
						coh += otherOrigin;		  // Head towards center of boids
						++cohCount;
					}
					if (distance <= Config::ali_distance)
					{
						ali += b->get_velocity(); // Align with every boids velocity
						++aliCount;
					}
				}
			}
			if (distance <= Config::sep_distance)
			{
				sep += -dir / powf(distance, 2.0f);
				++sepCount;
			}
		}
	}

	if (sepCount > 0) // separation
	{
		sep = v2f::normalize(sep / (float)sepCount, Config::boid_max_speed);
		apply_force(steer_at(sep) * Config::sep_weight);
	}
	if (aliCount > 0) // alignment
	{
		ali = v2f::normalize(ali / (float)aliCount, Config::boid_max_speed);
		apply_force(steer_at(ali) * Config::ali_weight);
	}
	if (cohCount > 0) // cohesion
	{
		coh = v2f::direction(origin, coh / (float)cohCount);
		coh = v2f::normalize(coh, Config::boid_max_speed);

		apply_force(steer_at(coh) * Config::coh_weight);
	}

	float length = v2f::length(velocity);

	if (length > Config::boid_max_speed)
		velocity = v2f::normalize(velocity, Config::boid_max_speed);
	else if (length < Config::boid_min_speed)
		velocity = v2f::normalize(velocity, Config::boid_min_speed);

	density = std::max(std::max(cohCount, aliCount), sepCount);
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

void Boid::outside_border(const float& deltaTime, const Rect_i& border)
{
	switch (Config::turn_at_border)
	{
	case true:
		turn_at_border(deltaTime, border);
		break;
	default:
		teleport_at_border(border);
		break;
	}
}
void Boid::turn_at_border(const float& deltaTime, const Rect_i& border)
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
void Boid::teleport_at_border(const Rect_i& border)
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

void Boid::position_color(const Rect_i& border)
{
	float t = position.x / border.width();
	float s = position.y / border.height();

	color = sf::Vector3f(
		(float)interpolate(Config::boid_color_top_left.x * 255, Config::boid_color_top_right.x * 255, Config::boid_color_bot_left.x * 255, Config::boid_color_bot_right.x * 255, t, s) / 255.0f,
		(float)interpolate(Config::boid_color_top_left.y * 255, Config::boid_color_top_right.y * 255, Config::boid_color_bot_left.y * 255, Config::boid_color_bot_right.y * 255, t, s) / 255.0f,
		(float)interpolate(Config::boid_color_top_left.z * 255, Config::boid_color_top_right.z * 255, Config::boid_color_bot_left.z * 255, Config::boid_color_bot_right.z * 255, t, s) / 255.0f);
}
void Boid::cycle_color(const float& deltaTime)
{
	cycle_time = std::fmodf(cycle_time + deltaTime * Config::boid_cycle_colors_speed, 1.0f);

	float scaled_time = cycle_time * (float)(Config::boid_cycle_colors.size() - 1);

	int index1 = (int)scaled_time;
	int index2 = ((int)scaled_time + 1) % Config::boid_cycle_colors.size();

	sf::Vector3f color1 = Config::boid_cycle_colors[index1];
	sf::Vector3f color2 = Config::boid_cycle_colors[index2];

	float newT = scaled_time - std::floorf(scaled_time);

	color = v2f::lerp(color1, color2, newT);
}
void Boid::density_color(const float& deltaTime)
{
	if (Config::boid_density_cycle_enabled)
		density_time = std::fmodf(density_time + deltaTime * Config::boid_density_cycle_speed, 1.0f);

	float scaled_density = std::fmodf(((float)density / Config::boid_density) + density_time, 1.0f) * (float)(Config::boid_density_colors.size() - 1);

	int index1 = (int)scaled_density;
	int index2 = ((int)scaled_density + 1) % Config::boid_density_colors.size();

	sf::Vector3f color1 = Config::boid_density_colors[index1];
	sf::Vector3f color2 = Config::boid_density_colors[index2];

	float newT = scaled_density - std::floorf(scaled_density);

	color = v2f::lerp(color1, color2, newT);
}
void Boid::impulse_color()
{
	for (const Impulse& impulse : Config::impulses)
	{
		sf::Vector2f impulse_pos = impulse.get_position();
		float impulse_length = impulse.get_length();

		float length = v2f::length(v2f::direction(impulse_pos, position));
		float diff = std::abs(length - impulse_length);

		float percentage = (impulse_length / Config::impulse_fade_distance);
		float size = impulse.get_size() * (1.0f - percentage);

		float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::impulse_colors.size() - 1);

		int index1 = (int)scaled_length;
		int index2 = ((int)scaled_length + 1) % Config::impulse_colors.size();

		sf::Vector3f color1 = Config::impulse_colors[index1];
		sf::Vector3f color2 = Config::impulse_colors[index2];

		float newT = scaled_length - std::floorf(scaled_length);

		if (diff <= size)
			color = v2f::lerp(color1, color2, newT);
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
