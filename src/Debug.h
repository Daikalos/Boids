#pragma once

#include <SFML/Graphics.hpp>

#include "ResourceHolder.hpp"
#include "InputHandler.h"
#include "Config.h"

class Debug
{
public:
	Debug(Config& config);
	~Debug();

	void load(const FontHolder& font_holder);
	void update(const InputHandler& input_handler, const float& dt);
	void draw(sf::RenderWindow& window) const;

	void set_update_freq(float value)
	{
		this->update_freq_max = value;
	}

	inline bool get_refresh() const
	{
		return refresh;
	}

private:
	void toggle()
	{
		enabled = !enabled;
		debug_text_state.setString(get_state());
		debug_text_info.setString(enabled ? debug_text_info.getString() : "");
	}

	inline std::string get_state() const
	{
		return enabled ? "DEBUG ENABLED" : "DEBUG DISABLED";
	}

private:
	Config* config;

	float update_freq_max;
	float update_freq;

	bool enabled{false};
	bool refresh{false};

	sf::Text debug_text_state;
	sf::Text debug_text_info;
	std::string debug_info;
};

