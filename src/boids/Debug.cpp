#include "Debug.h"

#include "../utilities/Utilities.h"

#include "Config.h"

Debug::Debug() : m_updateFreqMax(Config::Inst().Misc.DebugUpdateFreq) {}

bool Debug::GetRefresh() const noexcept			{ return m_refresh; }
const char* Debug::GetState() const noexcept	{ return m_enabled ? "DEBUG ENABLED" : "DEBUG DISABLED"; }

void Debug::SetUpdateFreq(float value)
{
	m_updateFreqMax = value;
}

void Debug::Load(const FontHolder& fontHolder)
{
	if (fontHolder.Contains(FontID::F8Bit))
	{
		const sf::Font& font = fontHolder.Get(FontID::F8Bit);

		m_textState.setFont(font);
		m_textInfo.setFont(font);
	}

	m_textState.setPosition(sf::Vector2f(32, 32));
	m_textState.setCharacterSize(26);

	m_textInfo.setPosition(sf::Vector2f(48, 48));
	m_textInfo.setCharacterSize(24);

	m_textState.setString(GetState() + std::string(" (PRESS THE TOGGLE KEY TO ENABLE)"));
	m_textInfo.setString("");
}

void Debug::Update(const InputHandler& inputHandler, std::uint32_t boidCount, std::uint32_t cellCount, float dt)
{
	m_refresh = false;

	if (!Config::Inst().Misc.DebugEnabled)
		return;

	if (inputHandler.GetKeyPressed(static_cast<sf::Keyboard::Key>(Config::Inst().Misc.DebugToggleKey)))
		Toggle();

	if (!m_enabled)
		return;

	m_fpsCounter.Update(dt);

	m_updateFreq -= dt;
	if (m_updateFreq <= 0.0f)
	{
		m_info =
			"\nCONFIG STATUS: " + std::string(Config::Inst().LoadStatus ? "SUCCESS" : "FAILED TO LOAD") +
			"\n\nBOIDS: " + std::to_string(boidCount) +
			"\nCELLS: " + std::to_string(cellCount) +
			"\nFPS: " + std::to_string((int)std::floorf(m_fpsCounter.GetFPS()));

		m_refresh = true;
		m_updateFreq = Config::Inst().Misc.DebugUpdateFreq;
	}

	m_textInfo.setString("\nCONFIG REFRESH: " + 
		util::remove_trailing_zeroes(std::to_string(util::set_precision(m_updateFreq, 2))) + m_info);
}

void Debug::Draw(sf::RenderWindow& window) const
{
	if (!Config::Inst().Misc.DebugEnabled)
		return;

	window.draw(m_textState);
	window.draw(m_textInfo);
}

void Debug::Toggle()
{
	m_enabled = !m_enabled;

	m_textState.setString(GetState());
	m_textInfo.setString(m_enabled ? m_textInfo.getString() : "");
}
