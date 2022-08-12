#include "Boid.h"

Boid::Boid(Grid& grid, Config& config, const AudioMeter& audio_meter, const Rect_i& border, const sf::Vector2f& pos)
	: grid(&grid), config(&config), audio_meter(&audio_meter), border(&border), position(pos)
{
	velocity = v2f::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), config.boid_max_speed);

	if (config.boid_cycle_colors_random)
		cycle_time = util::random(0.0f, 1.0f);
}

void Boid::update(const std::vector<Boid>& boids, const std::vector<Impulse>& impulses, const float& dt)
{
	prev_position = position;
	prev_velocity = velocity;

	flock(boids);

	position += velocity * dt;

	if (outside_border(dt))
		prev_position = position;

	// draw-info
	{
		color = sf::Vector3f();

		if (config->color_flags & ColorFlags::Positional)
			color += position_color() * config->color_positional_weight;
		if (config->color_flags & ColorFlags::Cycle)
			color += cycle_color(dt) * config->color_cycle_weight;
		if (config->color_flags & ColorFlags::Density)
			color += density_color(dt) * config->color_density_weight;
		if (config->color_flags & ColorFlags::Velocity)
			color += velocity_color() * config->color_velocity_weight;
		if (config->color_flags & ColorFlags::Rotation)
			color += rotation_color() * config->color_rotation_weight;
		if (config->color_flags & ColorFlags::Audio)
			color += audio_color(dt) * config->color_audio_weight;

		color = impulse_color(impulses);
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

	float min_distance = std::fminf(config->coh_distance, config->ali_distance);
	float length = v2f::length(velocity);
	sf::Vector2f origin = get_origin();

	for (int i = 0; i < neighbours; ++i)
	{
		int gridCellIndex = neighbourIndices[i];
		for (int j = grid->cellsStartIndices[gridCellIndex]; j <= grid->cellsEndIndices[gridCellIndex] && j > -1; ++j) // do in one loop
		{
			const Boid* b = &boids[j];

			if (b == this)
				continue;

			sf::Vector2f other_origin = b->get_origin();
			float distance = v2f::distance_squared(origin, other_origin);

			if (distance <= FLT_EPSILON)
				continue;

			sf::Vector2f dir = v2f::direction(origin, other_origin);

			if (distance <= min_distance)
			{
				float angle = v2f::angle(velocity, dir, length, v2f::length(dir));

				if (angle <= config->boid_view_angle)
				{
					if (distance <= config->coh_distance)
					{
						coh += other_origin;	  // Head towards center of boids
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
		coh = v2f::direction(get_origin(), coh / (float)cohCount);
		coh = v2f::normalize(coh, config->boid_max_speed);

		apply_force(steer_at(coh) * config->coh_weight);
	}

	{
		float length = v2f::length(velocity);

		if (length > config->boid_max_speed)
			velocity = v2f::normalize(velocity, config->boid_max_speed);
		else if (length < config->boid_min_speed)
			velocity = v2f::normalize(velocity, config->boid_min_speed);
	}

	density = std::max(std::max(cohCount, aliCount), sepCount);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction)
{
	sf::Vector2f steer = v2f::direction(velocity, steer_direction); // steering direction
	steer = v2f::limit(steer, config->boid_max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	if (std::abs(weight) <= FLT_EPSILON)
		return;

	sf::Vector2f steer = v2f::normalize(v2f::direction(get_origin(), point), config->boid_max_speed); 
	apply_force(steer_at(steer) * weight);
}

bool Boid::outside_border(const float& dt)
{
	switch (config->turn_at_border)
	{
	case true:
		return turn_at_border(dt);
	default:
		return teleport_at_border();
	}
}
bool Boid::turn_at_border(const float& dt)
{
	float width_margin = border->width() - border->width() * config->turn_margin_factor;
	float height_margin = border->height() - border->height() * config->turn_margin_factor;

	float left_margin = border->left + width_margin;
	float top_margin = border->top + height_margin;
	float right_margin = border->right - width_margin;
	float bot_margin = border->bot - height_margin;

	if (position.x + config->boid_size_width < left_margin)
		velocity.x += config->turn_factor * dt * (1.0f + std::powf(std::abs(position.x - left_margin) / width_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (position.x > right_margin)
		velocity.x -= config->turn_factor * dt * (1.0f + std::powf(std::abs(position.x - right_margin) / width_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (position.y + config->boid_size_height < top_margin)
		velocity.y += config->turn_factor * dt * (1.0f + std::powf(std::abs(position.y - top_margin) / height_margin, 2.0f)) * (1.0f / (density + 1.0f));

	if (position.y > bot_margin)
		velocity.y -= config->turn_factor * dt * (1.0f + std::powf(std::abs(position.y - bot_margin) / height_margin, 2.0f)) * (1.0f / (density + 1.0f));

	return false;
}
bool Boid::teleport_at_border()
{
	sf::Vector2f current = position;

	if (position.x + config->boid_size_width * 1.25f < border->left)
		position.x = (float)border->right;

	if (position.x > border->right)
		position.x = border->left - config->boid_size_width * 1.25f;

	if (position.y + config->boid_size_height * 1.25f < border->top)
		position.y = (float)border->bot;

	if (position.y > border->bot)
		position.y = border->top - config->boid_size_height * 1.25f;

	if (current != position)
		return true;

	return false;
}

sf::Vector3f Boid::position_color() const
{
	float t = position.x / border->width();
	float s = position.y / border->height();

	return sf::Vector3f(
		util::interpolate(config->boid_color_top_left.x * 255, config->boid_color_top_right.x * 255, config->boid_color_bot_left.x * 255, config->boid_color_bot_right.x * 255, t, s) / 255.0f,
		util::interpolate(config->boid_color_top_left.y * 255, config->boid_color_top_right.y * 255, config->boid_color_bot_left.y * 255, config->boid_color_bot_right.y * 255, t, s) / 255.0f,
		util::interpolate(config->boid_color_top_left.z * 255, config->boid_color_top_right.z * 255, config->boid_color_bot_left.z * 255, config->boid_color_bot_right.z * 255, t, s) / 255.0f);
}
sf::Vector3f Boid::cycle_color(const float& dt)
{
	if (!config->boid_cycle_colors.size())
		return sf::Vector3f();

	cycle_time = std::fmodf(cycle_time + dt * config->boid_cycle_colors_speed, 1.0f);

	float scaled_time = cycle_time * (float)(config->boid_cycle_colors.size() - 1);

	int index1 = (int)scaled_time;
	int index2 = ((int)scaled_time + 1) % config->boid_cycle_colors.size();

	sf::Vector3f color1 = config->boid_cycle_colors[index1];
	sf::Vector3f color2 = config->boid_cycle_colors[index2];

	float newT = scaled_time - std::floorf(scaled_time);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::density_color(const float& dt)
{
	if (!config->boid_density_colors.size())
		return sf::Vector3f();

	density_time = (config->boid_density_cycle_enabled) ? std::fmodf(density_time + dt * config->boid_density_cycle_speed, 1.0f) : 0.0f;

	float density_percentage = (density / (float)config->boid_density);

	float scaled_density = std::fmodf(density_percentage + density_time, 1.0f) * (float)(config->boid_density_colors.size() - 1);

	int index1 = (int)scaled_density;
	int index2 = ((int)scaled_density + 1) % config->boid_density_colors.size();

	sf::Vector3f color1 = config->boid_density_colors[index1];
	sf::Vector3f color2 = config->boid_density_colors[index2];

	float newT = scaled_density - std::floorf(scaled_density);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::velocity_color() const
{
	if (!config->boid_velocity_colors.size())
		return sf::Vector3f();

	float velocity_percentage = v2f::length(velocity) / config->boid_max_speed;

	float scaled_velocity = velocity_percentage * (float)(config->boid_velocity_colors.size() - 1);

	int index1 = (int)scaled_velocity;
	int index2 = ((int)scaled_velocity + 1) % config->boid_velocity_colors.size();

	sf::Vector3f color1 = config->boid_velocity_colors[index1];
	sf::Vector3f color2 = config->boid_velocity_colors[index2];

	float newT = scaled_velocity - std::floorf(scaled_velocity);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::rotation_color() const
{
	if (!config->boid_rotation_colors.size())
		return sf::Vector3f();

	float rotation_percentage = (v2f::angle(velocity) + float(M_PI)) / (2.0f * float(M_PI));

	float scaled_rotation = rotation_percentage * (float)(config->boid_rotation_colors.size() - 1);

	int index1 = (int)scaled_rotation;
	int index2 = ((int)scaled_rotation + 1) % config->boid_rotation_colors.size();

	sf::Vector3f color1 = config->boid_rotation_colors[index1];
	sf::Vector3f color2 = config->boid_rotation_colors[index2];

	float newT = scaled_rotation - std::floorf(scaled_rotation);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::audio_color(const float& dt)
{
	if (!config->audio_responsive_colors.size())
		return sf::Vector3f();

	float density_percentage = (density / (float)config->audio_responsive_density);
	float max_volume = std::fminf(audio_meter->get_volume() * config->audio_responsive_strength, config->audio_responsive_limit);

	float scaled_volume = std::fminf(max_volume * density_percentage, 1.0f) * (float)(config->audio_responsive_colors.size() - 1);

	int index1 = (int)scaled_volume;
	int index2 = ((int)scaled_volume + 1) % config->audio_responsive_colors.size();

	sf::Vector3f color1 = config->audio_responsive_colors[index1];
	sf::Vector3f color2 = config->audio_responsive_colors[index2];

	float newT = scaled_volume - std::floorf(scaled_volume);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::impulse_color(const std::vector<Impulse>& impulses)
{
	if (!config->impulse_colors.size())
		return sf::Vector3f();

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
			return v2f::lerp(color1, color2, newT);;
	}

	return color;
}

void Boid::update_grid_cells(const std::vector<Boid>& boids) const
{
	int index = std::distance(boids.data(), this);

	if (index == 0)
	{
		grid->cellsStartIndices[cell_index] = index;
		return;
	}

	if (index == config->boid_count - 1)
		grid->cellsEndIndices[cell_index] = index;

	int other_index = boids[index - 1].get_cell_index();

	if (other_index != cell_index)
	{
		grid->cellsStartIndices[cell_index] = index;
		grid->cellsEndIndices[other_index] = index - 1;
	}
}
