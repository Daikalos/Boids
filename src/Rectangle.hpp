#pragma once

#pragma warning(disable : 4201)

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

template<typename T>
class Rect
{
public:
	constexpr Rect();
	constexpr Rect(T left, T top, T width, T height);
	constexpr Rect(const sf::Vector2<T>& position, const sf::Vector2<T>& size);
	constexpr Rect(const sf::Rect<T>& rhs);

	template<typename U>
	constexpr explicit Rect(const Rect<U>& rhs);
	template<typename U>
	constexpr explicit Rect(const sf::Rect<U>& rhs);

public:
	constexpr operator sf::Rect<T>&();
	constexpr operator const sf::Rect<T>&() const;

	[[nodiscard]] constexpr Rect operator-() const;

	constexpr Rect& operator+=(const Rect& rhs);
	constexpr Rect& operator-=(const Rect& rhs);

	constexpr Rect& operator+=(const sf::Vector2<T>& rhs);
	constexpr Rect& operator-=(const sf::Vector2<T>& rhs);

	constexpr Rect& operator*=(const float rhs);

	[[nodiscard]] constexpr Rect operator+(const Rect& rhs) const;
	[[nodiscard]] constexpr Rect operator-(const Rect& rhs) const;

	[[nodiscard]] constexpr Rect operator+(const sf::Vector2<T>& rhs) const;
	[[nodiscard]] constexpr Rect operator-(const sf::Vector2<T>& rhs) const;

	[[nodiscard]] constexpr Rect operator*(const float rhs) const;

	[[nodiscard]] constexpr bool operator==(const Rect& rhs) const;
	[[nodiscard]] constexpr bool operator!=(const Rect& rhs) const;

public:
	[[nodiscard]] constexpr T Right() const;
	[[nodiscard]] constexpr T Bottom() const;

	[[nodiscard]] constexpr sf::Vector2<T> Position() const;
	[[nodiscard]] constexpr sf::Vector2<T> Size() const;
	[[nodiscard]] constexpr sf::Vector2<T> Center() const;

	[[nodiscard]] constexpr T Area() const;

	[[nodiscard]] constexpr Rect Union(const Rect& other) const;
	[[nodiscard]] constexpr std::optional<Rect> Intersection(const Rect& other) const;

	[[nodiscard]] constexpr bool Overlaps(const Rect& other) const;

	[[nodiscard]] constexpr bool Contains(const sf::Vector2<T>& point) const;
	[[nodiscard]] constexpr bool Contains(const Rect& other) const;

	union
	{
		sf::Rect<T> rect{};
		struct
		{
			T left, top, width, height;
		};
	};
};

using RectInt = Rect<int>;
using RectFloat = Rect<float>;

template<typename T>
inline constexpr Rect<T>::Rect() = default;

template<typename T>
inline constexpr Rect<T>::Rect(T left, T top, T width, T height)
	: rect({ left, top }, { width, height }) { };

template<typename T>
inline constexpr Rect<T>::Rect(const sf::Vector2<T>& position, const sf::Vector2<T>& size)
	: rect({ position.x, position.y }, { size.x, size.y }) { };

template<typename T>
inline constexpr Rect<T>::Rect(const sf::Rect<T>& rhs)
	: rect(rhs) { };

template<typename T>
template<typename U>
inline constexpr Rect<T>::Rect(const Rect<U>& rhs)
	: rect({ rhs.left, rhs.top }, { rhs.width, rhs.height }) { };

template<typename T>
template<typename U>
inline constexpr Rect<T>::Rect(const sf::Rect<U>& rhs)
	: rect(rhs) { };

template<typename T>
inline constexpr Rect<T>::operator sf::Rect<T>& ()
{
	return rect;
}

template<typename T>
inline constexpr Rect<T>::operator const sf::Rect<T>& () const
{
	return rect;
}

template<typename T>
inline constexpr bool Rect<T>::operator==(const Rect& rhs) const
{
	return (rect == rhs.rect);
}

template<typename T>
inline constexpr bool Rect<T>::operator!=(const Rect& rhs) const
{
	return (rect != rhs.rect);
}

template<typename T>
inline constexpr T Rect<T>::Right() const
{
	return static_cast<T>(left + width);
}
template<typename T>
inline constexpr T Rect<T>::Bottom() const
{
	return static_cast<T>(top + height);
}

template<typename T>
inline constexpr sf::Vector2<T> Rect<T>::Position() const
{
	return rect.position;
}

template<typename T>
inline constexpr sf::Vector2<T> Rect<T>::Size() const
{
	return rect.size;
}

template<typename T>
inline constexpr sf::Vector2<T> Rect<T>::Center() const
{
	return Position() + (Size() / T(2));
}

template<typename T>
constexpr T Rect<T>::Area() const
{
	return width * height;
}

template<typename T>
inline constexpr Rect<T> Rect<T>::Union(const Rect& other) const
{
	// returns a rectangle that encompasses both rects

	const auto min = [](T a, T b) { return (a < b) ? a : b; };
	const auto max = [](T a, T b) { return (a < b) ? b : a; };

	const T r1l = min(left, Right());
	const T r1t = min(top, Bottom());
	const T r1r = max(left, Right());
	const T r1b = max(top, Bottom());

	const T r2l = min(other.left, other.Right());
	const T r2t = min(other.top, other.Bottom());
	const T r2r = max(other.left, other.Right());
	const T r2b = max(other.top, other.Bottom());

	const T r3l = min(r1l, r2l);
	const T r3t = min(r1t, r2t);
	const T r3r = max(r1r, r2r);
	const T r3b = max(r1b, r2b);

	return Rect<T>(r3l, r3t, r3r - r3l, r3b - r3t);
}
template<typename T>
inline constexpr std::optional<Rect<T>> Rect<T>::Intersection(const Rect& other) const
{
	return rect.findIntersection(other);
}

template<typename T>
inline constexpr bool Rect<T>::Overlaps(const Rect& other) const
{
	const auto min = [](T a, T b) { return (a < b) ? a : b; };
	const auto max = [](T a, T b) { return (a < b) ? b : a; };

	const T r1l = min(left, Right());
	const T r1t = min(top, Bottom());
	const T r1r = max(left, Right());
	const T r1b = max(top, Bottom());

	const T r2l = min(other.left, other.Right());
	const T r2t = min(other.top, other.Bottom());
	const T r2r = max(other.left, other.Right());
	const T r2b = max(other.top, other.Bottom());

	return !(r1l > r2r || r2l > r1r || r1t > r2b || r2t > r1b);
}

template<typename T>
inline constexpr bool Rect<T>::Contains(const sf::Vector2<T>& point) const
{
	return rect.contains(point);
}

template<typename T>
inline constexpr bool Rect<T>::Contains(const Rect& other) const
{
	const auto min = [](T a, T b) { return (a < b) ? a : b; };
	const auto max = [](T a, T b) { return (a < b) ? b : a; };

	const T r1l = min(left, Right());
	const T r1t = min(top, Bottom());
	const T r1r = max(left, Right());
	const T r1b = max(top, Bottom());

	const T r2l = min(other.left, other.Right());
	const T r2t = min(other.top, other.Bottom());
	const T r2r = max(other.left, other.Right());
	const T r2b = max(other.top, other.Bottom());

	return (r2l >= r1l && r2r < r1r && r2t >= r1t && r2b < r1b);
}

template<typename T>
inline constexpr Rect<T> Rect<T>::operator-() const
{
	return Rect(-left, -top, -width, -height);
}

template<typename T>
inline constexpr Rect<T>& Rect<T>::operator+=(const Rect& rhs)
{
	left += rhs.left;
	top += rhs.top;
	width += rhs.width;
	height += rhs.height;

	return *this;
}
template<typename T>
inline constexpr Rect<T>& Rect<T>::operator-=(const Rect& rhs)
{
	return (*this += -rhs);
}

template<typename T>
inline constexpr Rect<T> Rect<T>::operator+(const Rect& rhs) const
{
	return Rect(*this) += rhs;
}
template<typename T>
inline constexpr Rect<T> Rect<T>::operator-(const Rect& rhs) const
{
	return Rect(*this) += -rhs;
}

template<typename T>
inline constexpr Rect<T>& Rect<T>::operator+=(const sf::Vector2<T>& rhs)
{
	left += rhs.x;
	top += rhs.y;

	return *this;
}
template<typename T>
inline constexpr Rect<T>& Rect<T>::operator-=(const sf::Vector2<T>& rhs)
{
	return (*this += -rhs);
}

template<typename T>
inline constexpr Rect<T>& Rect<T>::operator*=(const float rhs)
{
	width *= rhs;
	height *= rhs;

	return *this;
}

template<typename T>
inline constexpr Rect<T> Rect<T>::operator+(const sf::Vector2<T>& rhs) const
{
	return Rect<T>(*this) += rhs;
}
template<typename T>
inline constexpr Rect<T> Rect<T>::operator-(const sf::Vector2<T>& rhs) const
{
	return Rect<T>(*this) += -rhs;
}

template<typename T>
inline constexpr Rect<T> Rect<T>::operator*(const float rhs) const
{
	return Rect<T>(*this) *= rhs;
}