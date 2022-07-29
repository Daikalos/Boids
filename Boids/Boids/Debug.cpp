#include "Debug.h"

Debug::Debug(Config* config)
	: config(config), update_freq_max(config->debug_update_freq), update_freq(0.0f)
{

}

Debug::~Debug()
{

}

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
	debug_text_info.setString("\nFPS: 0\nBOIDS: " + std::to_string(config->boid_count));
}

bool Debug::update(const InputHandler& inputHandler, const float& deltaTime)
{
	if (!config->debug_enabled)
		return false;

	if (inputHandler.get_key_pressed(static_cast<sf::Keyboard::Key>(config->debug_toggle_key)))
		toggle();

	update_freq -= deltaTime;

	if (update_freq <= 0.0f)
	{
		debug_text_info.setString(
			"\nFPS: " + std::to_string((int)(1.0f / deltaTime)) +
			"\nBOIDS: " + std::to_string(config->boid_count));

		update_freq = update_freq_max;

		if (enabled)
			return true;
	}

	return false;
}

void Debug::draw(sf::RenderWindow& renderWindow)
{
	if (!config->debug_enabled)
		return;

	renderWindow.draw(debug_text_state);
	renderWindow.draw(debug_text_info);
}
