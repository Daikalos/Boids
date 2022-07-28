#pragma once

#include <iostream>
#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "Config.h"

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	sf::Texture* request_texture(std::string name);
	sf::Font* request_font(std::string name);

	void load_textures();
	void load_fonts();

	void clean_up();

private:
	void load_texture(std::string path, std::string name);
	void load_font(std::string path, std::string name);

	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts;
};

