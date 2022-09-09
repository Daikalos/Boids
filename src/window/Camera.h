#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "../utilities/NonCopyable.h"
#include "../utilities/VectorUtilities.h"

#include "InputHandler.h"

class Camera : public sf::View, NonCopyable
{
public:
	Camera() 
		: _position(0, 0), _scale(1.0f, 1.0f), _size(0, 0), _drag_pos(0, 0) {}

	// call after poll event
	//
	void update(const InputHandler& input_handler, const sf::RenderWindow& window);
	void handle_event(const sf::Event& event);

public:
	template<typename T> 
	sf::Vector2<T> view_to_world(const sf::Vector2<T>& position) const
	{
		return sf::Vector2<T>(get_view_matrix() * position);
	}

	[[nodiscard]] const float* get_world_matrix() const
	{
		if (_update_world)
		{
			_world_transform = sf::Transform()
				.translate(_size / 2.0f)
				.scale(_scale)
				.translate(-_position);

			_update_world = true;
		}

		return _world_transform.getMatrix();
	}

	[[nodiscard]] const sf::Transform& get_view_matrix() const
	{
		if (_update_view)
		{
			_view_transform = sf::Transform()
				.translate(_position)
				.scale(1.0f / _scale)
				.translate(_size / -2.0f);

			_update_view = false;
		}

		return _view_transform;
	}

	[[nodiscard]] sf::Vector2f get_mouse_world_position(const sf::RenderWindow& window) const 
	{ 
		return view_to_world(sf::Vector2f(sf::Mouse::getPosition(window)));
	}

	[[nodiscard]] constexpr sf::Vector2f get_position() const { return _position; }
	[[nodiscard]] constexpr sf::Vector2f get_scale() const { return _scale; }
	[[nodiscard]] constexpr sf::Vector2f get_size() const { return _size; }

	void set_position(const sf::Vector2f& position)
	{
		setCenter(position);
		_position = position;

		_update_view = true;
		_update_world = true;
	}
	void set_scale(const sf::Vector2f& scale)
	{
		setSize(_size * (1.0f / scale));
		_scale = scale;

		_update_view = true;
		_update_world = true;
	}
	void set_size(const sf::Vector2f& size) 
	{ 
		setSize(size * (1.0f / _scale));
		_size = size;

		_update_view = true;
		_update_world = true;
	}

private:
	sf::Vector2f _position;
	sf::Vector2f _scale;
	sf::Vector2f _size;

	mutable sf::Transform _view_transform;
	mutable sf::Transform _world_transform;
	mutable bool _update_view{true};
	mutable bool _update_world{true};

	sf::Vector2f _drag_pos;
};

