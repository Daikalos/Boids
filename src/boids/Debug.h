#pragma once

#include <SFML/Graphics.hpp>

#include "../window/ResourceHolder.hpp"
#include "../window/InputHandler.h"
#include "../utilities/FPSCounter.h"

#include "Config.h"

class Debug
{
public:
	Debug();

public:
	[[nodiscard]] bool GetRefresh() const noexcept;
	[[nodiscard]] const char* GetState() const noexcept;

	void SetUpdateFreq(float value);

public:
	void Load(const FontHolder& fontHolder);
	void Update(const InputHandler& inputHandler, std::uint32_t boidCount, std::uint32_t cellCount, float dt);
	void Draw(sf::RenderWindow& window) const;

private:
	void Toggle();

private:
	float			m_updateFreqMax		{0.0f};
	float			m_updateFreq		{0.0f};

	bool			m_enabled			{false};
	bool			m_refresh			{false};

	sf::Text		m_textState;
	sf::Text		m_textInfo;
	std::string		m_info;

	FPSCounter		m_fpsCounter;
};

