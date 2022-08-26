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

	const float* get_world_matrix() const
	{
		return sf::Transform()
			.translate(_size / 2.0f)
			.scale(_scale)
			.translate(-_position).getMatrix();
	}

	sf::Transform get_view_matrix() const
	{
		return sf::Transform()
			.translate(_position)
			.scale(1.0f / _scale)
			.translate(_size / -2.0f);
	}

	sf::Vector2f get_mouse_world_position(const sf::RenderWindow& window) const { return view_to_world(sf::Vector2f(sf::Mouse::getPosition(window))); }

	sf::Vector2f get_position() const { return _position; }
	sf::Vector2f get_scale() const { return _scale; }
	sf::Vector2f get_size() const { return _size; }

	void set_position(const sf::Vector2f& position)
	{
		_position = position;
		setCenter(position);
	}
	void set_scale(const sf::Vector2f& scale)
	{
		_scale = scale;
		setSize(_size * (1.0f / scale));
	}
	void set_size(const sf::Vector2f& size) 
	{ 
		_size = size;
		setSize(size * (1.0f / _scale));
	}

private:
	sf::Vector2f _position;
	sf::Vector2f _scale;
	sf::Vector2f _size;

	sf::Vector2f _drag_pos;
};

