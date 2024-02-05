#pragma once

#include <memory>

class IAudioMeterInfo
{
public:
	using Ptr = std::unique_ptr<IAudioMeterInfo>;

public:
	virtual ~IAudioMeterInfo() {};

	virtual void Initialize() = 0;
	virtual void Update(float dt) = 0;
	virtual void Clear() = 0;

	virtual float GetVolume() const noexcept = 0;
};