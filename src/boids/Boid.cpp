#include "Boid.h"

Boid::Boid(Grid& grid, Config& config, const AudioMeter& audio_meter, const RectInt& border, const sf::Vector2f& pos)
	: _grid(&grid), _config(&config), _audio_meter(&audio_meter), _border(&border), _position(pos)
{
	_velocity = v2f::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), config.boid_max_speed);

	if (config.boid_cycle_colors_random)
		_cycle_time = util::random(0.0f, 1.0f);
}

void Boid::update(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids, const std::vector<Impulse>& impulses, float dt)
{
	_prev_position = _position;
	_prev_velocity = _velocity;

	flock(boids, sorted_boids);

	_position += _velocity * dt;

	if (outside_border(dt))
		_prev_position = _position;

	// draw-info
	{
		_color = sf::Vector3f();

		if (_config->color_flags & ColorFlags::Positional)
			_color += position_color() * _config->color_positional_weight;
		if (_config->color_flags & ColorFlags::Cycle)
			_color += cycle_color(dt) * _config->color_cycle_weight;
		if (_config->color_flags & ColorFlags::Density)
			_color += density_color(dt) * _config->color_density_weight;
		if (_config->color_flags & ColorFlags::Velocity)
			_color += velocity_color() * _config->color_velocity_weight;
		if (_config->color_flags & ColorFlags::Rotation)
			_color += rotation_color() * _config->color_rotation_weight;
		if (_config->color_flags & ColorFlags::Audio)
			_color += audio_color(dt) * _config->color_audio_weight;

		_color = impulse_color(impulses);
	}
}

void Boid::flock(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids)
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	int sepCount = 0;
	int aliCount = 0;
	int cohCount = 0;

	const sf::Vector2f origin = get_saved_origin();
	const float min_distance = std::fminf(_config->coh_distance, _config->ali_distance);
	const float length = v2f::length(_saved_velocity);

	const int neighbours = 4;

	int neighbour_indicies[neighbours];
	sf::Vector2i neighbour[neighbours];

	const sf::Vector2f grid_cell_raw = _grid->relative_pos(origin);
	const sf::Vector2i grid_cell = sf::Vector2i(grid_cell_raw);
	const sf::Vector2f grid_cell_overflow = grid_cell_raw - sf::Vector2f(grid_cell);

	const sf::Vector2f relative_pos = grid_cell_overflow * sf::Vector2f(_grid->_cont_dims);

	const int x = (grid_cell_overflow.x > 0.5f ? 1 : -1);
	const int y = (grid_cell_overflow.y > 0.5f ? 1 : -1);

	const int x_neighbor = grid_cell.x + x;
	const int y_neighbor = grid_cell.y + y;

	neighbour[0] = sf::Vector2i(0, 0);
	neighbour[1] = sf::Vector2i(x, 0);
	neighbour[2] = sf::Vector2i(0, y);
	neighbour[3] = sf::Vector2i(x, y);

	neighbour_indicies[0] = _grid->at_pos(grid_cell.x, grid_cell.y);	// current
	neighbour_indicies[1] = _grid->at_pos(x_neighbor, grid_cell.y);	// left or right of current
	neighbour_indicies[2] = _grid->at_pos(grid_cell.x, y_neighbor);	// top or bot of current
	neighbour_indicies[3] = _grid->at_pos(x_neighbor, y_neighbor);	// top left/right bot left/right of current

	for (int i = 0; i < neighbours; ++i)
	{
		int grid_cell_index = neighbour_indicies[i];
		sf::Vector2i neighbour_cell = neighbour[i];

		for (int j = _grid->_cells_start_indices[grid_cell_index]; j <= _grid->_cells_end_indices[grid_cell_index] && j > -1; ++j) // do in one loop
		{
			const Boid* b = &boids[sorted_boids[j]];

			if (b == this)
				continue;

			const sf::Vector2f other_origin = b->get_saved_origin();

			const sf::Vector2f other_grid_cell_raw = _grid->relative_pos(other_origin);
			const sf::Vector2i other_grid_cell = sf::Vector2i(other_grid_cell_raw);
			const sf::Vector2f other_grid_cell_overflow = other_grid_cell_raw - sf::Vector2f(other_grid_cell);

			sf::Vector2f other_relative_pos = other_grid_cell_overflow * sf::Vector2f(_grid->_cont_dims); // other _grid cell should be relative

			other_relative_pos.x += (_grid->_cont_dims.x * neighbour_cell.x);
			other_relative_pos.y += (_grid->_cont_dims.y * neighbour_cell.y);

			const sf::Vector2f dir = v2f::direction(relative_pos, other_relative_pos);
			const float distance = v2f::length_sq(dir);

			if (distance <= FLT_EPSILON)
				continue;

			if (distance <= min_distance)
			{
				float angle = v2f::angle(_saved_velocity, dir, length, v2f::length(dir));

				if (angle <= _config->boid_view_angle)
				{
					if (distance <= _config->coh_distance)
					{
						coh += origin + dir; // Head towards center of boids
						++cohCount;
					}
					if (distance <= _config->ali_distance)
					{
						ali += b->get_saved_velocity(); // Align with every boids _velocity
						++aliCount;
					}
				}
			}
			if (distance <= _config->sep_distance)
			{
				sep += -dir / distance;
				++sepCount;
			}
		}
	}

	_density = std::max(std::max(cohCount, aliCount), sepCount);

	if (sepCount > 0) // separation
	{
		sep = v2f::normalize(sep / (float)sepCount, _config->boid_max_speed);
		_velocity += steer_at(sep) * _config->sep_weight;
	}
	if (aliCount > 0) // alignment
	{
		ali = v2f::normalize(ali / (float)aliCount, _config->boid_max_speed);
		_velocity += steer_at(ali) * _config->ali_weight;
	}
	if (cohCount > 0) // cohesion
	{
		coh = v2f::direction(origin, coh / (float)cohCount);
		coh = v2f::normalize(coh, _config->boid_max_speed);

		_velocity += steer_at(coh) * _config->coh_weight;
	}

	_velocity = v2f::clamp(_velocity, _config->boid_max_speed, _config->boid_min_speed);
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction) const
{
	sf::Vector2f steer = v2f::direction(_velocity, steer_direction); // steering direction
	steer = v2f::limit(steer, _config->boid_max_steer);

	return steer;
}

void Boid::steer_towards(sf::Vector2f point, float weight)
{
	if (std::abs(weight) <= FLT_EPSILON)
		return;

	sf::Vector2f steer = v2f::normalize(v2f::direction(get_origin(), point), _config->boid_max_speed); 
	_velocity += steer_at(steer) * weight;
}

bool Boid::outside_border(float dt)
{
	switch (_config->turn_at_border)
	{
	case true:
		return turn_at_border(dt);
	default:
		return teleport_at_border();
	}
}
bool Boid::turn_at_border(float dt)
{
	const float width_margin = _border->width() - _border->width() * _config->turn_margin_factor;
	const float height_margin = _border->height() - _border->height() * _config->turn_margin_factor;

	const float left_margin = _border->left + width_margin;
	const float top_margin = _border->top + height_margin;
	const float right_margin = _border->right - width_margin;
	const float bot_margin = _border->bot - height_margin;

	if (_position.x + _config->boid_size_width < left_margin)
		_velocity.x += _config->turn_factor * dt * (1.0f + std::powf(std::abs(_position.x - left_margin) / width_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.x > right_margin)
		_velocity.x -= _config->turn_factor * dt * (1.0f + std::powf(std::abs(_position.x - right_margin) / width_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.y + _config->boid_size_height < top_margin)
		_velocity.y += _config->turn_factor * dt * (1.0f + std::powf(std::abs(_position.y - top_margin) / height_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.y > bot_margin)
		_velocity.y -= _config->turn_factor * dt * (1.0f + std::powf(std::abs(_position.y - bot_margin) / height_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	return false;
}
bool Boid::teleport_at_border()
{
	const sf::Vector2f current = _position;

	if (_position.x + _config->boid_size_width < _border->left)
		_position.x = (float)_border->right;

	if (_position.x > _border->right)
		_position.x = _border->left - _config->boid_size_width;

	if (_position.y + _config->boid_size_height < _border->top)
		_position.y = (float)_border->bot;

	if (_position.y > _border->bot)
		_position.y = _border->top - _config->boid_size_height;

	return (current != _position);
}

sf::Vector3f Boid::position_color() const
{
	const float t = _position.x / _border->width();
	const float s = _position.y / _border->height();

	return sf::Vector3f(
		util::interpolate(_config->boid_color_top_left.x * 255, _config->boid_color_top_right.x * 255, _config->boid_color_bot_left.x * 255, _config->boid_color_bot_right.x * 255, t, s) / 255.0f,
		util::interpolate(_config->boid_color_top_left.y * 255, _config->boid_color_top_right.y * 255, _config->boid_color_bot_left.y * 255, _config->boid_color_bot_right.y * 255, t, s) / 255.0f,
		util::interpolate(_config->boid_color_top_left.z * 255, _config->boid_color_top_right.z * 255, _config->boid_color_bot_left.z * 255, _config->boid_color_bot_right.z * 255, t, s) / 255.0f);
}
sf::Vector3f Boid::cycle_color(float dt)
{
	if (!_config->boid_cycle_colors.size())
		return sf::Vector3f();

	_cycle_time = std::fmodf(_cycle_time + dt * _config->boid_cycle_colors_speed, 1.0f);

	float scaled_time = _cycle_time * (float)(_config->boid_cycle_colors.size() - 1);

	const int index1 = (int)scaled_time;
	const int index2 = ((int)scaled_time + 1) % _config->boid_cycle_colors.size();

	const sf::Vector3f color1 = _config->boid_cycle_colors[index1];
	const sf::Vector3f color2 = _config->boid_cycle_colors[index2];

	const float newT = scaled_time - std::floorf(scaled_time);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::density_color(float dt)
{
	if (!_config->boid_density_colors.size())
		return sf::Vector3f();

	_density_time = (_config->boid_density_cycle_enabled) ? std::fmodf(_density_time + dt * _config->boid_density_cycle_speed, 1.0f) : 0.0f;

	const float density_percentage = (_density / (float)_config->boid_density);

	const float scaled_density = std::fmodf(density_percentage + _density_time, 1.0f) * (float)(_config->boid_density_colors.size() - 1);

	const int index1 = (int)scaled_density;
	const int index2 = ((int)scaled_density + 1) % _config->boid_density_colors.size();

	const sf::Vector3f color1 = _config->boid_density_colors[index1];
	const sf::Vector3f color2 = _config->boid_density_colors[index2];

	const float newT = scaled_density - std::floorf(scaled_density);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::velocity_color() const
{
	if (!_config->boid_velocity_colors.size())
		return sf::Vector3f();

	const float velocity_percentage = util::clamp((v2f::length(_velocity) - _config->boid_min_speed) / (_config->boid_max_speed - _config->boid_min_speed), 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(_config->boid_velocity_colors.size() - 1);

	const int index1 = (int)scaled_velocity;
	const int index2 = ((int)scaled_velocity + 1) % _config->boid_velocity_colors.size();

	const sf::Vector3f color1 = _config->boid_velocity_colors[index1];
	const sf::Vector3f color2 = _config->boid_velocity_colors[index2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::rotation_color() const
{
	if (!_config->boid_rotation_colors.size())
		return sf::Vector3f();

	const float rotation_percentage = (v2f::angle(_velocity) + float(M_PI)) / (2.0f * float(M_PI));

	const float scaled_rotation = rotation_percentage * (float)(_config->boid_rotation_colors.size() - 1);

	const int index1 = (int)scaled_rotation;
	const int index2 = ((int)scaled_rotation + 1) % _config->boid_rotation_colors.size();

	const sf::Vector3f color1 = _config->boid_rotation_colors[index1];
	const sf::Vector3f color2 = _config->boid_rotation_colors[index2];

	const float newT = scaled_rotation - std::floorf(scaled_rotation);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::audio_color(float dt) const
{
	if (!_config->audio_responsive_colors.size())
		return sf::Vector3f();

	const float density_percentage = (_density / (float)_config->audio_responsive_density);
	const float max_volume = std::fminf(_audio_meter->get_volume() * _config->audio_responsive_strength, _config->audio_responsive_limit);

	const float scaled_volume = std::fminf(max_volume * density_percentage, 1.0f) * (float)(_config->audio_responsive_colors.size() - 1);

	const int index1 = (int)scaled_volume;
	const int index2 = ((int)scaled_volume + 1) % _config->audio_responsive_colors.size();

	const sf::Vector3f color1 = _config->audio_responsive_colors[index1];
	const sf::Vector3f color2 = _config->audio_responsive_colors[index2];

	const float newT = scaled_volume - std::floorf(scaled_volume);

	return v2f::lerp(color1, color2, newT);
}
sf::Vector3f Boid::impulse_color(const std::vector<Impulse>& impulses) const
{
	if (!_config->impulse_colors.size())
		return sf::Vector3f();

	for (const Impulse& impulse : impulses)
	{
		sf::Vector2f impulse_pos = impulse.get_position();
		const float impulse_length = impulse.get_length();

		const float length = v2f::length(v2f::direction(impulse_pos, _position));
		const float diff = std::abs(length - impulse_length);

		const float percentage = (impulse_length / _config->impulse_fade_distance);
		const float size = impulse.get_size() * (1.0f - percentage);

		if (diff <= size)
		{
			const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(_config->impulse_colors.size() - 1);

			const int index1 = (int)scaled_length;
			const int index2 = ((int)scaled_length + 1) % _config->impulse_colors.size();

			const sf::Vector3f color1 = _config->impulse_colors[index1];
			const sf::Vector3f color2 = _config->impulse_colors[index2];

			const float newT = scaled_length - std::floorf(scaled_length);

			return v2f::lerp(color1, color2, newT);
		}
	}

	return _color;
}

void Boid::update_grid_cells(const std::vector<Boid>& boids, const std::vector<int>& sorted_boids, int index) const
{
	if (index == 0)
	{
		_grid->_cells_start_indices[_cell_index] = index;
		return;
	}

	if (index == _config->boid_count - 1)
		_grid->_cells_end_indices[_cell_index] = index;

	const int other_index = boids[sorted_boids[index - 1]].get_cell_index();

	if (other_index != _cell_index)
	{
		_grid->_cells_start_indices[_cell_index] = index;
		_grid->_cells_end_indices[other_index] = index - 1;
	}
}
