#include "Debug.h"

void Debug::load(const ResourceManager& resourceManager)
{
	sf::Font* font = resourceManager.request_font("8bit");

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
	debug_text_info.setString("\nFPS: 0\nBOIDS: " + std::to_string(Config::boid_count));
}

void Debug::update(const InputHandler& inputHandler, const float& deltaTime)
{
	if (inputHandler.get_key_pressed(static_cast<sf::Keyboard::Key>(Config::debug_toggle_key)))
		toggle();

	update_freq -= deltaTime;

	if (update_freq <= 0.0f)
	{
		if (enabled)
			Config::refresh();

		debug_text_info.setString(
			"\nFPS: " + std::to_string((int)(1.0f / deltaTime)) +
			"\nBOIDS: " + std::to_string(Config::boid_count));

		update_freq = update_freq_max;
	}
}

void Debug::draw(sf::RenderWindow& renderWindow)
{
	renderWindow.draw(debug_text_state);
	renderWindow.draw(debug_text_info);
}
