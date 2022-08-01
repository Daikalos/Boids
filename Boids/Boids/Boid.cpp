#include "Boid.h"

Boid::Boid(Grid* grid, Config* config, const AudioMeter* audio_meter, const Rect_i* border, const sf::Vector2f& pos)
	: grid(grid), config(config), audio_meter(audio_meter), border(border), position(pos)
{
	velocity = v2f::normalize(sf::Vector2f(
		util::random(-config->boid_max_speed, config->boid_max_speed),
		util::random(-config->boid_max_speed, config->boid_max_speed)), config->boid_max_speed);

	if (config->boid_cycle_colors_random)
		cycle_time = util::random(0.0f, 1.0f);

	update_points();
}

void Boid::update(const std::vector<Boid>& boids, const std::vector<Impulse>& impulses, const float& dt)
{
	prev_pointA = pointA;
	prev_pointB = pointB;
	prev_pointC = pointC;

	prev_color = color;

	flock(boids);

	sf::Vector2f new_pos = position + velocity * dt;
	position = outside_border(new_pos, dt);

	update_points();

	if (position != new_pos)
	{
		prev_pointA = pointA;
		prev_pointB = pointB;
		prev_pointC = pointC;
	}

	// draw-info
	{
		switch (config->color_option)
		{
		case 0:
			position_color();
			break;
		case 2:
			density_color(dt);
			break;
		case 3:
			audio_color(dt);
			break;
		default:
			cycle_color(dt);
			break;
		}

		impulse_color(impulses);
	}
}

void Boid::flock(const std::vector<Boid>& boids)
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	int sepCount = 0;
	int aliCount = 0;
	int cohCount = 0;

	int neighbours = 0;
	int neighbourIndices[4] {-1,-1,-1,-1};

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
			const Boid* b = &boids[j];

			if (b == this && b != nullptr)
				continue;

			sf::Vector2f otherOrigin = b->get_origin();
			float distance = v2f::distance_squared(origin, otherOrigin);

			if (distance <= FLT_EPSILON)
				continue;

			sf::Vector2f dir = v2f::direction(origin, otherOrigin);
			bool use_angle = distance <= std::fminf(config->coh_distance, config->ali_distance);

			if (use_angle)
			{
				float angle = v2f::angle(velocity, dir);

				if (util::to_degrees(angle) <= (config->boid_view_angle / 2.0f))
				{
					if (distance <= config->coh_distance)
					{
						coh += otherOrigin;		  // Head towards center of boids
						++cohCount;
					}
					if (distance <= config->ali_distance)
					{
						ali += b->get_velocity(); // Align with every boids velocity
						++aliCount;
					}
				}
			}
			if (distance <= config->sep_distance)
			{
				sep += -dir / distance;
				++sepCount;
			}
		}
	}

	if (sepCount > 0) // separation
	{
		sep = v2f::normalize(sep / (float)sepCount, config->boid_max_speed);
		apply_force(steer_at(sep) * config->sep_weight);
	}
	if (aliCount > 0) // alignment
	{
		ali = v2f::normalize(ali / (float)aliCount, config->boid_max_speed);
		apply_force(steer_at(ali) * config->ali_weight);
	}
	if (cohCount > 0) // cohesion
	{
		coh = v2f::direction(origin, coh / (float)cohCount);
		coh = v2f::normalize(coh, config->boid_max_speed);

		apply_force(steer_at(coh) * config->coh_weight);
	}

	float length = v2f::length(velocity);

	if (length > config->boid_max_speed)
		velocity = v2f::normalize(velocity, config->boid_max_speed);
	else if (length < config->boid_min_speed)
		velocity = v2f::normalize(velocity, config->boid_min_speed);

	density = std::fmaxf(std::fmaxf(cohCount, aliCount), sepCount);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = v2f::direction(velocity, steer_direction); // steering direction
	steer = v2f::limit(steer, config->boid_max_steer);

	return steer;
}

void Boid::update_points()
{
	rotation = v2f::angle(velocity);

	origin = sf::Vector2f(
		position.x + (config->boid_size_width / 2.0f),
		position.y + (config->boid_size_height / 2.0f));

	pointA = v2f::rotate_point({ position.x + config->boid_size_width, position.y + (config->boid_size_height / 2) }, origin, rotation); // middle right tip
	pointB = v2f::rotate_point({ position.x							 , position.y								   }, origin, rotation); // top left corner
	pointC = v2f::rotate_point({ position.x							 , position.y + config->boid_size_height	   }, origin, rotation); // bot left corner
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	if (std::abs(weight) <= FLT_EPSILON)
		return;

	sf::Vector2f steer = v2f::normalize(v2f::direction(get_origin(), point), config->boid_max_speed); 
	apply_force(steer_at(steer) * weight);
}

sf::Vector2f Boid::outside_border(sf::Vector2f pos, const float& dt)
{
	switch (config->turn_at_border)
	{
	case true:
		return turn_at_border(pos, dt);
	default:
		return teleport_at_border(pos);
	}
}
sf::Vector2f Boid::turn_at_border(const sf::Vector2f& pos, const float& dt)
{
	float width_margin = border->width() - border->width() * config->turn_margin_factor;
	float height_margin = border->height() - border->height() * config->turn_margin_factor;

	float left_margin = border->left + width_margin;
	float top_margin = border->top + height_margin;
	float right_margin = border->right - width_margin;
	float bot_margin = border->bot - height_margin;

	if (pos.x + config->boid_size_width < left_margin)
		velocity.x += config->turn_factor * dt * (1.0f + std::powf(std::abs(pos.x - left_margin) / width_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (pos.x > right_margin)
		velocity.x -= config->turn_factor * dt * (1.0f + std::powf(std::abs(pos.x - right_margin) / width_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (pos.y + config->boid_size_height < top_margin)
		velocity.y += config->turn_factor * dt * (1.0f + std::powf(std::abs(pos.y - top_margin) / height_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (pos.y > bot_margin)
		velocity.y -= config->turn_factor * dt * (1.0f + std::powf(std::abs(pos.y - bot_margin) / height_margin, 2.0f)) * (1.0f / (density + 1.0f));

	return pos;
}
sf::Vector2f Boid::teleport_at_border(sf::Vector2f& pos)
{
	if (pos.x + config->boid_size_width * 1.25f < border->left)
		pos.x = (float)border->right;

	if (pos.x > border->right)
		pos.x = border->left - config->boid_size_width * 1.25f;

	if (pos.y + config->boid_size_height * 1.25f < border->top)
		pos.y = (float)border->bot;

	if (pos.y > border->bot)
		pos.y = border->top - config->boid_size_height * 1.25f;

	return pos;
}

void Boid::position_color()
{
	float t = position.x / border->width();
	float s = position.y / border->height();

	color = sf::Vector3f(
		(float)interpolate(config->boid_color_top_left.x * 255, config->boid_color_top_right.x * 255, config->boid_color_bot_left.x * 255, config->boid_color_bot_right.x * 255, t, s) / 255.0f,
		(float)interpolate(config->boid_color_top_left.y * 255, config->boid_color_top_right.y * 255, config->boid_color_bot_left.y * 255, config->boid_color_bot_right.y * 255, t, s) / 255.0f,
		(float)interpolate(config->boid_color_top_left.z * 255, config->boid_color_top_right.z * 255, config->boid_color_bot_left.z * 255, config->boid_color_bot_right.z * 255, t, s) / 255.0f);
}
void Boid::cycle_color(const float& dt)
{
	cycle_time = std::fmodf(cycle_time + dt * config->boid_cycle_colors_speed, 1.0f);

	float scaled_time = cycle_time * (float)(config->boid_cycle_colors.size() - 1);

	int index1 = (int)scaled_time;
	int index2 = ((int)scaled_time + 1) % config->boid_cycle_colors.size();

	sf::Vector3f color1 = config->boid_cycle_colors[index1];
	sf::Vector3f color2 = config->boid_cycle_colors[index2];

	float newT = scaled_time - std::floorf(scaled_time);

	color = v2f::lerp(color1, color2, newT);
}
void Boid::density_color(const float& dt)
{
	density_time = (config->boid_density_cycle_enabled) ? std::fmodf(density_time + dt * config->boid_density_cycle_speed, 1.0f) : 0.0f;

	float density_percentage = (density / (float)config->boid_density);

	float scaled_density = std::fmodf(density_percentage + density_time, 1.0f) * (float)(config->boid_density_colors.size() - 1);

	int index1 = (int)scaled_density;
	int index2 = ((int)scaled_density + 1) % config->boid_density_colors.size();

	sf::Vector3f color1 = config->boid_density_colors[index1];
	sf::Vector3f color2 = config->boid_density_colors[index2];

	float newT = scaled_density - std::floorf(scaled_density);

	color = v2f::lerp(color1, color2, newT);
}
void Boid::audio_color(const float& dt)
{
	float density_percentage = (density / (float)config->audio_responsive_density);
	float max_volume = std::fminf(audio_meter->get_volume() * config->audio_responsive_strength, config->audio_responsive_limit);

	float scaled_volume = std::fminf(max_volume * density_percentage, 1.0f) * (float)(config->audio_responsive_colors.size() - 1);

	int index1 = (int)scaled_volume;
	int index2 = ((int)scaled_volume + 1) % config->audio_responsive_colors.size();

	sf::Vector3f color1 = config->audio_responsive_colors[index1];
	sf::Vector3f color2 = config->audio_responsive_colors[index2];

	float newT = scaled_volume - std::floorf(scaled_volume);

	color = v2f::lerp(color1, color2, newT);
}
void Boid::impulse_color(const std::vector<Impulse>& impulses)
{
	for (const Impulse& impulse : impulses)
	{
		sf::Vector2f impulse_pos = impulse.get_position();
		float impulse_length = impulse.get_length();

		float length = v2f::length(v2f::direction(impulse_pos, position));
		float diff = std::abs(length - impulse_length);

		float percentage = (impulse_length / config->impulse_fade_distance);
		float size = impulse.get_size() * (1.0f - percentage);

		float scaled_length = std::fmodf(percentage, 1.0f) * (float)(config->impulse_colors.size() - 1);

		int index1 = (int)scaled_length;
		int index2 = ((int)scaled_length + 1) % config->impulse_colors.size();

		sf::Vector3f color1 = config->impulse_colors[index1];
		sf::Vector3f color2 = config->impulse_colors[index2];

		float newT = scaled_length - std::floorf(scaled_length);

		if (diff <= size)
		{
			color = v2f::lerp(color1, color2, newT);
			return;
		}
	}
}

void Boid::update_grid_cells(const std::vector<Boid>& boids) const
{
	int index = static_cast<int>(this - boids.data());

	if (index == 0)
	{
		grid->cellsStartIndices[cell_index] = index;
		return;
	}

	if (index == config->boid_count - 1)
		grid->cellsEndIndices[cell_index] = index;

	int otherIndex = boids[index - 1].get_cell_index();

	if (otherIndex != cell_index)
	{
		grid->cellsStartIndices[cell_index] = index;
		grid->cellsEndIndices[otherIndex] = index - 1;
	}
}
