#pragma once

#include <memory>

#include "IAudioMeterInfo.h"

#if defined(_WIN32)

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include <string>
#include <unordered_map>

#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = NULL; } }

class AudioMeterWin final : public IAudioMeterInfo
{
private:
	using ProcessInfo = std::pair<IAudioSessionControl*, IAudioMeterInformation*>;

public:
	AudioMeterWin(float refresh_freq);
	~AudioMeterWin();

public:
	void Initialize() override;
	void Update(float dt) override;
	void Clear() override;

	[[nodiscard]] float GetVolume() const noexcept override;

private:
	void RefreshAll();
	void Refresh(const std::wstring* processName);

private:
	float					m_volume			{0.0f};
	float					m_lerpVolume		{0.0f};

	float					m_refreshFreqMax	{0.0f};
	float					m_refreshFreq		{0.0f};

	IAudioMeterInformation* m_meterInfo			{nullptr};
	IAudioSessionManager2*	m_sessionManager	{nullptr};
	IMMDevice*				m_device			{nullptr};

	std::unordered_map<std::wstring, ProcessInfo> m_psc; // processes session control
};

#endif