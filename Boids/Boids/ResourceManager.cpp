#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	clean_up();
}

sf::Texture* ResourceManager::request_texture(std::string name, sf::Texture* fallback) const
{
	auto it = textures.find(name);

	if (it != textures.end())
		return it->second.get();
	
	return fallback;
}

sf::Font* ResourceManager::request_font(std::string name, sf::Font* fallback) const
{
	auto it = fonts.find(name);

	if (it != fonts.end())
		return it->second.get();

	return fallback;
}

void ResourceManager::load_textures()
{
	std::string path;

	path = "content/" + Config::background_texture;
	load_texture(path, "background");
}
void ResourceManager::load_fonts()
{
	std::string path;

	path = "content/font_8bit.ttf";
	load_font(path, "8bit");
}

void ResourceManager::clean_up()
{
	textures.clear();
	fonts.clear();
}

void ResourceManager::load_texture(std::string path, std::string name)
{
	std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();

	if (!texture->loadFromFile(path))
		return;

	textures[name] = texture;
}
void ResourceManager::load_font(std::string path, std::string name)
{
	std::shared_ptr<sf::Font> font = std::make_shared<sf::Font>();

	if (!font->loadFromFile(path))
		return;

	fonts[name] = font;
}
