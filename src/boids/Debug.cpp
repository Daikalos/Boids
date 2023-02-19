#include "Debug.h"

Debug::Debug() : _update_freq_max(Config::GetInstance().debug_update_freq) 
{ 

}

void Debug::set_update_freq(float value)
{
	_update_freq_max = value;
}

bool Debug::get_refresh() const noexcept
{
	return _refresh;
}

std::string Debug::get_state() const noexcept
{
	return _enabled ? "DEBUG ENABLED" : "DEBUG DISABLED";
}

void Debug::load(const FontHolder& _font_holder)
{
	if (_font_holder.exists(FontID::F8Bit))
	{
		_debug_text_state.setFont(_font_holder.get(FontID::F8Bit));
		_debug_text_info.setFont(_font_holder.get(FontID::F8Bit));
	}

	_debug_text_state.setPosition(sf::Vector2f(32, 32));
	_debug_text_state.setCharacterSize(26);

	_debug_text_info.setPosition(sf::Vector2f(48, 48));
	_debug_text_info.setCharacterSize(24);

	_debug_text_state.setString(get_state() + " (PRESS THE TOGGLE KEY TO ENABLE)");
	_debug_text_info.setString("");
}

void Debug::update(const InputHandler& input_handler, float dt)
{
	_refresh = false;

	if (!Config::GetInstance().debug_enabled)
		return;

	if (input_handler.get_key_pressed(static_cast<sf::Keyboard::Key>(Config::GetInstance().debug_toggle_key)))
		toggle();

	if (!_enabled)
		return;

	_update_freq -= dt;
	if (_update_freq <= 0.0f)
	{
		_debug_info = 
			"\nCONFIG STATUS: " + std::string(Config::GetInstance().load_status ? "SUCCESS" : "FAILED TO LOAD") +
			"\n\nBOIDS: " + std::to_string(Config::GetInstance().boid_count) +
			"\nFPS: " + std::to_string((int)(1.0f / dt));

		_refresh = true;
		_update_freq = Config::GetInstance().debug_update_freq;
	}

	_debug_text_info.setString(
		"\nCONFIG REFRESH: " + util::remove_trailing_zeroes(std::to_string(util::set_precision(_update_freq, 2))) + _debug_info);
}

void Debug::draw(sf::RenderWindow& window) const
{
	if (!Config::GetInstance().debug_enabled)
		return;

	window.draw(_debug_text_state);
	window.draw(_debug_text_info);
}

void Debug::toggle()
{
	_enabled = !_enabled;

	_debug_text_state.setString(get_state());
	_debug_text_info.setString(_enabled ? _debug_text_info.getString() : "");
}
