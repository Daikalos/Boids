#include "AudioMeter.h"

AudioMeter::AudioMeter(Config& config, float refresh_freq)
	: config(&config), refresh_freq_max(refresh_freq), refresh_freq(refresh_freq_max) 
{

}

AudioMeter::~AudioMeter()
{
	SAFE_RELEASE(pMeterInfo);
	SAFE_RELEASE(pSessionManager);

	for (int i = 0; i < config->audio_responsive_apps.size(); ++i)
	{
		std::wstring process_name = config->audio_responsive_apps[i];

		SAFE_RELEASE(processes_session_control[process_name].first);
		SAFE_RELEASE(processes_session_control[process_name].second);
	}

	CoUninitialize();
}

void AudioMeter::initialize()
{
	IMMDeviceEnumerator* pEnumerator = NULL;

	if (FAILED(CoInitialize(NULL)))
		return;

	// Get enumerator for audio endpoint devices.
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator)))
		return;

	// Get peak meter for default audio-rendering device.
	if (FAILED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice)))
		return;

	SAFE_RELEASE(pEnumerator);

	if (FAILED(pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pSessionManager)))
		return;

	if (config->audio_responsive_apps.size() > 0)
	{
		refresh(nullptr);
	}
	else
	{
		if (FAILED(pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo)))
			return;
	}

	SAFE_RELEASE(pDevice);
}

void AudioMeter::update(const float& dt)
{
	volume = 0.0f;

	if (!(config->color_flags & ColorFlags::Audio))
		return;

	if (config->audio_responsive_apps.size() == 0)
	{
		if (pMeterInfo)
		{
			if (SUCCEEDED(pMeterInfo->GetPeakValue(&volume)))
				volume = -20 * std::log10f(1.0f - volume);
		}
		else
		{
			if (FAILED(pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo)))
				return;
		}
	}

	for (std::wstring process_name : config->audio_responsive_apps)
	{
		if (processes_session_control.contains(process_name))
		{
			IAudioSessionControl* sessionControl = processes_session_control[process_name].first;
			IAudioMeterInformation* meterInformation = processes_session_control[process_name].second;

			AudioSessionState state;
			if (sessionControl == NULL || meterInformation == NULL || FAILED(sessionControl->GetState(&state)) || state == AudioSessionStateExpired)
			{
				SAFE_RELEASE(processes_session_control[process_name].first);
				SAFE_RELEASE(processes_session_control[process_name].second);
				processes_session_control.erase(process_name);
				continue;
			}

			float temp = 0.0f;
			if (SUCCEEDED(meterInformation->GetPeakValue(&temp)) && temp > volume)
				volume = -20 * std::log10f(1.0f - temp);
		}
		else
		{
			refresh_freq -= dt;

			if (refresh_freq <= 0.0f)
			{
				refresh(&process_name);
				refresh_freq = refresh_freq_max;
			}
		}
	}
}

void AudioMeter::clear()
{
	for (std::pair<const std::wstring, std::pair<IAudioSessionControl*, IAudioMeterInformation*>>& element : processes_session_control)
	{
		SAFE_RELEASE(element.second.first);
		SAFE_RELEASE(element.second.second);
	}
	processes_session_control.clear();
}

void AudioMeter::refresh(std::wstring* comp)
{
	IAudioSessionEnumerator* pSessionEnumerator = NULL;

	if (FAILED(pSessionManager->GetSessionEnumerator(&pSessionEnumerator)))
		return;

	int sessionCount = 0;
	if (FAILED(pSessionEnumerator->GetCount(&sessionCount)))
		return;

	for (int i = 0; i < sessionCount; ++i)
	{
		IAudioSessionControl* sessionControl = NULL;
		IAudioSessionControl2* sessionControl2 = NULL;
		IAudioMeterInformation* meterInformation = NULL;

		std::wstring process_name;

		if (SUCCEEDED(pSessionEnumerator->GetSession(i, &sessionControl)))
		{
			AudioSessionState state;
			if (SUCCEEDED(sessionControl->GetState(&state)) && state != AudioSessionStateExpired)
			{
				if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2)))
				{
					LPWSTR sessionID;
					if (SUCCEEDED(sessionControl2->GetSessionInstanceIdentifier(&sessionID)))
					{
						for (std::wstring pn : config->audio_responsive_apps)
						{
							if (pn.size() == 0)
								continue;

							if (comp != nullptr && *comp != pn)
								continue;

							if (wcsstr(sessionID, pn.c_str()) != 0)
							{
								if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&meterInformation)))
								{
									process_name = pn;
									break;
								}
							}
						}
					}
				}
			}
		}

		if (process_name.size())
			processes_session_control[process_name] = std::make_pair(sessionControl, meterInformation);

		SAFE_RELEASE(sessionControl);
	}

	SAFE_RELEASE(pSessionEnumerator);
}
