#pragma once
#include <chrono>

class Timer {
private:
	Timer();
	static Timer m_this;
	double m_elapsedTime = 0.0;
	double m_frameTimeSum = 0.0;
	double m_frameCount = 0.0;
	std::chrono::steady_clock::time_point m_time;
public:
	static Timer* getInstance();
	void update();
	double getDt();
	void reset();
};