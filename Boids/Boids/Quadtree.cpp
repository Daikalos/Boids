#include "Quadtree.h"

template class Quadtree<Boid>;

template<typename T>
Quadtree<T>::Quadtree(const Rect_i& rect, int capacity) 
	: rectangle(rect), capacity(capacity)
{
	northWest = nullptr;
	northEast = nullptr;
	southWest = nullptr;
	southEast = nullptr;

	divided = false;
}

template<typename T>
Quadtree<T>::~Quadtree()
{
	delete northWest;
	delete northEast;
	delete southWest;
	delete southEast;
}

template<typename T>
std::vector<const T*> Quadtree<T>::query(const Rect_i& rect) const
{
	std::vector<const T*> foundItems;

	if (!intersects(rect))
		return foundItems;

	for (const T* item : items)
	{
		const Boid* boid = dynamic_cast<const Boid*>(item);
		if (boid != nullptr)
		{
			if (contains(boid->get_position()))
			{
				foundItems.push_back(item);
			}
		}
	}

	if (!divided)
		return foundItems;

	std::vector<const T*> nwQuery = northWest->query(rect);
	std::vector<const T*> neQuery = northEast->query(rect);
	std::vector<const T*> swQuery = southWest->query(rect);
	std::vector<const T*> seQuery = southEast->query(rect);

	foundItems.insert(foundItems.end(), nwQuery.begin(), nwQuery.end());
	foundItems.insert(foundItems.end(), neQuery.begin(), neQuery.end());
	foundItems.insert(foundItems.end(), swQuery.begin(), swQuery.end());
	foundItems.insert(foundItems.end(), seQuery.begin(), seQuery.end());

	return foundItems;
}

template<typename T>
bool Quadtree<T>::insert(const T& item)
{
	if (&item == nullptr)
		return false;

	if (!contains(item.get_origin()))
		return false;

	if (items.size() < capacity)
	{
		items.push_back(&item);
		return true;
	}
	else
	{
		if (!divided)
			subdivide();

		if (northWest->insert(item))
			return true;
		else if (northEast->insert(item))
			return true;
		else if (southWest->insert(item))
			return true;
		else if (southEast->insert(item))
			return true;
	}
	return false;
}

template<typename T>
bool Quadtree<T>::contains(const sf::Vector2f& point) const
{
	return 
		point.x >= rectangle.left &&
		point.y >= rectangle.top &&
		point.x <= rectangle.right &&
		point.y <= rectangle.bot;
}

template<typename T>
bool Quadtree<T>::intersects(const Rect_i& rect) const
{
	return
		!(rect.right <= rectangle.left ||
		  rect.left >= rectangle.right ||
		  rect.top >= rectangle.bot ||
		  rect.bot <= rectangle.top);
}

template<typename T>
void Quadtree<T>::subdivide()
{
	const sf::Vector2i center = 
	{ 
		(rectangle.left + rectangle.right) / 2, 
		(rectangle.top + rectangle.bot) / 2 
	};

	northWest = new Quadtree<T>(Rect_i({ rectangle.left, rectangle.top }, { center.x,        center.y      } ), capacity);
	northEast = new Quadtree<T>(Rect_i({ center.x,       rectangle.top }, { rectangle.right, center.y      } ), capacity);
	southWest = new Quadtree<T>(Rect_i({ rectangle.left, center.y	    }, { center.x,        rectangle.bot } ), capacity);
	southEast = new Quadtree<T>(Rect_i({ center.x,       center.y	    }, { rectangle.right, rectangle.bot } ), capacity);

	divided = true;
}
