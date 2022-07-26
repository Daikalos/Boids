#include "AudioMeter.h"

void AudioMeter::initialize()
{
	if (Config::color_option != 3)
		return;

	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;

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

	if (Config::audio_responsive_apps.size() > 0)
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

void AudioMeter::update(const float& deltaTime)
{
	if (Config::color_option != 3)
		return;

	Config::volume = 0.0f;

	if (pMeterInfo)
	{
		if (SUCCEEDED(pMeterInfo->GetPeakValue(&Config::volume)))
			Config::volume = -20 * std::log10f(1.0f - Config::volume);

		return;
	}

	for (int i = 0; i < Config::audio_responsive_apps.size(); ++i)
	{
		std::wstring process_name = Config::audio_responsive_apps[i];

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
			if (SUCCEEDED(meterInformation->GetPeakValue(&temp)) && temp > Config::volume)
				Config::volume = -20 * std::log10f(1.0f - temp);
		}
		else
		{
			refresh_freq -= deltaTime;

			if (refresh_freq <= 0.0f)
			{
				refresh(&process_name);
				refresh_freq = refresh_freq_max;
			}
		}
	}
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
						for (int j = 0; j < Config::audio_responsive_apps.size(); ++j)
						{
							process_name = Config::audio_responsive_apps[j];

							if (process_name.size() == 0)
								continue;

							if (comp != nullptr && *comp != process_name)
								continue;

							if (wcsstr(sessionID, process_name.c_str()) != 0)
							{
								if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&meterInformation)))
								{
									break;
								}
							}
						}
					}
				}
			}
		}

		if (sessionControl && meterInformation)
			processes_session_control[process_name] = std::make_pair(sessionControl, meterInformation);

		SAFE_RELEASE(sessionControl);
	}

	SAFE_RELEASE(pSessionEnumerator);
}
