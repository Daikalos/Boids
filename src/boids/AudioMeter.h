#pragma once

#include <memory>

#include "../utilities/NonCopyable.h"
#include "Config.h"

class AudioMeterInfoBase : public NonCopyable
{
public:
	using ptr = std::unique_ptr<AudioMeterInfoBase>;

public:
	virtual ~AudioMeterInfoBase()   {};

	virtual void initialize()		{};
	virtual void update(float dt)	{};
	virtual void clear()			{};

	[[nodiscard]] constexpr float get_volume() const noexcept
	{
		return _volume;
	}

protected:
	float _volume{0.0f};
};

#if defined(_WIN32)

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>

#include <string>
#include <unordered_map>

#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = NULL; } }

class AudioMeterWin final : public AudioMeterInfoBase
{
private:
	using ProcessInfo = typename std::pair<IAudioSessionControl*, IAudioMeterInformation*>;

public:
	AudioMeterWin(Config& config, float refresh_freq);
	~AudioMeterWin();

	void initialize() override;
	void update(float dt) override;
	void clear() override;

private:
	void refresh(std::wstring* comp);

private:
	Config*					_config				{nullptr};

	float					_refresh_freq_max	{0.0f};
	float					_refresh_freq		{0.0f};

	IAudioMeterInformation* _meter_info			{nullptr};
	IAudioSessionManager2*	_session_manager	{nullptr};
	IMMDevice*				_device				{nullptr};

	std::unordered_map<std::wstring, ProcessInfo> _processes_session_control;
};

#endif