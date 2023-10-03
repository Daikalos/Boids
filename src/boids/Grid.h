#pragma once

#include <memory>

#include <SFML/System/Vector2.hpp>

#include "../utilities/Rectangle.hpp"

class Grid
{
public:
	Grid() = default;
	Grid(const RectFloat& rect, const sf::Vector2f& contDims);

public:
	[[nodiscard]] const RectFloat& GetRootRect() const noexcept;
	[[nodiscard]] const sf::Vector2f& GetContDims() const noexcept;
	[[nodiscard]] const int* GetStartIndices() const noexcept;
	[[nodiscard]] const int* GetEndIndices() const noexcept;
	[[nodiscard]] int GetCount() const noexcept;

	void SetStartIndex(int index, int value);
	void SetEndIndex(int index, int value);

public:
	[[nodiscard]] sf::Vector2f RelativePos(const sf::Vector2f& position) const;
	[[nodiscard]] int AtPos(const sf::Vector2f& position) const;
	[[nodiscard]] int AtPos(const sf::Vector2i& position) const noexcept;
	[[nodiscard]] int AtPos(int x, int y) const noexcept;

public:
	void ResetBuffers();

private:
	RectFloat		m_rootRect;
	sf::Vector2f	m_contDims;

	std::unique_ptr<int[]> m_startIndices;
	std::unique_ptr<int[]> m_endIndices;

	int m_width{0}, m_height{0}, m_count{0};
};

