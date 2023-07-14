#include "Background.h"

#include "Config.h"

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

	if (Config::Inst().Background.FitScreen)
	{
		desiredScale = sf::Vector2f(
			size.x / m_background.getLocalBounds().width,
			size.y / m_background.getLocalBounds().height);
	}
	else if (Config::Inst().Background.OverrideSize)
	{
		desiredScale = sf::Vector2f(
			Config::Inst().Background.Width / m_background.getLocalBounds().width,
			Config::Inst().Background.Height / m_background.getLocalBounds().height);
	}

	bool setColor = 
		Config::Inst().Background.Color.x > FLT_EPSILON || 
		Config::Inst().Background.Color.y > FLT_EPSILON || 
		Config::Inst().Background.Color.z > FLT_EPSILON;

	m_background.setPosition(sf::Vector2f(
		(float)Config::Inst().Background.PositionX,
		(float)Config::Inst().Background.PositionY));

	m_background.setScale(desiredScale);

	m_background.setColor(setColor ? sf::Color(
		static_cast<sf::Uint8>(Config::Inst().Background.Color.x * 255.0f),
		static_cast<sf::Uint8>(Config::Inst().Background.Color.y * 255.0f),
		static_cast<sf::Uint8>(Config::Inst().Background.Color.z * 255.0f)) : sf::Color::White);
}

void Background::Draw(sf::RenderWindow& window) const
{
	if (m_background.getTexture()->getSize().x == 0U ||
		m_background.getTexture()->getSize().y == 0U)
		return;

	window.draw(m_background);
}