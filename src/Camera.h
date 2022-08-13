#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Config.h"

#include "InputHandler.h"
#include "NonCopyable.h"

#include "VecUtil.h"

class Camera : public sf::View, NonCopyable
{
public:
	Camera(Config& config) 
		: config(&config), position(0, 0), scale(config.camera_zoom, config.camera_zoom), size(0, 0), drag_pos(0, 0) {}
	~Camera() {}

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
			.translate(size / 2.0f)
			.scale(scale)
			.translate(-position).getMatrix();
	}

	sf::Transform get_view_matrix() const
	{
		return sf::Transform()
			.translate(position)
			.scale(1.0f / scale)
			.translate(size / -2.0f);
	}

	sf::Vector2f get_mouse_world_position(const sf::RenderWindow& window) const { return view_to_world(sf::Vector2f(sf::Mouse::getPosition(window))); }

	sf::Vector2f get_position() const { return position; }
	sf::Vector2f get_scale() const { return scale; }
	sf::Vector2f get_size() const { return size; }

	void set_position(sf::Vector2f position)
	{
		setCenter(position);
		this->position = position;
	}
	void set_scale(sf::Vector2f scale)
	{
		setSize(size * (1.0f / scale));
		this->scale = scale;
	}
	void set_size(sf::Vector2f size) 
	{ 
		this->size = size;
		setSize(size);
	}

private:
	Config* config;

	sf::Vector2f position;
	sf::Vector2f scale;
	sf::Vector2f size;

	sf::Vector2f drag_pos;
};

