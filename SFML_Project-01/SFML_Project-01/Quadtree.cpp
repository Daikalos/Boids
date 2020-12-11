#include "Quadtree.h"

Quad::Quad(const sf::Vector2i& t_left, const sf::Vector2i& b_right, int capacity) :
	top_left(t_left), bot_right(b_right), capacity(capacity)
{
	northWest = nullptr;
	northEast = nullptr;
	southWest = nullptr;
	southEast = nullptr;

	divided = false;
}

Quad::~Quad()
{
	delete northWest;
	delete northEast;
	delete southWest;
	delete southEast;
}

std::vector<Boid> Quad::query(const sf::Vector2i& t_left, const sf::Vector2i& b_right) const
{
	std::vector<Boid> foundBoids;

	if (!intersects(t_left, b_right))
		return foundBoids;

	for (const Boid* b : boids)
	{
		if (contains(b->getPosition()))
		{
			foundBoids.push_back(*b);
		}
	}

	if (!divided)
		return foundBoids;

	std::vector<Boid> nwQuery = northWest->query(t_left, b_right);
	std::vector<Boid> neQuery = northEast->query(t_left, b_right);
	std::vector<Boid> swQuery = southWest->query(t_left, b_right);
	std::vector<Boid> seQuery = southEast->query(t_left, b_right);

	foundBoids.insert(foundBoids.end(), nwQuery.begin(), nwQuery.end());
	foundBoids.insert(foundBoids.end(), neQuery.begin(), neQuery.end());
	foundBoids.insert(foundBoids.end(), swQuery.begin(), swQuery.end());
	foundBoids.insert(foundBoids.end(), seQuery.begin(), seQuery.end());

	return foundBoids;
}

bool Quad::insert(const Boid& boid)
{
	if (&boid == NULL)
		return false;

	if (!contains(boid.getPosition()))
		return false;

	if (boids.size() < capacity)
	{
		boids.push_back(&boid);
		return true;
	}
	else
	{
		if (!divided)
		{
			subdivide();
		}

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

bool Quad::contains(const sf::Vector2<double>& point) const
{
	return 
		point.x >= top_left.x &&
		point.y >= top_left.y &&
		point.x <= bot_right.x && 
		point.y <= bot_right.y;
}

bool Quad::intersects(const sf::Vector2i& t_left, const sf::Vector2i& b_right) const
{
	return
		!(b_right.x <= top_left.x ||
		  t_left.x >= bot_right.x ||
		  t_left.y >= bot_right.y ||
		  b_right.y <= top_left.y);
}

void Quad::subdivide()
{
	const int x0 = top_left.x;
	const int y0 = top_left.y;
	const int x1 = bot_right.x;
	const int y1 = bot_right.y;
	const int width = (top_left.x + bot_right.x) / 2;
	const int height = (top_left.y + bot_right.y) / 2;

	sf::Vector2i nwTopLeft = sf::Vector2i(x0, y0);
	sf::Vector2i nwBotRight = sf::Vector2i(width, height);
	northWest = new Quad(nwTopLeft, nwBotRight, capacity);

	sf::Vector2i neTopLeft = sf::Vector2i(width, y0);
	sf::Vector2i neBotRight = sf::Vector2i(x1, height);
	northEast = new Quad(neTopLeft, neBotRight, capacity);

	sf::Vector2i swTopLeft = sf::Vector2i(x0, height);
	sf::Vector2i swBotRight = sf::Vector2i(width, y1);
	southWest = new Quad(swTopLeft, swBotRight, capacity);

	sf::Vector2i seTopLeft = sf::Vector2i(width, height);
	sf::Vector2i seBotRight = sf::Vector2i(x1, y1);
	southEast = new Quad(seTopLeft, seBotRight, capacity);

	divided = true;
}
