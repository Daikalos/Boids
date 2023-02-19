#include "AudioMeter.h"

#if defined(_WIN32)

AudioMeterWin::AudioMeterWin(float refresh_freq) 
	: _refresh_freq_max(refresh_freq), _refresh_freq(_refresh_freq_max) {}

AudioMeterWin::~AudioMeterWin()
{
	SAFE_RELEASE(_meter_info);
	SAFE_RELEASE(_session_manager);

	for (int i = 0; i < Config::GetInstance().audio_responsive_apps.size(); ++i)
	{
		std::wstring& process_name = Config::GetInstance().audio_responsive_apps[i];

		SAFE_RELEASE(_processes_session_control[process_name].first);
		SAFE_RELEASE(_processes_session_control[process_name].second);
	}

	CoUninitialize();
}

float AudioMeterWin::get_volume() const noexcept
{
	return _volume;
}

void AudioMeterWin::initialize()
{
	IMMDeviceEnumerator* enumerator = NULL;

	if (FAILED(CoInitialize(NULL)))
		return;

	// Get enumerator for audio endpoint devices.
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&enumerator)))
		return;

	// Get peak meter for default audio-rendering device.
	if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_device)))
		return;

	SAFE_RELEASE(enumerator);

	if (FAILED(_device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&_session_manager)))
		return;

	if (Config::GetInstance().audio_responsive_apps.size() > 0)
	{
		refresh(nullptr);
	}
	else
	{
		if (FAILED(_device->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&_meter_info)))
			return;
	}

	SAFE_RELEASE(_device);
}

void AudioMeterWin::update(float dt)
{
	_volume = 0.0f;

	if (!(Config::GetInstance().color_flags & CF_Audio))
		return;

	if (Config::GetInstance().audio_responsive_apps.size() == 0)
	{
		if (_meter_info)
		{
			if (SUCCEEDED(_meter_info->GetPeakValue(&_volume)))
				_volume = -20 * std::log10f(1.0f - _volume);
		}
		else
		{
			if (FAILED(_device->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&_meter_info)))
				return;
		}
	}

	for (const std::wstring& process_name : Config::GetInstance().audio_responsive_apps)
	{
		if (_processes_session_control.contains(process_name))
		{
			IAudioSessionControl* sessionControl = _processes_session_control[process_name].first;
			IAudioMeterInformation* meterInformation = _processes_session_control[process_name].second;

			AudioSessionState state;
			if (sessionControl == NULL || meterInformation == NULL || FAILED(sessionControl->GetState(&state)) || state == AudioSessionStateExpired)
			{
				SAFE_RELEASE(_processes_session_control[process_name].first);
				SAFE_RELEASE(_processes_session_control[process_name].second);
				_processes_session_control.erase(process_name);
				continue;
			}

			float temp = 0.0f;
			if (SUCCEEDED(meterInformation->GetPeakValue(&temp)) && temp > _volume)
				_volume = -20 * std::log10f(1.0f - temp);
		}
		else
		{
			_refresh_freq -= dt;

			if (_refresh_freq <= 0.0f)
			{
				refresh(&process_name);
				_refresh_freq = _refresh_freq_max;
			}
		}
	}
}

void AudioMeterWin::clear()
{
	for (auto& element : _processes_session_control)
	{
		SAFE_RELEASE(element.second.first);
		SAFE_RELEASE(element.second.second);
	}
	_processes_session_control.clear();
}

void AudioMeterWin::refresh(const std::wstring* comp)
{
	IAudioSessionEnumerator* pSessionEnumerator = NULL;

	if (FAILED(_session_manager->GetSessionEnumerator(&pSessionEnumerator)))
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
						for (const std::wstring& pn : Config::GetInstance().audio_responsive_apps)
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
			_processes_session_control[process_name] = std::make_pair(sessionControl, meterInformation);

		SAFE_RELEASE(sessionControl);
	}

	SAFE_RELEASE(pSessionEnumerator);
}

#endif