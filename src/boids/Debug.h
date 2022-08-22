#pragma once

#include <SFML/Graphics.hpp>

#include "../window/ResourceHolder.hpp"
#include "../window/InputHandler.h"
#include "../utilities/NonCopyable.h"

#include "Config.h"

class Debug : public NonCopyable
{
public:
	Debug(Config& config)
		: _config(&config), _update_freq_max(config.debug_update_freq) { }

	void load(const FontHolder& _font_holder);
	void update(const InputHandler& input_handler, float dt);
	void draw(sf::RenderWindow& window) const;

	void set_update_freq(float value)
	{
		_update_freq_max = value;
	}

	inline bool get_refresh() const
	{
		return _refresh;
	}

private:
	void toggle()
	{
		_enabled = !_enabled;

		_debug_text_state.setString(get_state());
		_debug_text_info.setString(_enabled ? _debug_text_info.getString() : "");
	}

	inline std::string get_state() const
	{
		return _enabled ? "DEBUG ENABLED" : "DEBUG DISABLED";
	}

private:
	Config*		_config;

	float		_update_freq_max{0.0f};
	float		_update_freq{0.0f};

	bool		_enabled{false};
	bool		_refresh{false};

	sf::Text	_debug_text_state;
	sf::Text	_debug_text_info;
	std::string _debug_info;
};

