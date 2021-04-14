#pragma once
#include <chrono>

const unsigned int MAX_NR_OF_RECORDED_FRAMES = 100000;

class Timer {
private:
	Timer();
	static Timer m_this;
	double m_elapsedTime = 0.0;
	double m_frameTimeSum = 0.0;
	double m_frameCount = 0.0;
	double m_averageFPS = 0.0;
	double m_maxDt = 0.0;
	std::chrono::steady_clock::time_point m_time;

	double m_recordedFrameTimes[MAX_NR_OF_RECORDED_FRAMES];
	int m_nrOfRecordedFrames = 0;
	bool m_recording = false;

	void saveRecording();
public:
	static Timer* getInstance();
	void update();
	double getDt();
	double getAverageFPS(int updateInterval); 
	void reset();
};