#pragma once

#include <map>
#include <string>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include "Config.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = NULL; } }

class AudioMeter
{
public:
	AudioMeter() : refresh_freq_max(1.0f), refresh_freq(refresh_freq_max)
	{

	}
	~AudioMeter()
	{
		SAFE_RELEASE(pMeterInfo);
		SAFE_RELEASE(pSessionManager);

		for (int i = 0; i < Config::audio_responsive_apps.size(); ++i)
		{
			std::wstring process_name = Config::audio_responsive_apps[i];

			SAFE_RELEASE(processes_session_control[process_name].first);
			SAFE_RELEASE(processes_session_control[process_name].second);
		}

		CoUninitialize();
	}

	void initialize();
	void update(const float& deltaTime);

private:
	void refresh(std::wstring* comp);

private:
	float refresh_freq_max{ 0.0f };
	float refresh_freq{ 0.0f };

	IAudioMeterInformation* pMeterInfo{NULL};
	IAudioSessionManager2* pSessionManager{NULL};

	std::map<std::wstring, std::pair<IAudioSessionControl*, IAudioMeterInformation*>> processes_session_control;
};