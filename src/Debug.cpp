#include "Debug.h"

Debug::Debug(Config& config)
	: config(&config), update_freq_max(this->config->debug_update_freq), update_freq(0.0f)
{

}

Debug::~Debug()
{

}

void Debug::load(const ResourceManager& resource_manager)
{
	sf::Font* font = resource_manager.request_font("8bit");

	if (font != nullptr)
	{
		debug_text_state.setFont(*font);
		debug_text_info.setFont(*font);
	}

	debug_text_state.setPosition(sf::Vector2f(32, 32));
	debug_text_state.setCharacterSize(26);

	debug_text_info.setPosition(sf::Vector2f(32, 64));
	debug_text_info.setCharacterSize(24);

	debug_text_state.setString(get_state());
	debug_text_info.setString("\nFPS: 0\nBOIDS: " + std::to_string(config->boid_count));
}

void Debug::update(const InputHandler& input_handler, const float& dt)
{
	refresh = false;

	if (!config->debug_enabled)
		return;

	if (input_handler.get_key_pressed(static_cast<sf::Keyboard::Key>(config->debug_toggle_key)))
		toggle();

	update_freq -= dt;

	if (update_freq <= 0.0f)
	{
		debug_text_info.setString(
			"\nFPS: " + std::to_string((int)(1.0f / dt)) +
			"\nBOIDS: " + std::to_string(config->boid_count));

		update_freq = update_freq_max;

		if (enabled)
			refresh = true;
	}
}

void Debug::draw(sf::RenderWindow& renderWindow)
{
	if (!config->debug_enabled)
		return;

	renderWindow.draw(debug_text_state);
	renderWindow.draw(debug_text_info);
}