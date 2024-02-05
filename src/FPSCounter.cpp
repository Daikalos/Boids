#include "FPSCounter.h"

float FPSCounter::GetFPS() const
{
	return m_ready ? (1.0f / (m_average_fps / BUFFER_SIZE)) : 0.0f;
}

void FPSCounter::Update(float dt)
{
	m_average_fps -= m_buffer[m_current];
	m_buffer[m_current] = dt;
	m_average_fps += m_buffer[m_current];

	m_current = (m_current + 1) % BUFFER_SIZE;

	if (m_current == BUFFER_SIZE - 1)
		m_ready = true;
}
