#pragma once

#include <functional>
#include <memory>

template<class R>
class ResourceLoader
{
public:
	using Loader = std::function<std::unique_ptr<R>()>;

public:
	ResourceLoader(const Loader& loader);
	ResourceLoader(Loader&& loader);

	std::unique_ptr<R> operator()() const;
	std::unique_ptr<R> Load() const;

private:
	Loader m_loader;
};

template<class R>
inline ResourceLoader<R>::ResourceLoader(const Loader& loader)
	: m_loader(loader) { }

template<class R>
inline ResourceLoader<R>::ResourceLoader(Loader&& loader)
	: m_loader(std::move(loader)) { }

template<class R>
inline std::unique_ptr<R> ResourceLoader<R>::operator()() const
{
	return m_loader();
}

template<class R>
inline std::unique_ptr<R> ResourceLoader<R>::Load() const
{
	return m_loader();
}

template<class R, typename Func>
inline ResourceLoader<R> MakeResourceLoader(Func&& func)
{
	return ResourceLoader<R>(
		[func = std::forward<Func>(func)]() -> std::unique_ptr<R>
		{
			auto resource = std::make_unique<R>();

			if (func(*resource))
				return resource;

			return nullptr;
		});
}