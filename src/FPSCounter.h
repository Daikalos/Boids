#pragma once

class FPSCounter
{
private:
	static constexpr int BUFFER_SIZE = 128;

public:
	float GetFPS() const;

public:
	void Update(float dt);

private:
	float	m_buffer[BUFFER_SIZE]	{0.0f};
	float	m_average_fps			{0.0f};
	int		m_current				{0};
	bool	m_ready					{false};
};