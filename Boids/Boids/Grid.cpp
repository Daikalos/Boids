#include "Grid.h"

template class Grid<Boid>;

template<typename T>
Grid<T>::Grid(int cont_width, int cont_height, int grid_width, int grid_height)
	: contDims({ cont_width, cont_height })
{
	width = grid_width / cont_width;
    height = grid_height / cont_height;

	containers = new Container<T>[width * height];

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			containers[x + y * width] = Container<T>(Rect_i(
				{ x * cont_width, y * cont_height },
				{ x * cont_width + cont_width, y * cont_height + cont_height }));
		}
	}
}

template<typename T>
Grid<T>::~Grid()
{
	delete[] containers;
}

template<typename T>
void Grid<T>::insert(const T& item)
{
	Container<T>* newCntn = at_pos(item);
	Container<T>* curCntn = items[&item];

	if (newCntn == nullptr || curCntn == newCntn)
		return;

	if (curCntn != nullptr && curCntn != newCntn)
	{
		curCntn->erase(item);
	}

	items[&item] = newCntn;
	newCntn->insert(item);
}

template<typename T>
std::vector<const T*> Grid<T>::query(sf::Vector2f pos, float radius)
{
	std::vector<const T*> foundItems;
	std::unordered_set<const Container<T>*> cntns;

	for (float x = -radius; x <= radius; x += radius)
		for (float y = -radius; y <= radius; y += radius)
		{
			const Container<T>* cntn = at_pos(sf::Vector2f(pos.x + x, pos.y + y));

			if (cntn != nullptr)
				cntns.insert(cntn);
		}

	for (const Container<T>* c : cntns)
	{
		for (const T* b : c->items)
		{
			foundItems.push_back(b);
		}
	}

	return foundItems;
}
