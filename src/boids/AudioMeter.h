#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include <string>
#include <unordered_map>

#include "../utilities/NonCopyable.h"

#include "Config.h"

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
	float get_volume() const noexcept { return _volume; }

private:
	void refresh(std::wstring* comp);

private:
	Config* _config{nullptr};

	float _volume{0.0f};
	float _refresh_freq_max{0.0f};
	float _refresh_freq{0.0f};

	IAudioMeterInformation* _meter_info{NULL};
	IAudioSessionManager2* _session_manager{NULL};
	IMMDevice* _device{NULL};

	std::unordered_map<std::wstring, std::pair<IAudioSessionControl*, IAudioMeterInformation*>> _processes_session_control;
};