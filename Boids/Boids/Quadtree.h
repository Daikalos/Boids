#pragma once

#include <SFML/Graphics.hpp>
#include "Rectangle.h"
#include "Boid.h"

template<typename T> class Quadtree
{
public:
	Quadtree(const Rect_i& rect, int capacity);
	virtual ~Quadtree();

	std::vector<T> query(const Rect_i& rect) const;
	
	bool insert(const T& boid);

private:
	bool contains(const sf::Vector2f& point) const;
	bool intersects(const Rect_i& rect) const;

	void subdivide();

private:
	const Rect_i rectangle;

	Quadtree* northWest;
	Quadtree* northEast;
	Quadtree* southWest;
	Quadtree* southEast;

	std::vector<const T*> items;

	int capacity;
	bool divided;

private:
	Quadtree() = delete;
	Quadtree(const Quadtree& rhs) = delete;
	Quadtree& operator=(const Quadtree& rhs) = delete;
};

typedef Quadtree<Boid> QuadtreeB;