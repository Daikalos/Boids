#include "Boid.h"

Boid::Boid(const sf::Vector2f& pos)
	: _position(pos), _prev_position(pos)
{
	_prev_velocity = _velocity = vu::normalize(sf::Vector2f(
		util::random(-1.0f, 1.0f),
		util::random(-1.0f, 1.0f)), Config::GetInstance().boid_max_speed);

	if (Config::GetInstance().boid_cycle_colors_random)
		_cycle_time = util::random(0.0f, 1.0f);
}

Boid::Boid(const sf::Vector2f& pos, const sf::Vector2f& velocity)
	: _position(pos), _prev_position(pos), _velocity(velocity), _prev_velocity(velocity)
{
	if (Config::GetInstance().boid_cycle_colors_random)
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

void Boid::update_grid_cells(Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy, const uint32_t index) const
{
	if (index == 0)
	{
		grid._cells_start_indices[_cell_index] = index;
		return;
	}

	if (index == boids.size() - 1)
		grid._cells_end_indices[_cell_index] = index;

	const int other_index = boids[proxy[index - 1]].get_cell_index();

	if (other_index != _cell_index)
	{
		grid._cells_start_indices[_cell_index] = index;
		grid._cells_end_indices[other_index] = index - 1;
	}
}

void Boid::flock(const Grid& grid, std::span<const Boid> boids, std::span<const std::uint32_t> proxy)
{
	sf::Vector2f sep;
	sf::Vector2f ali;
	sf::Vector2f coh;

	std::uint16_t sep_count = 0;
	std::uint16_t ali_count = 0;
	std::uint16_t coh_count = 0;

	const sf::Vector2f origin = get_origin();
	const float vel_length = vu::distance(_velocity);

	constexpr std::uint8_t neighbours = 4; // max 4 neighbours at a time

	int neighbour_indicies[neighbours];
	sf::Vector2f neighbour[neighbours];

	const sf::Vector2f grid_cell_raw = grid.relative_pos(origin);
	const sf::Vector2i grid_cell = sf::Vector2i(grid_cell_raw);
	const sf::Vector2f grid_cell_overflow = grid_cell_raw - sf::Vector2f(grid_cell);

	const int x = (grid_cell_overflow.x > 0.5f ? 1 : -1);
	const int y = (grid_cell_overflow.y > 0.5f ? 1 : -1);

	const int x_neighbor = grid_cell.x + x;
	const int y_neighbor = grid_cell.y + y;

	neighbour[0] = grid._cont_dims * sf::Vector2f(0,		0);
	neighbour[1] = grid._cont_dims * sf::Vector2f((float)x, 0);
	neighbour[2] = grid._cont_dims * sf::Vector2f(0,		(float)y);
	neighbour[3] = grid._cont_dims * sf::Vector2f((float)x, (float)y);

	neighbour_indicies[0] = grid.at_pos(grid_cell.x, grid_cell.y);	// current
	neighbour_indicies[1] = grid.at_pos(x_neighbor, grid_cell.y);	// left or right of current
	neighbour_indicies[2] = grid.at_pos(grid_cell.x, y_neighbor);	// top or bot of current
	neighbour_indicies[3] = grid.at_pos(x_neighbor, y_neighbor);	// top left/right bot left/right of current

	Config& config = Config::GetInstance();

	for (std::uint8_t i = 0; i < neighbours; ++i)
	{
		const int grid_cell_index = neighbour_indicies[i];
		const int start = grid._cells_start_indices[grid_cell_index];

		if (start < 0)
			continue;

		const sf::Vector2f neighbour_cell = neighbour[i];
		const int end = grid._cells_end_indices[grid_cell_index];

		for (int j = start; j <= end; ++j) // do in one loop
		{
			const Boid& b = boids[proxy[j]];

			if (&b == this) [[unlikely]]
				continue;

			const sf::Vector2f other_relative_pos = 
				neighbour_cell + b.get_relative_position(); // need to get relative to this boid

			const sf::Vector2f dir	= vu::direction(_relative_pos, other_relative_pos);
			const float distance_sq = std::max(vu::distance_sq(dir), FLT_EPSILON);
			const float angle		= _prev_velocity.angleTo(dir).asRadians();

			if (angle >= -config.boid_view_angle && angle <= config.boid_view_angle) // angle only effects cohesion and alignment
			{
				if (distance_sq <= config.coh_distance)
				{
					coh += origin + dir; // Head towards center of boids
					++coh_count;
				}
				if (distance_sq <= config.ali_distance)
				{
					ali += b.get_prev_velocity(); // Align with every boids velocity
					++ali_count;
				}
			}

			if (distance_sq <= config.sep_distance)
			{
				sep += -dir / distance_sq;
				++sep_count;
			}
		}
	}

	if (coh_count) coh = vu::normalize(vu::direction(origin, coh / (float)coh_count), config.boid_max_speed);
	if (ali_count) ali = vu::normalize(ali / (float)ali_count, config.boid_max_speed);
	if (sep_count) sep = vu::normalize(sep / (float)sep_count, config.boid_max_speed);

	_velocity +=
		steer_at(coh) * config.coh_weight +
		steer_at(ali) * config.ali_weight +
		steer_at(sep) * config.sep_weight;

	_density = std::max(std::max(coh_count, ali_count), sep_count);
}

void Boid::update(const RectFloat& border, const IAudioMeterInfo* audio_meter, std::span<const Impulse> impulses, float dt)
{
	_velocity = vu::clamp(_velocity, Config::GetInstance().boid_min_speed, Config::GetInstance().boid_max_speed);

	_position += _velocity * dt;

	if (outside_border(border, dt))
		_prev_position = _position;

	if (Config::GetInstance().color_flags & CF_Cycle)
		_cycle_time = std::fmodf(_cycle_time + dt * Config::GetInstance().boid_cycle_colors_speed, 1.0f);
	if (Config::GetInstance().color_flags & CF_Density)
		_density_time = (Config::GetInstance().boid_density_cycle_enabled) ? std::fmodf(_density_time + dt * Config::GetInstance().boid_density_cycle_speed, 1.0f) : 0.0f;
}

sf::Vector2f Boid::steer_at(const sf::Vector2f& steer_direction) const
{
	return vu::limit(vu::direction(_velocity, steer_direction), Config::GetInstance().boid_max_steer);
}

void Boid::steer_towards(const sf::Vector2f& direction, float length, float weight)
{
	if (std::fabsf(weight) <= FLT_EPSILON)
		return;

	const sf::Vector2f steer = vu::normalize(direction, length, Config::GetInstance().boid_max_speed);

	_velocity += steer_at(steer) * weight;
}
void Boid::steer_towards(const sf::Vector2f& point, float weight)
{
	steer_towards(point, vu::distance(point), weight);
}

sf::Vector3f Boid::get_color(const RectFloat& border, const IAudioMeterInfo* audio_meter, std::span<const Impulse> impulses) const noexcept
{
	sf::Vector3f color = sf::Vector3f();

	if (Config::GetInstance().color_flags & CF_Positional)
		color += position_color(border) * Config::GetInstance().color_positional_weight;
	if (Config::GetInstance().color_flags & CF_Cycle)
		color += cycle_color() * Config::GetInstance().color_cycle_weight;
	if (Config::GetInstance().color_flags & CF_Density)
		color += density_color() * Config::GetInstance().color_density_weight;
	if (Config::GetInstance().color_flags & CF_Velocity)
		color += velocity_color() * Config::GetInstance().color_velocity_weight;
	if (Config::GetInstance().color_flags & CF_Rotation)
		color += rotation_color() * Config::GetInstance().color_rotation_weight;
	if (Config::GetInstance().color_flags & CF_Audio)
		color += audio_color(audio_meter) * Config::GetInstance().color_audio_weight;

	impulse_color(color, impulses);

	return color;
}

const sf::Vector2f& Boid::get_position() const noexcept
{
	return _position; 
}
const sf::Vector2f& Boid::get_prev_position() const noexcept
{ 
	return _prev_position; 
}
const sf::Vector2f& Boid::get_relative_position() const noexcept
{ 
	return _relative_pos; 
}
const sf::Vector2f& Boid::get_velocity() const noexcept
{ 
	return _velocity; 
}
const sf::Vector2f& Boid::get_prev_velocity() const noexcept
{ 
	return _prev_velocity; 
}
std::uint16_t Boid::get_cell_index() const noexcept
{ 
	return _cell_index; 
}

sf::Vector2f Boid::get_origin() const noexcept
{
	return get_position() + sf::Vector2f(
		Config::GetInstance().boid_size_width,
		Config::GetInstance().boid_size_height) / 2.0f;
}
sf::Vector2f Boid::get_prev_origin() const noexcept
{
	return get_prev_position() + sf::Vector2f(
		Config::GetInstance().boid_size_width,
		Config::GetInstance().boid_size_height) / 2.0f;
}

void Boid::set_cycle_time(const float val) noexcept
{
	_cycle_time = val;
}

bool Boid::outside_border(const RectFloat& border, float dt)
{
	return Config::GetInstance().turn_at_border ? turn_at_border(border, dt) : teleport_at_border(border);
}
bool Boid::turn_at_border(const RectFloat& border, float dt)
{
	const float width_margin = border.width() - border.width() * Config::GetInstance().turn_margin_factor;
	const float height_margin = border.height() - border.height() * Config::GetInstance().turn_margin_factor;

	const float left_margin = border.left + width_margin;
	const float top_margin = border.top + height_margin;
	const float right_margin = border.right - width_margin;
	const float bot_margin = border.bot - height_margin;

	if (_position.x + Config::GetInstance().boid_size_width < left_margin)
		_velocity.x += Config::GetInstance().turn_factor * dt * (1.0f + std::powf(std::abs(_position.x - left_margin) / width_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.x > right_margin)
		_velocity.x -= Config::GetInstance().turn_factor * dt * (1.0f + std::powf(std::abs(_position.x - right_margin) / width_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.y + Config::GetInstance().boid_size_height < top_margin)
		_velocity.y += Config::GetInstance().turn_factor * dt * (1.0f + std::powf(std::abs(_position.y - top_margin) / height_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	if (_position.y > bot_margin)
		_velocity.y -= Config::GetInstance().turn_factor * dt * (1.0f + std::powf(std::abs(_position.y - bot_margin) / height_margin, 2.0f)) * (1.0f / (_density + 1.0f));

	return false;
}
bool Boid::teleport_at_border(const RectFloat& border)
{
	const sf::Vector2f current = _position;
	const float boid_size_max = std::max(Config::GetInstance().boid_size_width, Config::GetInstance().boid_size_height);

	if (_position.x + boid_size_max < border.left)
		_position.x = (float)border.right;

	if (_position.x > border.right)
		_position.x = border.left - boid_size_max;

	if (_position.y + boid_size_max < border.top)
		_position.y = (float)border.bot;

	if (_position.y > border.bot)
		_position.y = border.top - boid_size_max;

	return (current != _position);
}

sf::Vector3f Boid::position_color(const RectFloat& border) const
{
	const float t = _position.x / border.width();
	const float s = _position.y / border.height();

	return sf::Vector3f(
		util::interpolate(Config::GetInstance().boid_color_top_left.x * 255.0f, Config::GetInstance().boid_color_top_right.x * 255.0f, Config::GetInstance().boid_color_bot_left.x * 255.0f, Config::GetInstance().boid_color_bot_right.x * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::GetInstance().boid_color_top_left.y * 255.0f, Config::GetInstance().boid_color_top_right.y * 255.0f, Config::GetInstance().boid_color_bot_left.y * 255.0f, Config::GetInstance().boid_color_bot_right.y * 255.0f, t, s) / 255.0f,
		util::interpolate(Config::GetInstance().boid_color_top_left.z * 255.0f, Config::GetInstance().boid_color_top_right.z * 255.0f, Config::GetInstance().boid_color_bot_left.z * 255.0f, Config::GetInstance().boid_color_bot_right.z * 255.0f, t, s) / 255.0f);
}
sf::Vector3f Boid::cycle_color() const
{
	if (Config::GetInstance().boid_cycle_colors.empty()) [[unlikely]]
		return sf::Vector3f();

	float scaled_time = _cycle_time * (float)(Config::GetInstance().boid_cycle_colors.size() - 1);

	const auto index1 = (int)scaled_time;
	const auto index2 = ((int)scaled_time + 1) % (int)Config::GetInstance().boid_cycle_colors.size();

	const sf::Vector3f color1 = Config::GetInstance().boid_cycle_colors[index1];
	const sf::Vector3f color2 = Config::GetInstance().boid_cycle_colors[index2];

	const float newT = scaled_time - std::floorf(scaled_time);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::density_color() const
{
	if (Config::GetInstance().boid_density_colors.empty() || !Config::GetInstance().boid_density) [[unlikely]]
		return sf::Vector3f();

	const float density_percentage = (_density / (float)Config::GetInstance().boid_density);

	const float scaled_density = std::fmodf(density_percentage + _density_time, 1.0f) * (float)(Config::GetInstance().boid_density_colors.size() - 1);

	const auto index1 = (int)scaled_density;
	const auto index2 = ((int)scaled_density + 1) % (int)Config::GetInstance().boid_density_colors.size();

	const sf::Vector3f color1 = Config::GetInstance().boid_density_colors[index1];
	const sf::Vector3f color2 = Config::GetInstance().boid_density_colors[index2];

	const float newT = scaled_density - std::floorf(scaled_density);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::velocity_color() const
{
	if (Config::GetInstance().boid_velocity_colors.empty() || (Config::GetInstance().boid_max_speed == Config::GetInstance().boid_min_speed)) [[unlikely]]
		return sf::Vector3f();

	const float velocity_percentage = std::clamp((vu::distance(_velocity) - Config::GetInstance().boid_min_speed) / (Config::GetInstance().boid_max_speed - Config::GetInstance().boid_min_speed), 0.0f, 1.0f);

	const float scaled_velocity = velocity_percentage * (float)(Config::GetInstance().boid_velocity_colors.size() - 1);

	const auto index1 = (int)scaled_velocity;
	const auto index2 = ((int)scaled_velocity + 1) % (int)Config::GetInstance().boid_velocity_colors.size();

	const sf::Vector3f color1 = Config::GetInstance().boid_velocity_colors[index1];
	const sf::Vector3f color2 = Config::GetInstance().boid_velocity_colors[index2];

	const float newT = scaled_velocity - std::floorf(scaled_velocity);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::rotation_color() const
{
	if (Config::GetInstance().boid_rotation_colors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float rotation_percentage = (_velocity.angle().asRadians() + float(M_PI)) / (2.0f * float(M_PI));

	const float scaled_rotation = rotation_percentage * (float)(Config::GetInstance().boid_rotation_colors.size() - 1);

	const auto index1 = (int)scaled_rotation;
	const auto index2 = ((int)scaled_rotation + 1) % (int)Config::GetInstance().boid_rotation_colors.size();

	const sf::Vector3f color1 = Config::GetInstance().boid_rotation_colors[index1];
	const sf::Vector3f color2 = Config::GetInstance().boid_rotation_colors[index2];

	const float newT = scaled_rotation - std::floorf(scaled_rotation);

	return vu::lerp(color1, color2, newT);
}
sf::Vector3f Boid::audio_color(const IAudioMeterInfo* audio_meter) const
{
	if (Config::GetInstance().audio_responsive_colors.empty()) [[unlikely]]
		return sf::Vector3f();

	const float density_percentage = (_density / (float)Config::GetInstance().audio_responsive_density);
	const float max_volume = std::fminf(audio_meter->get_volume() * Config::GetInstance().audio_responsive_strength, Config::GetInstance().audio_responsive_limit);

	const float scaled_volume = std::fminf(max_volume * density_percentage, 1.0f) * (float)(Config::GetInstance().audio_responsive_colors.size() - 1);

	const auto index1 = (int)scaled_volume;
	const auto index2 = ((int)scaled_volume + 1) % (int)Config::GetInstance().audio_responsive_colors.size();

	const sf::Vector3f color1 = Config::GetInstance().audio_responsive_colors[index1];
	const sf::Vector3f color2 = Config::GetInstance().audio_responsive_colors[index2];

	const float newT = scaled_volume - std::floorf(scaled_volume);

	return vu::lerp(color1, color2, newT);
}
void Boid::impulse_color(sf::Vector3f& color, std::span<const Impulse> impulses) const
{
	if (Config::GetInstance().impulse_colors.empty()) [[unlikely]]
		return;

	for (const Impulse& impulse : impulses)
	{
		const sf::Vector2f impulse_pos = impulse.get_position();
		const float impulse_length = impulse.get_length();

		const float length = vu::distance(_position, impulse_pos);
		const float diff = std::abs(length - impulse_length);

		const float percentage = (impulse_length / Config::GetInstance().impulse_fade_distance);
		const float size = impulse.get_size() * (1.0f - percentage);

		if (diff <= size)
		{
			const float scaled_length = std::fmodf(percentage, 1.0f) * (float)(Config::GetInstance().impulse_colors.size() - 1);

			const auto index1 = (int)scaled_length;
			const auto index2 = ((int)scaled_length + 1) % (int)Config::GetInstance().impulse_colors.size();

			const sf::Vector3f color1 = Config::GetInstance().impulse_colors[index1];
			const sf::Vector3f color2 = Config::GetInstance().impulse_colors[index2];

			const float newT = scaled_length - std::floorf(scaled_length);

			color = vu::lerp(color1, color2, newT);
		}
	}
}
