#pragma once

#include <SFML/Graphics.hpp>

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"

#include "Config.h"

class Grid
{
public:
	Grid() = default;
	Grid(const RectFloat& rect, const sf::Vector2f& cont_dims);

public:
	int GetCount() const noexcept;

public:
	[[nodiscard]] sf::Vector2f RelativePos(const sf::Vector2f& position) const;
	[[nodiscard]] int AtPos(const sf::Vector2f& position) const;
	[[nodiscard]] int AtPos(const sf::Vector2i& position) const noexcept;
	[[nodiscard]] int AtPos(int x, int y) const noexcept;

public:
	void ResetBuffers();

public:
	RectFloat		rootRect;
	sf::Vector2f	contDims;

	std::unique_ptr<int[]> startIndices;
	std::unique_ptr<int[]> endIndices;

private:
	int m_width{0}, m_height{0}, m_count{0};
};

