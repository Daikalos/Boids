#include "Debug.h"

void Debug::load(const FontHolder& font_holder)
{
	if (font_holder.exists(FontID::F8Bit))
	{
		debug_text_state.setFont(font_holder.get(FontID::F8Bit));
		debug_text_info.setFont(font_holder.get(FontID::F8Bit));
	}

	debug_text_state.setPosition(sf::Vector2f(32, 32));
	debug_text_state.setCharacterSize(26);

	debug_text_info.setPosition(sf::Vector2f(48, 48));
	debug_text_info.setCharacterSize(24);

	debug_text_state.setString(get_state() + " (PRESS THE TOGGLE KEY TO ENABLE)");
	debug_text_info.setString("");
}

void Debug::update(const InputHandler& input_handler, float dt)
{
	refresh = false;

	if (!config->debug_enabled)
		return;

	if (input_handler.get_key_pressed(static_cast<sf::Keyboard::Key>(config->debug_toggle_key)))
		toggle();

	if (!enabled)
		return;

	update_freq -= dt;
	if (update_freq <= 0.0f)
	{
		debug_info = 
			"\nCONFIG STATUS: " + std::string(config->load_status ? "SUCCESS" : "FAILED TO LOAD") +
			"\n\nBOIDS: " + std::to_string(config->boid_count) +
			"\nFPS: " + std::to_string((int)(1.0f / dt));

		refresh = true;
		update_freq = update_freq_max;
	}

	debug_text_info.setString(
		"\nCONFIG REFRESH: " + util::remove_trailing_zeroes(std::to_string(util::set_precision(update_freq, 2))) + debug_info);
}

void Debug::draw(sf::RenderWindow& window) const
{
	if (!config->debug_enabled)
		return;

	window.pushGLStates();

	window.draw(debug_text_state);
	window.draw(debug_text_info);

	window.popGLStates();
}
