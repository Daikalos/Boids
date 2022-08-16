#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include <string>
#include <unordered_map>

#include "Config.h"
#include "NonCopyable.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = NULL; } }

class AudioMeter : public NonCopyable
{
public:
	AudioMeter(Config& config, float refresh_freq);
	~AudioMeter();

	void initialize();
	void update(float dt);

	void clear();

public:
	float get_volume() const noexcept { return volume; }

private:
	void refresh(std::wstring* comp);

private:
	Config* config;

	float volume{0.0f};
	float refresh_freq_max{0.0f};
	float refresh_freq{0.0f};

	IAudioMeterInformation* pMeterInfo{NULL};
	IAudioSessionManager2* pSessionManager{NULL};
	IMMDevice* pDevice{NULL};

	std::unordered_map<std::wstring, std::pair<IAudioSessionControl*, IAudioMeterInformation*>> processes_session_control;
};