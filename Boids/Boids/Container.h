#pragma once

#include <unordered_set>
#include <mutex>
#include "Rectangle.h"

static std::mutex insert_lock;
static std::mutex erase_lock;

template<typename T> struct Container
{
	Container() { }
	Container(Rect_i rect) : rect(rect) { }

	void insert(const T& item)
	{
		std::lock_guard<std::mutex> lock(insert_lock);
		items.insert(&item);
	}
	void erase(const T& item)
	{
		std::lock_guard<std::mutex> lock(erase_lock);
		items.erase(&item);
	}

	Rect_i rect;
	std::unordered_set<const T*> items;
};