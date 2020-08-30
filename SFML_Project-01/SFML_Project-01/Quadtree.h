#pragma once

#include <SFML/Graphics.hpp>
#include "Boid.h"

class Quad
{
public:
	Quad(sf::Vector2i topLeft, sf::Vector2i botRight, int capacity);
	~Quad();

	void Update();

	std::vector<Boid> Query(sf::Vector2i topLeft, sf::Vector2i botRight) const;
	bool Insert(const Boid& boid);

private:
	bool Contains(sf::Vector2f point) const;
	bool Intersects(sf::Vector2i topLeft, sf::Vector2i botRight) const;

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

