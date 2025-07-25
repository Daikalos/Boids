#include "Background.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include "WindowsUtilities.h"
#include "SFMLLoaders.hpp"

#include "Config.h"

Background::Background() : m_background(DEFAULT_TEXTURE)
{

}

void Background::Load(TextureHolder& textureHolder, const sf::Vector2i& size)
{
	if (m_loadAsync.valid())
		m_loadAsync.wait();

	m_loadAsync = std::async(std::launch::async,
		[this, &textureHolder, &size]()
		{
			try
			{
				if (Config::Inst().Background.UseWallpaper)
				{
					std::wstring wallpaperPath = GetWallpaperPath();

					textureHolder.Acquire(TextureID::Background,
						FromFile<sf::Texture>(wallpaperPath), res::LoadStrategy::Reload);
				}
				else
				{
					textureHolder.Acquire(TextureID::Background,
						FromFile<sf::Texture>(RESOURCE_FOLDER + Config::Inst().Background.Texture), res::LoadStrategy::Reload);
				}
			}
			catch (std::runtime_error e)
			{

			}

			LoadTexture(textureHolder);
			LoadProperties(size);
		});
}

void Background::LoadTexture(const TextureHolder& textureHolder)
{
	if (textureHolder.Contains(TextureID::Background))
	{
		m_background.setTexture(textureHolder.Get(TextureID::Background), true);
	}
	else
	{
		m_background.setTextureRect(sf::IntRect{});
	}
}

void Background::LoadProperties(const sf::Vector2i& size)
{
	sf::Vector2f desiredScale = sf::Vector2f(1.0f, 1.0f);

	if (Config::Inst().Background.FitScreen)
	{
		desiredScale = sf::Vector2f(
			size.x / m_background.getLocalBounds().size.x,
			size.y / m_background.getLocalBounds().size.y);
	}
	else if (Config::Inst().Background.OverrideSize)
	{
		desiredScale = sf::Vector2f(
			Config::Inst().Background.Width / m_background.getLocalBounds().size.x,
			Config::Inst().Background.Height / m_background.getLocalBounds().size.y);
	}

	m_background.setPosition(sf::Vector2f(Config::Inst().Background.Position));
	m_background.setScale(desiredScale);

	bool setColor =
		Config::Inst().Background.Color.x > FLT_EPSILON ||
		Config::Inst().Background.Color.y > FLT_EPSILON ||
		Config::Inst().Background.Color.z > FLT_EPSILON;

	m_background.setColor(setColor ? sf::Color(
		(std::uint8_t)(Config::Inst().Background.Color.x * 255.999f),
		(std::uint8_t)(Config::Inst().Background.Color.y * 255.999f),
		(std::uint8_t)(Config::Inst().Background.Color.z * 255.999f)) : sf::Color::White);
}

void Background::Draw(sf::RenderWindow& window) const
{
	if (m_background.getTexture().getSize().x == 0U ||
		m_background.getTexture().getSize().y == 0U)
		return;

	window.draw(m_background);
}