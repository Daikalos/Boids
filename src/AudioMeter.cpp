#include "AudioMeter.h"

#include "CommonUtilities.hpp"

#include "Config.h"

#if defined(_WIN32)

AudioMeterWin::AudioMeterWin(float refresh_freq) 
	: m_refreshFreqMax(refresh_freq), m_refreshFreq(m_refreshFreqMax) {}

AudioMeterWin::~AudioMeterWin()
{
	SAFE_RELEASE(m_meterInfo);
	SAFE_RELEASE(m_sessionManager);

	for (const auto& process_name : Config::Inst().Audio.Apps)
	{
		SAFE_RELEASE(m_psc[process_name].first);
		SAFE_RELEASE(m_psc[process_name].second);
	}

	CoUninitialize();
}

float AudioMeterWin::GetVolume() const noexcept
{
	return m_lerpVolume;
}

void AudioMeterWin::Initialize()
{
	IMMDeviceEnumerator* enumerator = NULL;

	if (FAILED(CoInitialize(NULL)))
		return;

	// Get enumerator for audio endpoint devices.
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&enumerator)))
		return;

	// Get peak meter for default audio-rendering device.
	if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device)))
		return;

	SAFE_RELEASE(enumerator); // no longer needed

	if (FAILED(m_device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&m_sessionManager)))
	{
		SAFE_RELEASE(m_device);
		return;
	}

	if (Config::Inst().Audio.Apps.empty())
	{
		if (FAILED(m_device->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&m_meterInfo)))
			return;
	}
	else
	{
		RefreshAll();
	}

	SAFE_RELEASE(m_device);
}

void AudioMeterWin::Update(float dt)
{
	m_volume = 0.0f;

	if ((Config::Inst().Color.Flags & CF_Audio) == 0)
	{
		m_lerpVolume = 0.0f;
		return;
	}

	if (Config::Inst().Audio.Apps.empty() && m_meterInfo)
	{
		if (SUCCEEDED(m_meterInfo->GetPeakValue(&m_volume)))
			m_volume = -20.0f * std::log10f(1.0f - m_volume);
	}
	else
	{
		for (const std::wstring& processName : Config::Inst().Audio.Apps)
		{
			const auto it = m_psc.find(processName);
			if (it != m_psc.end())
			{
				IAudioSessionControl* sessionControl = it->second.first;
				IAudioMeterInformation* meterInformation = it->second.second;

				AudioSessionState state;
				if (sessionControl == NULL || meterInformation == NULL || FAILED(sessionControl->GetState(&state)) || state == AudioSessionStateExpired)
				{
					SAFE_RELEASE(sessionControl);
					SAFE_RELEASE(meterInformation);

					m_psc.erase(it);

					continue;
				}

				float temp = 0.0f;
				if (SUCCEEDED(meterInformation->GetPeakValue(&temp)) && temp > m_volume)
					m_volume = -20.0f * std::log10f(1.0f - temp);
			}
			else
			{
				m_refreshFreq -= dt;
				if (m_refreshFreq <= 0.0f)
				{
					Refresh(&processName);
					m_refreshFreq = m_refreshFreqMax;
				}
			}
		}
	}

	m_lerpVolume = util::Lerp(m_lerpVolume, m_volume, std::clamp(Config::Inst().Audio.Speed * dt, 0.0f, 1.0f));
}

void AudioMeterWin::Clear()
{
	for (auto& element : m_psc)
	{
		SAFE_RELEASE(element.second.first);
		SAFE_RELEASE(element.second.second);
	}

	m_psc.clear();
}

void AudioMeterWin::RefreshAll()
{
	IAudioSessionEnumerator* pSessionEnumerator = NULL;

	if (FAILED(m_sessionManager->GetSessionEnumerator(&pSessionEnumerator)))
		return;

	int sessionCount = 0;
	if (FAILED(pSessionEnumerator->GetCount(&sessionCount)))
		return;

	for (int i = 0; i < sessionCount; ++i)
	{
		IAudioSessionControl* sessionControl = NULL;
		IAudioSessionControl2* sessionControl2 = NULL;
		IAudioMeterInformation* meterInformation = NULL;

		const wchar_t* foundProcess = nullptr;

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
						for (const std::wstring& processName : Config::Inst().Audio.Apps)
						{
							if (processName.empty())
								continue;

							if (wcsstr(sessionID, processName.c_str()) != 0)
							{
								if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&meterInformation)))
								{
									foundProcess = processName.c_str();
									break;
								}
							}
						}
					}
				}
			}
		}

		if (foundProcess != nullptr && sessionControl && meterInformation)
		{
			m_psc[foundProcess] = std::make_pair(sessionControl, meterInformation);
		}
		else
		{
			SAFE_RELEASE(sessionControl);
			SAFE_RELEASE(meterInformation);
		}

		SAFE_RELEASE(sessionControl2);
	}

	SAFE_RELEASE(pSessionEnumerator);
}

void AudioMeterWin::Refresh(const std::wstring* processName)
{
	if (processName == nullptr || processName->empty())
		return;

	IAudioSessionEnumerator* pSessionEnumerator = NULL;

	if (FAILED(m_sessionManager->GetSessionEnumerator(&pSessionEnumerator)))
		return;

	int sessionCount = 0;
	if (FAILED(pSessionEnumerator->GetCount(&sessionCount)))
		return;

	for (int i = 0; i < sessionCount; ++i)
	{
		IAudioSessionControl* sessionControl		= NULL;
		IAudioSessionControl2* sessionControl2		= NULL;
		IAudioMeterInformation* meterInformation	= NULL;

		const wchar_t* foundProcess = nullptr;

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
						if (wcsstr(sessionID, processName->c_str()) != 0)
						{
							if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&meterInformation)))
								foundProcess = processName->c_str();
						}
					}
				}
			}
		}

		if (foundProcess != nullptr && sessionControl && meterInformation)
		{
			m_psc[foundProcess] = std::make_pair(sessionControl, meterInformation);
		}
		else
		{
			SAFE_RELEASE(sessionControl);
			SAFE_RELEASE(meterInformation);
		}

		SAFE_RELEASE(sessionControl2);
	}

	SAFE_RELEASE(pSessionEnumerator);
}

#endif