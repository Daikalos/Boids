#include "Background.h"

void Background::LoadTexture(const TextureHolder& textureHolder)
{
	if (textureHolder.Exists(TextureID::Background))
		m_background.setTexture(textureHolder.Get(TextureID::Background), true);
	else
		m_background.setTexture(sf::Texture(), true);
}

void Background::LoadProperties(const sf::Vector2i& size)
{
	sf::Vector2f desiredScale = sf::Vector2f(1.0f, 1.0f);

	if (Config::Inst().BackgroundFitScreen)
	{
		desiredScale = sf::Vector2f(
			size.x / m_background.getLocalBounds().width,
			size.y / m_background.getLocalBounds().height);
	}
	else if (Config::Inst().BackgroundOverrideSize)
	{
		desiredScale = sf::Vector2f(
			Config::Inst().BackgroundWidth / m_background.getLocalBounds().width,
			Config::Inst().BackgroundHeight / m_background.getLocalBounds().height);
	}

	bool setColor = 
		Config::Inst().BackgroundColor.x > FLT_EPSILON || 
		Config::Inst().BackgroundColor.y > FLT_EPSILON || 
		Config::Inst().BackgroundColor.z > FLT_EPSILON;

	m_background.setPosition(sf::Vector2f(
		(float)Config::Inst().BackgroundPositionX,
		(float)Config::Inst().BackgroundPositionY));

	m_background.setScale(desiredScale);

	m_background.setColor(setColor ? sf::Color(
		(sf::Uint8)(Config::Inst().BackgroundColor.x * 255.0f),
		(sf::Uint8)(Config::Inst().BackgroundColor.y * 255.0f),
		(sf::Uint8)(Config::Inst().BackgroundColor.z * 255.0f)) : sf::Color::White);
}

void Background::Draw(sf::RenderWindow& window) const
{
	if (m_background.getTexture()->getSize().x == 0U ||
		m_background.getTexture()->getSize().y == 0U)
		return;

	window.draw(m_background);
}