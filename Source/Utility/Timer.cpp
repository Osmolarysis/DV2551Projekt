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
	
	if (m_elapsedTime > m_maxDt)
		m_maxDt = m_elapsedTime;

	m_frameTimeSum += delta.count();
	m_frameCount += 1.0;

	m_time = now;
}

double Timer::getDt()
{
	return m_elapsedTime;
}

double Timer::getAverageFPS(int updateInterval)
{
	if (m_frameTimeSum > updateInterval) {
		double averageDt = m_frameTimeSum / m_frameCount;
		m_averageFPS = 1000.0 / averageDt;

		//std::cout << "Max DT: " << m_maxDt << ", Average DT: " << averageDt/1000.0 << "\n";

		m_maxDt = 0.0;
		m_frameTimeSum = 0.0;
		m_frameCount = 0.0;
	}

	return m_averageFPS;
}

void Timer::reset()
{
	m_time = std::chrono::steady_clock::now();
}
