#include "Quadtree.h"

Quadtree::Quadtree(const rect_i& rect, int capacity) 
	: rectangle(rect), capacity(capacity)
{
	northWest = nullptr;
	northEast = nullptr;
	southWest = nullptr;
	southEast = nullptr;

	divided = false;
}

Quadtree::~Quadtree()
{
	delete northWest;
	delete northEast;
	delete southWest;
	delete southEast;
}

std::vector<Boid> Quadtree::query(const rect_i& rect) const
{
	std::vector<Boid> foundBoids;

	if (!intersects(rect))
		return foundBoids;

	for (const Boid* b : boids)
	{
		if (contains(b->get_position()))
		{
			foundBoids.push_back(*b);
		}
	}

	if (!divided)
		return foundBoids;

	std::vector<Boid> nwQuery = northWest->query(rect);
	std::vector<Boid> neQuery = northEast->query(rect);
	std::vector<Boid> swQuery = southWest->query(rect);
	std::vector<Boid> seQuery = southEast->query(rect);

	foundBoids.insert(foundBoids.end(), nwQuery.begin(), nwQuery.end());
	foundBoids.insert(foundBoids.end(), neQuery.begin(), neQuery.end());
	foundBoids.insert(foundBoids.end(), swQuery.begin(), swQuery.end());
	foundBoids.insert(foundBoids.end(), seQuery.begin(), seQuery.end());

	return foundBoids;
}

bool Quadtree::insert(const Boid& boid)
{
	if (&boid == NULL)
		return false;

	if (!contains(boid.get_position()))
		return false;

	if (boids.size() < capacity)
	{
		boids.push_back(&boid);
		return true;
	}
	else
	{
		if (!divided)
			subdivide();

		if (northWest->insert(boid))
			return true;
		else if (northEast->insert(boid))
			return true;
		else if (southWest->insert(boid))
			return true;
		else if (southEast->insert(boid))
			return true;
	}
	return false;
}

bool Quadtree::contains(const sf::Vector2f& point) const
{
	return 
		point.x >= rectangle.left &&
		point.y >= rectangle.top &&
		point.x <= rectangle.right &&
		point.y <= rectangle.bot;
}

bool Quadtree::intersects(const rect_i& rect) const
{
	return
		!(rect.right <= rectangle.left ||
		  rect.left >= rectangle.right ||
		  rect.top >= rectangle.bot ||
		  rect.bot <= rectangle.top);
}

void Quadtree::subdivide()
{
	const sf::Vector2i center = 
	{ 
		(rectangle.left + rectangle.right) / 2, 
		(rectangle.top + rectangle.bot) / 2 
	};

	northWest = new Quadtree({ { rectangle.left, rectangle.top }, { center.x,        center.y      } }, capacity);
	northEast = new Quadtree({ { center.x,       rectangle.top }, { rectangle.right, center.y      } }, capacity);
	southWest = new Quadtree({ { rectangle.left, center.y },      { center.x,        rectangle.bot } }, capacity);
	southEast = new Quadtree({ { center.x,       center.y },      { rectangle.right, rectangle.bot } }, capacity);

	divided = true;
}
