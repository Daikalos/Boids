#pragma once

#include <SFML/Graphics.hpp>

#include "../window/ResourceHolder.hpp"
#include "../window/InputHandler.h"
#include "../utilities/NonCopyable.h"

#include "Config.h"

class Debug : public NonCopyable
{
public:
	Debug(Config& config);

	void set_update_freq(float value);

	[[nodiscard]] bool get_refresh() const noexcept;
	[[nodiscard]] std::string get_state() const noexcept;

public:
	void load(const FontHolder& _font_holder);
	void update(const InputHandler& input_handler, float dt);
	void draw(sf::RenderWindow& window) const;

private:
	void toggle();

private:
	const Config*	_config				{nullptr};

	float			_update_freq_max	{0.0f};
	float			_update_freq		{0.0f};

	bool			_enabled			{false};
	bool			_refresh			{false};

	sf::Text		_debug_text_state;
	sf::Text		_debug_text_info;
	std::string		_debug_info;
};

