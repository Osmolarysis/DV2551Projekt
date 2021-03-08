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
	//m_timer = clock();
}

void Timer::update()
{
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	std::chrono::duration<double, std::milli> delta = now - m_time;
	m_elapsedTime = delta.count() / 1000.0;

	m_time = now;

	//printf("\ndt: %f", m_elapsedTime);

	/*
	clock_t now = clock();
	m_elapsedTime = double((now - m_timer) / 1000.0);
	if (m_elapsedTime > 1.0)
	{
		m_elapsedTime = 0.0;
	}
	m_timer = now;
	*/
}

double Timer::getDt()
{
	return m_elapsedTime;
}

void Timer::reset()
{
	m_time = std::chrono::steady_clock::now();
	//m_timer = clock();
}
