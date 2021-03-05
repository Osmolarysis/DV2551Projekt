#pragma once
#include <time.h>
class Timer {
private:
	Timer();
	static Timer m_this;
	clock_t m_timer = clock();
	double m_elapsedTime = 0.0;

public:
	static Timer* getInstance();
	void update();
	double getDt();
	void reset();
};