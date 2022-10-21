#pragma once

#include <memory>

#include "../utilities/NonCopyable.h"
#include "Config.h"

class IAudioMeterInfo : public NonCopyable
{
public:
	using ptr = std::unique_ptr<IAudioMeterInfo>;

public:
	virtual ~IAudioMeterInfo() {};

	virtual void initialize() = 0;
	virtual void update(float dt) = 0;
	virtual void clear() = 0;

	virtual float get_volume() const noexcept = 0;
};

class AudioMeterEmpty final : public IAudioMeterInfo
{
public:
	void initialize() override {}
	void update(float dt) override {}
	void clear() override {}

	float get_volume() const noexcept { return 0.0f; }
};

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
	using ProcessInfo = typename std::pair<IAudioSessionControl*, IAudioMeterInformation*>;

public:
	AudioMeterWin(Config& config, float refresh_freq);
	~AudioMeterWin();

public:
	void initialize() override;
	void update(float dt) override;
	void clear() override;

	[[nodiscard]] float get_volume() const noexcept override;

private:
	void refresh(const std::wstring* comp);

private:
	Config*					_config				{nullptr};

	float					_volume				{0.0f};

	float					_refresh_freq_max	{0.0f};
	float					_refresh_freq		{0.0f};

	IAudioMeterInformation* _meter_info			{nullptr};
	IAudioSessionManager2*	_session_manager	{nullptr};
	IMMDevice*				_device				{nullptr};

	std::unordered_map<std::wstring, ProcessInfo> _processes_session_control;
};

#endif