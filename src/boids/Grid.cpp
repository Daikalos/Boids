#include "Grid.h"

#include <algorithm>

#include "../utilities/VectorUtilities.hpp"

#include "Config.h"

Grid::Grid(const RectFloat& rect, const sf::Vector2f& contDims)
	: m_rootRect(rect), m_contDims(contDims)
{
	float sizeMax = std::max(Config::Inst().Boids.Width, Config::Inst().Boids.Height);
	sf::Vector2f offset = sf::Vector2f(sizeMax, sizeMax) / 2.0f;

	m_rootRect.left -= offset.x;
	m_rootRect.top -= offset.y;
	m_rootRect.width += offset.x * 2.0f;
	m_rootRect.height += offset.y * 2.0f;

	float a = m_rootRect.width / m_contDims.x;
	float b = m_rootRect.height / m_contDims.y;

	m_contDims.x = m_rootRect.width / std::floorf(a);
	m_contDims.y = m_rootRect.height / std::floorf(b);

	m_width = (int)a;
	m_height = (int)b;

	m_count = m_width * m_height;

	m_startIndices = std::make_unique<int[]>(m_count);
	m_endIndices = std::make_unique<int[]>(m_count);
}

const RectFloat& Grid::GetRootRect() const noexcept
{
	return m_rootRect;
}
const sf::Vector2f& Grid::GetContDims() const noexcept
{
	return m_contDims;
}
const int* Grid::GetStartIndices() const noexcept
{
	return m_startIndices.get();
}
const int* Grid::GetEndIndices() const noexcept
{
	return m_endIndices.get();
}
int Grid::GetCount() const noexcept
{
	return m_count;
}

void Grid::SetStartIndex(int index, int value)
{
	m_startIndices[index] = value;
}
void Grid::SetEndIndex(int index, int value)
{
	m_endIndices[index] = value;
}

sf::Vector2f Grid::RelativePos(const sf::Vector2f& position) const
{
	return (position - m_rootRect.Position()) / m_contDims;
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
	x = util::Wrap(x, 0, m_width);
	y = util::Wrap(y, 0, m_height);

	return x + y * m_width;
}

void Grid::ResetBuffers()
{
	std::ranges::fill_n(m_startIndices.get(), m_count, -1);
	std::ranges::fill_n(m_endIndices.get(), m_count, -1);
}