#include "Timer.h"
#include <iostream>

Timer Timer::m_this;

Timer* Timer::getInstance() 
{
	return &m_this;
}

Timer::Timer()
{
	m_time = std::chrono::steady_clock::now();
}

void Timer::update()
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	std::chrono::duration<double, std::milli> delta = now - m_time;
	m_elapsedTime = delta.count() / 1000.0;
	
	m_frameTimeSum += delta.count();
	m_frameCount += 1.0;

	if (m_frameTimeSum > 1000.0) {
		double averageDt = m_frameTimeSum / m_frameCount;
		double averageFps = 1000.0 / averageDt;
		printf("Average FPS: %f\n", averageFps);

		m_frameTimeSum = 0.0;
		m_frameCount = 0.0;
	}

	m_time = now;
}

double Timer::getDt()
{
	return m_elapsedTime;
}

void Timer::reset()
{
	m_time = std::chrono::steady_clock::now();
}
