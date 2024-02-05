#pragma once

#include <future>
#include <memory>
#include <unordered_map>
#include <thread>
#include <shared_mutex>

#include "ResourceLoader.hpp"

enum class FontID
{
	F8Bit
};

enum class TextureID
{
	Boid,
	Background
};

namespace res
{
	enum class LoadStrategy
	{
		New,
		Reuse,
		Reload
	};
}

static constexpr auto RESOURCE_FOLDER = "assets/";

/// Holds resources, for example, fonts, textures, and sounds.
///
template <class R, typename I>
class ResourceHolder
{
public:
	using ResourcePtr		= typename std::unique_ptr<R>;

	using ReturnType		= R&;
	using ConstReturnType	= const R&;

public:
	auto operator[](const I& id) -> ReturnType;
	auto operator[](const I& id) const -> ConstReturnType;

	auto Get(const I& id) -> ReturnType;
	auto Get(const I& id) const -> ConstReturnType;

public:
	auto Acquire(const I& id, const ResourceLoader<R>& loader, res::LoadStrategy strat = res::LoadStrategy::New) -> ReturnType;

	auto AcquireAsync(const I& id, const ResourceLoader<R>& loader, res::LoadStrategy strat = res::LoadStrategy::New) -> std::future<ReturnType>;

	void Release(const I& id);

	bool Contains(const I& id) const;

private:
	void ReleaseImpl(const I& id);
	auto Load(const I& id, const ResourceLoader<R>& loader) -> ReturnType;
	auto Insert(const I& id, ResourcePtr& resource) -> ReturnType;

private:
	std::unordered_map<I, ResourcePtr> m_resources;
	mutable std::shared_mutex m_mutex;
};

template<class R, typename I>
inline auto ResourceHolder<R, I>::operator[](const I& id) -> ReturnType
{
	return Get();
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::operator[](const I& id) const -> ConstReturnType
{
	return Get();
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::Get(const I& id) -> ReturnType
{
	std::shared_lock lock(m_mutex);

	const auto it = m_resources.find(id);
	if (it == m_resources.end())
		throw std::runtime_error("Resource does not exist");

	return *it->second;
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::Get(const I& id) const -> ConstReturnType
{
	return const_cast<ResourceHolder<R, I>&>(*this).Get(id);
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::Acquire(const I& id, const ResourceLoader<R>& loader, res::LoadStrategy strat) -> ReturnType
{
	std::lock_guard lock(m_mutex);

	const auto it = m_resources.find(id);
	if (it == m_resources.end())
		return Load(id, loader);

	switch (strat)
	{
	case res::LoadStrategy::New:
		throw std::runtime_error("Failed to load, already exists in container");
	case res::LoadStrategy::Reload:
		ReleaseImpl(id);
		return Load(id, loader);
	default: // reuse as default
		return *it->second;
	}
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::AcquireAsync(const I& id, const ResourceLoader<R>& loader, res::LoadStrategy strat) -> std::future<ReturnType>
{
	return std::async(std::launch::async, 
		[this](I id, ResourceLoader<R> loader, res::LoadStrategy strat) -> ReturnType
		{
			ResourcePtr resource = loader(); // we load it first for async benefits

			if (!resource)
				throw std::runtime_error("Failed to load resource");

			std::lock_guard lock(m_mutex); // guard race condition for resources

			const auto it = m_resources.find(id);
			if (it == m_resources.end())
				return Insert(id, resource);

			switch (strat)
			{
			case res::LoadStrategy::New:
				throw std::runtime_error("Failed to load, already exists in container");
			case res::LoadStrategy::Reload:
				ReleaseImpl(id);
				return Insert(id, resource);
			default: // reuse as default
				return *it->second;
			}
		}, id, loader, strat);
}

template<class R, typename I>
inline void ResourceHolder<R, I>::Release(const I& id)
{
	std::lock_guard lock(m_mutex);
	ReleaseImpl(id);
}

template<class R, typename I>
inline bool ResourceHolder<R, I>::Contains(const I& id) const
{
	std::shared_lock lock(m_mutex);
	return m_resources.contains(id);
}

template<class R, typename I>
inline void ResourceHolder<R, I>::ReleaseImpl(const I& id)
{
	const auto it = m_resources.find(id);
	if (it == m_resources.end())
		throw std::runtime_error("Resource does not exist");

	m_resources.erase(it);
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::Load(const I& id, const ResourceLoader<R>& loader) -> ReturnType
{
	ResourcePtr resource = loader();

	if (!resource)
		throw std::runtime_error("Failed to load resource");

	return Insert(id, resource);
}

template<class R, typename I>
inline auto ResourceHolder<R, I>::Insert(const I& id, ResourcePtr& resource) -> ReturnType
{
	return *m_resources.try_emplace(id, std::move(resource)).first->second;
}

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

using TextureHolder = ResourceHolder<sf::Texture, TextureID>;
using FontHolder = ResourceHolder<sf::Font, FontID>;


static const sf::Texture DEFAULT_TEXTURE;
static const sf::Font DEFAULT_FONT;