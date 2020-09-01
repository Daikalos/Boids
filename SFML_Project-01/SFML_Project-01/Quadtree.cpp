#include "Quadtree.h"

Quad::Quad(sf::Vector2i topLeft, sf::Vector2i botRight, int capacity) :
	m_TopLeft(topLeft), m_BotRight(botRight), m_Capacity(capacity)
{
	m_NorthWest = nullptr;
	m_NorthEast = nullptr;
	m_SouthWest = nullptr;
	m_SouthEast = nullptr;

	m_Divided = false;
}

Quad::~Quad()
{
	if (m_Divided)
		Clear();
}

std::vector<Boid> Quad::Query(sf::Vector2i topLeft, sf::Vector2i botRight) const
{
	std::vector<Boid> foundBoids;

	if (!Intersects(topLeft, botRight))
		return foundBoids;

	for (const Boid* b : m_Boids)
	{
		if (Contains(b->GetPosition()))
		{
			foundBoids.push_back(*b);
		}
	}

	if (!m_Divided)
		return foundBoids;

	std::vector<Boid> nwQuery = m_NorthWest->Query(topLeft, botRight);
	std::vector<Boid> neQuery = m_NorthEast->Query(topLeft, botRight);
	std::vector<Boid> swQuery = m_SouthWest->Query(topLeft, botRight);
	std::vector<Boid> seQuery = m_SouthEast->Query(topLeft, botRight);

	foundBoids.insert(foundBoids.end(), nwQuery.begin(), nwQuery.end());
	foundBoids.insert(foundBoids.end(), neQuery.begin(), neQuery.end());
	foundBoids.insert(foundBoids.end(), swQuery.begin(), swQuery.end());
	foundBoids.insert(foundBoids.end(), seQuery.begin(), seQuery.end());

	return foundBoids;
}

bool Quad::Insert(const Boid& boid)
{
	if (&boid == NULL)
		return false;

	if (!Contains(boid.GetPosition()))
		return false;

	if (m_Boids.size() < m_Capacity)
	{
		m_Boids.push_back(&boid);
		return true;
	}
	else
	{
		if (!m_Divided)
		{
			Subdivide();
		}

		if (m_NorthWest->Insert(boid))
			return true;
		else if (m_NorthEast->Insert(boid))
			return true;
		else if (m_SouthWest->Insert(boid))
			return true;
		else if (m_SouthEast->Insert(boid))
			return true;
	}
	return false;
}

void Quad::Clear()
{
	delete m_NorthWest;
	delete m_NorthEast;
	delete m_SouthWest;
	delete m_SouthEast;
}

bool Quad::Contains(sf::Vector2f point) const
{
	return 
		point.x >= m_TopLeft.x && 
		point.y >= m_TopLeft.y && 
		point.x <= m_BotRight.x && 
		point.y <= m_BotRight.y;
}

bool Quad::Intersects(sf::Vector2i topLeft, sf::Vector2i botRight) const
{
	return
		!(botRight.x <= m_TopLeft.x ||
		  topLeft.x >= m_BotRight.x ||
		  topLeft.y >= m_BotRight.y ||
		  botRight.y <= m_TopLeft.y);
}

void Quad::Subdivide()
{
	int x0 = m_TopLeft.x;
	int y0 = m_TopLeft.y;
	int x1 = m_BotRight.x;
	int y1 = m_BotRight.y;
	int width = (m_TopLeft.x + m_BotRight.x) / 2;
	int height = (m_TopLeft.y + m_BotRight.y) / 2;

	sf::Vector2i nwTopLeft = sf::Vector2i(x0, y0);
	sf::Vector2i nwBotRight = sf::Vector2i(width, height);
	m_NorthWest = new Quad(nwTopLeft, nwBotRight, m_Capacity);

	sf::Vector2i neTopLeft = sf::Vector2i(width, y0);
	sf::Vector2i neBotRight = sf::Vector2i(x1, height);
	m_NorthEast = new Quad(neTopLeft, neBotRight, m_Capacity);

	sf::Vector2i swTopLeft = sf::Vector2i(x0, height);
	sf::Vector2i swBotRight = sf::Vector2i(width, y1);
	m_SouthWest = new Quad(swTopLeft, swBotRight, m_Capacity);

	sf::Vector2i seTopLeft = sf::Vector2i(width, height);
	sf::Vector2i seBotRight = sf::Vector2i(x1, y1);
	m_SouthEast = new Quad(seTopLeft, seBotRight, m_Capacity);

	m_Divided = true;
}
