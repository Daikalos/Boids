#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "../utilities/NonCopyable.h"
#include "../utilities/VectorUtilities.h"

#include "InputHandler.h"

class Camera : public sf::View, NonCopyable
{
public:
	Camera();

public:
	[[nodiscard]] const sf::Vector2f& GetPosition() const;
	[[nodiscard]] const sf::Vector2f& GetScale() const;
	[[nodiscard]] const sf::Vector2f& GetSize() const;

	sf::Vector2f ViewToWorld(const sf::Vector2f& position) const;

	[[nodiscard]] const float* GetWorldMatrix() const;
	[[nodiscard]] const sf::Transform& GetViewMatrix() const;

	[[nodiscard]] sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window) const;

	void SetPosition(const sf::Vector2f& position);
	void SetScale(const sf::Vector2f& scale);
	void SetSize(const sf::Vector2f& size);

public:
	// call after poll event
	//
	void Update(const InputHandler& input_handler, const sf::RenderWindow& window);
	void HandleEvent(const sf::Event& event);

private:
	sf::Vector2f m_position;
	sf::Vector2f m_scale;
	sf::Vector2f m_size;

	mutable sf::Transform m_viewTransform;
	mutable sf::Transform m_worldTransform;
	mutable bool m_updateView	{true};
	mutable bool m_updateWorld	{true};

	sf::Vector2f m_dragPos;
};

