#pragma once

#include <string>
#include <filesystem>

#include "ResourceLoader.hpp"

template<class R> 
	requires requires (R res) { res.loadFromFile(std::filesystem::path()); }
inline ResourceLoader<R> FromFile(const std::filesystem::path& path)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.loadFromFile(path);
		});
}

template<class R, typename T> 
	requires requires (R res) { res.loadFromFile(std::filesystem::path(), T()); }
inline ResourceLoader<R> FromFile(const std::filesystem::path& path, T arg)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.loadFromFile(path, arg);
		});
}

template<class R> 
	requires requires (R res) { res.openFromFile(std::filesystem::path()); }
inline ResourceLoader<R> OpenFile(const std::filesystem::path& path)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.openFromFile(path);
		});
}

template<class R, typename T> 
	requires requires (R res) { res.openFromFile(std::filesystem::path(), T()); }
inline ResourceLoader<R> OpenFile(const std::filesystem::path& path, T arg)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.openFromFile(path, arg);
		});
}

template<class R, typename T, typename U> 
	requires requires (R res) { res.loadFromMemory(T(), U()); }
inline ResourceLoader<R> FromMemory(T arg1, U arg2)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.loadFromMemory(arg1, arg2);
		});
}

template<class R, typename T, typename U, typename V>
	requires requires (R res) { res.loadFromMemory(T(), U(), V()); }
inline ResourceLoader<R> FromMemory(T arg1, U arg2, V arg3)
{
	return MakeResourceLoader<R>(
		[=](R& resource)
		{
			return resource.loadFromMemory(arg1, arg2, arg3);
		});
}