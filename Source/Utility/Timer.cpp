#include "Timer.h"

Timer Timer::m_this;

Timer* Timer::getInstance() 
{
	return &m_this;
}

Timer::Timer()
{
	m_timer = clock();
}

void Timer::update()
{
	clock_t now = clock();
	m_elapsedTime = double((now - m_timer) / 1000.0);
	if (m_elapsedTime > 1.0) 
	{
		m_elapsedTime = 0.0;
	}
	m_timer = now;
}

double Timer::getDt()
{
	return m_elapsedTime;
}

void Timer::reset()
{
	m_timer = clock();
}
