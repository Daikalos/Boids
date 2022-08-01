#pragma once

#include <SFML/Graphics.hpp>

#include "ResourceManager.h"
#include "InputHandler.h"

class Debug
{
public:
	Debug(Config& config);
	~Debug();

	void load(const ResourceManager& resource_manager);
	bool update(const InputHandler& input_handler, const float& dt);
	void draw(sf::RenderWindow& window);

	inline void toggle()
	{
		enabled = !enabled;
		debug_text_state.setString(get_state());
	}

	inline void set_update_freq(float value)
	{
		this->update_freq_max = value;
	}

private:
	std::string get_state() const
	{
		return enabled ? "DEBUG ENABLED" : "DEBUG DISABLED";
	}

private:
	Config& config;

	float update_freq_max;
	float update_freq;

	bool enabled{false};

	sf::Text debug_text_state;
	sf::Text debug_text_info;
};

