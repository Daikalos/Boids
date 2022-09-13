#include "Boid.h"
#include "Wrapper.h"

Boid::Boid(Config& config, const sf::Vector2f& pos)
	:  _config(&config), _position(pos)
{
	_velocity = vu::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), _config->boid_max_speed);

	if (_config->boid_cycle_colors_random)
		_cycle_time = util::random(0.0f, 1.0f);
}

void Boid::pre_update(const Grid& grid) noexcept
{
	_prev_position = _position;
	_prev_velocity = _velocity;

	const sf::Vector2f grid_cell_raw = grid.relative_pos(get_origin());
	const sf::Vector2i grid_cell = sf::Vector2i(grid_cell_raw);
	const sf::Vector2f grid_cell_overflow = grid_cell_raw - sf::Vector2f(grid_cell);

	_relative_pos = grid_cell_overflow * grid._cont_dims;
	_cell_index = grid.at_pos(grid_cell);
}

void Boid::update_grid_cells(Grid& grid, std::span<Wrapper> boids, const int index) const
{
	if (index == 0)
	{
		grid._cells_start_indices[_cell_index] = index;
		return;
	}

	if (index == _config->boid_count - 1)
		grid._cells_end_indices[_cell_index] = index;

	const int other_index = boids[index - 1].boid->get_cell_index();

	if (other_index != _cell_index)
	{
		grid._cells_start_indices[_cell_index] = index;
		grid._cells_end_indices[other_index] = index - 1;
	}
}

void Boid::flock(const Grid& grid, std::span<Wrapper> boids)
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	int sep_count = 0;
	int ali_count = 0;
	int coh_count = 0;

	const sf::Vector2f origin = get_origin();
	const float vel_length = vu::distance(_velocity);

	const int neighbours = 4; // max 4 neighbours at a time

	int neighbour_indicies[neighbours] {0};
	sf::Vector2f neighbour[neighbours];

	const sf::Vector2f grid_cell_raw = grid.relative_pos(origin);
	const sf::Vector2i grid_cell = sf::Vector2i(grid_cell_raw);
	const sf::Vector2f grid_cell_overflow = grid_cell_raw - sf::Vector2f(grid_cell);

	const int x = (grid_cell_overflow.x > 0.5f ? 1 : -1);
	const int y = (grid_cell_overflow.y > 0.5f ? 1 : -1);

	const int x_neighbor = grid_cell.x + x;
	const int y_neighbor = grid_cell.y + y;

	neighbour[0] = grid._cont_dims * sf::Vector2f(0, 0);
	neighbour[1] = grid._cont_dims * sf::Vector2f(x, 0);
	neighbour[2] = grid._cont_dims * sf::Vector2f(0, y);
	neighbour[3] = grid._cont_dims * sf::Vector2f(x, y);

	neighbour_indicies[0] = grid.at_pos(grid_cell.x, grid_cell.y);	// current
	neighbour_indicies[1] = grid.at_pos(x_neighbor, grid_cell.y);		// left or right of current
	neighbour_indicies[2] = grid.at_pos(grid_cell.x, y_neighbor);		// top or bot of current
	neighbour_indicies[3] = grid.at_pos(x_neighbor, y_neighbor);		// top left/right bot left/right of current

	for (int i = 0; i < neighbours; ++i)
	{
		const int grid_cell_index = neighbour_indicies[i];
		const int start = grid._cells_start_indices[grid_cell_index];

		if (start < 0)
			continue;

		const sf::Vector2f neighbour_cell = neighbour[i];
		const int end = grid._cells_end_indices[grid_cell_index];

		for (int j = start; j <= end; ++j) // do in one loop
		{
			const Boid& b = *boids[j].boid;

			if (&b == this)
				continue;

			const sf::Vector2f other_relative_pos = 
				neighbour_cell + b.get_relative_position(); // need to get relative to this boid

			const sf::Vector2f dir	= vu::direction(_relative_pos, other_relative_pos);
			const float distance_sq = std::fmaxf(vu::distance_sq(dir), FLT_EPSILON);
			const float angle		= vu::angle(_prev_velocity, dir, vel_length, std::sqrtf(distance_sq));

			if (angle <= _config->boid_view_angle) // angle only effects cohesion and alignment
			{
				if (distance_sq <= _config->coh_distance)
				{
					coh += origin + dir; // Head towards center of boids
					++coh_count;
				}
				if (distance_sq <= _config->ali_distance)
				{
					ali += b.get_prev_velocity(); // Align with every boids velocity
					++ali_count;
				}
			}

			if (distance_sq <= _config->sep_distance)
			{
				sep += -dir / distance_sq;
				++sep_count;
			}
		}
	}

	if (coh_count) coh = vu::normalize(vu::direction(origin, coh / (float)coh_count), _config->boid_max_speed);
	if (ali_count) ali = vu::normalize(ali / (float)ali_count, _config->boid_max_speed);
	if (sep_count) sep = vu::normalize(sep / (float)sep_count, _config->boid_max_speed);

	_velocity +=
		steer_at(coh) * _config->coh_weight +
		steer_at(ali) * _config->ali_weight +
		steer_at(sep) * _config->sep_weight;

	_density = std::max(std::max(coh_count, ali_count), sep_count);
}

void Boid::update(const RectInt& border, const AudioMeterInfoBase::ptr& audio_meter, std::span<const Impulse> impulses, float dt)
{
	_velocity = vu::clamp(_velocity, _config->boid_min_speed, _config->boid_max_speed);

	_position += _velocity * dt;

	if (outside_border(border, dt))
		_prev_position = _position;

	// draw-info
	{
		_color = sf::Vector3f();

		if (_config->color_flags & ColorFlags::Positional)
			_color += position_color(border) * _config->color_positional_weight;
		if (_config->color_flags & ColorFlags::Cycle)
			_color += cycle_color(dt) * _config->color_cycle_weight;
		if (_config->color_flags & ColorFlags::Density)
			_color += density_color(dt) * _config->color_density_weight;
		if (_config->color_flags & ColorFlags::Velocity)
			_color += velocity_color() * _config->color_velocity_weight;
		if (_config->color_flags & ColorFlags::Rotation)
			_color += rotation_color() * _config->color_rotation_weight;
		if (_config->color_flags & ColorFlags::Audio)
			_color += audio_color(audio_meter, dt) * _config->color_audio_weight;

		_color = impulse_color(impulses);
	}
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction) const
{
	return vu::limit(vu::direction(_velocity, steer_direction), _config->boid_max_steer);
}

void Boid::steer_towards(const sf::Vector2f& direction, float length, float weight)
{
	if (std::fabsf(weight) <= FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::normalize(direction, length, _config->boid_max_speed);

	_velocity += steer_at(steer) * weight;
}
void Boid::steer_towards(const sf::Vector2f& point, float weight)
{
	steer_towards(point, vu::distance(point), weight);
}

bool Boid::outside_border(const RectInt& border, float dt)
{
	return _config->turn_at_border ? turn_at_border(border, dt) : teleport_at_border(border);
}
bool Boid::turn_at_border(const RectInt& border, float dt)
{
	const float width_margin = border.width() - border.width() * _config->turn_margin_factor;
	const float height_margin = border.height() - border.height() * _config->turn_margin_factor;

	const float left_margin = border.left + width_margin;
	const float top_margin = border.top + height_margin;
	const float right_margin = border.right - width_margin;
	const float bot_margin = border.bot - height_margin;

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
bool Boid::teleport_at_border(const RectInt& border)
{
	const sf::Vector2f current = _position;

	if (_position.x + _config->boid_size_width < border.left)
		_position.x = (float)border.right;

	if (_position.x > border.right)
		_position.x = border.left - _config->boid_size_width;

	if (_position.y + _config->boid_size_height < border.top)
		_position.y = (float)border.bot;

	if (_position.y > border.bot)
		_position.y = border.top - _config->boid_size_height;

	return (current != _position);
}

sf::Vector3f Boid::position_color(const RectInt& border) const
{
	const float t = _position.x / border.width();
	const float s = _position.y / border.height();

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

	return vu::lerp(color1, color2, newT);
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

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::velocity_color() const
{
	if (!_config->boid_velocity_colors.size())
		return sf::Vector3f();

	const float velocity_percentage = std::clamp((vu::distance(_velocity) - _config->boid_min_speed) / (_config->boid_max_speed - _config->boid_min_speed), 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(_config->boid_velocity_colors.size() - 1);

	const int index1 = (int)scaled_velocity;
	const int index2 = ((int)scaled_velocity + 1) % _config->boid_velocity_colors.size();

	const sf::Vector3f color1 = _config->boid_velocity_colors[index1];
	const sf::Vector3f color2 = _config->boid_velocity_colors[index2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::rotation_color() const
{
	if (!_config->boid_rotation_colors.size())
		return sf::Vector3f();

	const float rotation_percentage = (vu::angle(_velocity) + float(M_PI)) / (2.0f * float(M_PI));

	const float scaled_rotation = rotation_percentage * (float)(_config->boid_rotation_colors.size() - 1);

	const int index1 = (int)scaled_rotation;
	const int index2 = ((int)scaled_rotation + 1) % _config->boid_rotation_colors.size();

	const sf::Vector3f color1 = _config->boid_rotation_colors[index1];
	const sf::Vector3f color2 = _config->boid_rotation_colors[index2];

	const float newT = scaled_rotation - std::floorf(scaled_rotation);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::audio_color(const AudioMeterInfoBase::ptr& audio_meter, float dt) const
{
	if (!_config->audio_responsive_colors.size())
		return sf::Vector3f();

	const float density_percentage = (_density / (float)_config->audio_responsive_density);
	const float max_volume = std::fminf(audio_meter->get_volume() * _config->audio_responsive_strength, _config->audio_responsive_limit);

	const float scaled_volume = std::fminf(max_volume * density_percentage, 1.0f) * (float)(_config->audio_responsive_colors.size() - 1);

	const int index1 = (int)scaled_volume;
	const int index2 = ((int)scaled_volume + 1) % _config->audio_responsive_colors.size();

	const sf::Vector3f color1 = _config->audio_responsive_colors[index1];
	const sf::Vector3f color2 = _config->audio_responsive_colors[index2];

	const float newT = scaled_volume - std::floorf(scaled_volume);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::impulse_color(std::span<const Impulse> impulses) const
{
	if (!_config->impulse_colors.size())
		return sf::Vector3f();

	for (const Impulse& impulse : impulses)
	{
		const sf::Vector2f impulse_pos = impulse.get_position();
		const float impulse_length = impulse.get_length();

		const float length = vu::distance(_position, impulse_pos);
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

			return vu::lerp(color1, color2, newT);
		}
	}

	return _color;
}
