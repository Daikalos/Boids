#include "Camera.h"

#include "InputHandler.h"

#include "VectorUtilities.hpp"

Camera::Camera() 
	: m_position(0, 0)
	, m_scale(1.0f, 1.0f)
	, m_size(0, 0)
	, m_dragPos(0, 0) {}

const sf::Vector2f& Camera::GetPosition() const { return m_position; }
const sf::Vector2f& Camera::GetScale() const { return m_scale; }
const sf::Vector2f& Camera::GetSize() const { return m_size; }

sf::Vector2f Camera::WorldToView(const sf::Vector2f& position) const
{
	return GetViewMatrix() * position;
}

const float* Camera::GetWorldMatrix() const
{
	if (m_updateWorld)
	{
		m_worldTransform = sf::Transform()
			.translate(m_size / 2.0f)
			.scale(m_scale)
			.translate(-m_position);

		m_updateWorld = false;
	}

	return m_worldTransform.getMatrix();
}

const sf::Transform& Camera::GetViewMatrix() const
{
	if (m_updateView)
	{
		m_viewTransform = sf::Transform()
			.translate(m_position)
			.scale(1.0f / m_scale)
			.translate(m_size / -2.0f);

		m_updateView = false;
	}

	return m_viewTransform;
}

sf::Vector2f Camera::GetMouseWorldPosition(const sf::RenderWindow& window) const
{
	return WorldToView(sf::Vector2f(sf::Mouse::getPosition(window)));
}

void Camera::SetPosition(const sf::Vector2f& position)
{
	if (m_position != position)
	{
		setCenter(position);
		m_position = position;

		m_updateView = true;
		m_updateWorld = true;
	}
}
void Camera::SetScale(const sf::Vector2f& scale)
{
	if (m_scale != scale)
	{
		setSize(m_size * (1.0f / scale));
		m_scale = scale;

		m_updateView = true;
		m_updateWorld = true;
	}
}
void Camera::SetSize(const sf::Vector2f& size)
{
	if (m_size != size)
	{
		setSize(size * (1.0f / m_scale));
		m_size = size;

		m_updateView = true;
		m_updateWorld = true;
	}
}

void Camera::Update(const InputHandler& inputHandler, const sf::RenderWindow& window)
{
	if (inputHandler.GetButtonPressed(sf::Mouse::Button::Middle))
		m_dragPos = GetMouseWorldPosition(window);
	if (inputHandler.GetButtonHeld(sf::Mouse::Button::Middle))
		m_position += (m_dragPos - GetMouseWorldPosition(window));

	SetPosition(m_position);
}

void Camera::HandleEvent(const sf::Event& event)
{
	if (const auto* resized = event.getIf<sf::Event::Resized>())
	{
		SetSize(sf::Vector2f((float)resized->size.x, (float)resized->size.y));
		SetPosition(GetSize() / 2.0f);
	}
}
