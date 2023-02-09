#include "Grid.h"

Grid::Grid(Config& config, const RectFloat& rect, const sf::Vector2f& cont_dims)
	: _rect(rect), _cont_dims(cont_dims)
{
	float boid_size_max = std::max(config.boid_size_width, config.boid_size_height);

	_rect.top_left -= sf::Vector2f(boid_size_max, boid_size_max) / 2.0f;
	_rect.bot_right += sf::Vector2f(boid_size_max, boid_size_max) / 2.0f;

	float a = _rect.width() / _cont_dims.x;
	float b = _rect.height() / _cont_dims.y;

	_cont_dims.x = _rect.width() / std::floorf(a);
	_cont_dims.y = _rect.height() / std::floorf(b);

	_width = (int)a;
	_height = (int)b;

	_count = _width * _height;

	_cells_start_indices.resize(_count, -1);
	_cells_end_indices.resize(_count, -1);
}

void Grid::reset_buffers()
{
	std::fill_n(_cells_start_indices.begin(), _cells_start_indices.size(), -1);
	std::fill_n(_cells_end_indices.begin(), _cells_end_indices.size(), -1);
}