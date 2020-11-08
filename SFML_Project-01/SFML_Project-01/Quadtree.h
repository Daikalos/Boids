#pragma once

#include <SFML/Graphics.hpp>
#include "Boid.h"

class Quad
{
public:
	Quad(const sf::Vector2i& topLeft, const sf::Vector2i& botRight, int capacity);
	~Quad();

	std::vector<Boid> Query(const sf::Vector2i& topLeft, const sf::Vector2i& botRight) const;
	bool Insert(const Boid& boid);

private:
	bool Contains(const sf::Vector2<double>& point) const;
	bool Intersects(const sf::Vector2i& topLeft, const sf::Vector2i& botRight) const;

	void Subdivide();

	void Clear();

private:
	sf::Vector2i m_TopLeft;
	sf::Vector2i m_BotRight;

	Quad* m_NorthWest;
	Quad* m_NorthEast;
	Quad* m_SouthWest;
	Quad* m_SouthEast;

	std::vector<const Boid*> m_Boids;

	int m_Capacity;
	bool m_Divided = false;
};

