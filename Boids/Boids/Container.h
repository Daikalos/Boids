#pragma once

#include <unordered_set>
#include <mutex>
#include "Rectangle.h"

static std::mutex insert_mtx;
static std::mutex erase_mtx;

template<typename T> struct Container
{
	Container() { }
	Container(Rect_i rect) : rect(rect) { }

	void insert(const T* item)
	{
		items.insert(item);
	}
	void erase(const T* item)
	{
		items.erase(item);
	}

	Rect_i rect;
	std::unordered_set<const T*> items;
};