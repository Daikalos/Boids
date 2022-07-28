#pragma once

#include <SFML/Graphics.hpp>

#include "ResourceManager.h"
#include "InputHandler.h"

class Debug
{
public:
	Debug() : update_freq_max(Config::debug_update_freq), update_freq(0.0f) { }
	~Debug() {}

	void load(const ResourceManager& resourceManager);
	void update(const InputHandler& inputHandler, const float& deltaTime);
	void draw(sf::RenderWindow& renderWindow);

	inline void toggle()
	{
		enabled = !enabled;
		debug_text_state.setString(get_state());
	}

private:
	std::string get_state() const
	{
		return enabled ? "DEBUG ENABLED" : "DEBUG DISABLED";
	}

private:
	bool enabled{false};

	sf::Text debug_text_state;
	sf::Text debug_text_info;

	float update_freq_max;
	float update_freq;
};

