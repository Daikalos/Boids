#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <unordered_map>

#include <iostream>
#include <filesystem>

enum class TextureID
{
	Background
};

enum class FontID
{
	F8Bit
};

static const std::string RESOURCE_FOLDER = "content/";

template <class Resource, class Identifier>
class ResourceHolder
{
public:
	using Ptr = std::unique_ptr<Resource>;

public:
	void Load(const Identifier& id, const std::string& path);
	void Remove(const Identifier& id);

	bool Exists(const Identifier& id) const;

	Resource& Get(const Identifier& id);
	const Resource& Get(const Identifier& id) const;

private:
	std::unordered_map<Identifier, Ptr> m_resources;
};

template<typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::Load(const Identifier& id, const std::string& path)
{
	Ptr resource(new Resource());

	if (!resource->loadFromFile(RESOURCE_FOLDER + path))
	{
		Remove(id);
		return;
	}

	auto inserted = m_resources.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}

template<class Resource, class Identifier>
void ResourceHolder<Resource, Identifier>::Remove(const Identifier& id)
{
	if (!Exists(id))
		return;

	m_resources.erase(id);
}

template<typename Resource, typename Identifier>
Resource& ResourceHolder<Resource, Identifier>::Get(const Identifier& id)
{
	return Get(id);
}

template<typename Resource, typename Identifier>
const Resource& ResourceHolder<Resource, Identifier>::Get(const Identifier& id) const
{
	auto it = m_resources.find(id);
	assert(it != m_resources.end());

	return *it->second.get();
}


template<class Resource, class Identifier>
bool ResourceHolder<Resource, Identifier>::Exists(const Identifier& id) const
{
	return m_resources.find(id) != m_resources.end();
}

using TextureHolder = ResourceHolder<sf::Texture, TextureID>;
using FontHolder = ResourceHolder<sf::Font, FontID>;

