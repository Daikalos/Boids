#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <unordered_map>

#include <iostream>
#include <filesystem>

#include "NonCopyable.h"

enum class TextureID
{
	Background
};

enum class FontID
{
	F8Bit
};

template <class Resource, class Identifier>
class ResourceHolder : private NonCopyable
{
public:
	using ptr = std::unique_ptr<Resource>;

public:
	ResourceHolder() { }
	~ResourceHolder() { }

	void load(const Identifier& id, const std::string& path);

	template <class Parameter>
	void load(const Identifier& id, const std::string& path, const Parameter& second_param);

	Resource& get(const Identifier& id);
	const Resource& get(const Identifier& id) const;

	bool exists(const Identifier& id) const;

private:
	std::unordered_map<Identifier, ptr> _resources;
};

template<typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::load(const Identifier& id, const std::string& path)
{
	std::unique_ptr<Resource> resource(new Resource());

	if (!resource->loadFromFile(path))
		return;

	auto inserted = _resources.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}

template<typename Resource, typename Identifier>
template<typename Parameter>
void ResourceHolder<Resource, Identifier>::load(const Identifier& id, const std::string& path, const Parameter& second_param)
{
	std::unique_ptr<Resource> resource(new Resource());

	if (!resource->loadFromFile(path, second_param))
		return;

	auto inserted = _resources.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}

template<typename Resource, typename Identifier>
Resource& ResourceHolder<Resource, Identifier>::get(const Identifier& id)
{
	return get(id);
}

template<typename Resource, typename Identifier>
const Resource& ResourceHolder<Resource, Identifier>::get(const Identifier& id) const
{
	auto it = _resources.find(id);
	assert(it != _resources.end());

	return *it->second.get();
}

template<class Resource, class Identifier>
bool ResourceHolder<Resource, Identifier>::exists(const Identifier& id) const
{
	return _resources.find(id) != _resources.end();
}

using TextureHolder = ResourceHolder<sf::Texture, TextureID>;
using FontHolder = ResourceHolder<sf::Font, FontID>;

