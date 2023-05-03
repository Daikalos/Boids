#include "Grid.h"

Grid::Grid(const RectFloat& rect, const sf::Vector2f& cont_dims)
	: rootRect(rect), contDims(cont_dims)
{
	float sizeMax = std::max(Config::Inst().BoidWidth, Config::Inst().BoidHeight);

	rootRect.top_left -= sf::Vector2f(sizeMax, sizeMax) / 2.0f;
	rootRect.bot_right += sf::Vector2f(sizeMax, sizeMax) / 2.0f;

	float a = rootRect.width() / contDims.x;
	float b = rootRect.height() / contDims.y;

	contDims.x = rootRect.width() / std::floorf(a);
	contDims.y = rootRect.height() / std::floorf(b);

	m_width = (int)a;
	m_height = (int)b;

	m_count = m_width * m_height;

	startIndices.resize(m_count, -1);
	endIndices.resize(m_count, -1);
}

sf::Vector2f Grid::RelativePos(const sf::Vector2f& position) const
{
	return (position - rootRect.top_left) / contDims;
}
int Grid::AtPos(const sf::Vector2f& position) const
{
	return AtPos(sf::Vector2i(RelativePos(position)));
}
int Grid::AtPos(const sf::Vector2i& position) const noexcept
{
	return AtPos(position.x, position.y);
}
int Grid::AtPos(int x, int y) const noexcept
{
	x = util::wrap(x, 0, m_width);
	y = util::wrap(y, 0, m_height);

	return x + y * m_width;
}
sf::Vector2i Grid::AtPos(const int i) const
{
	return sf::Vector2i(i % m_width, i / m_width);
}

void Grid::ResetBuffers()
{
	std::fill_n(startIndices.begin(), startIndices.size(), -1);
	std::fill_n(endIndices.begin(), endIndices.size(), -1);
}