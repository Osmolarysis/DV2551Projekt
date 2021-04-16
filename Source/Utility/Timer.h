#pragma once
#include <chrono>
#include <d3d12.h>

const unsigned int MAX_NR_OF_RECORDED_FRAMES = 10000;

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

	struct GPUQueueTimes {
		UINT64 copyTime;
		UINT64 computeTime;
		UINT64 directTime;
		GPUQueueTimes(UINT64 _copyTime, UINT64 _computeTime, UINT64 _directTime) {
			copyTime = _copyTime;
			computeTime = _computeTime;
			directTime = _directTime;
		}
		GPUQueueTimes() {
			copyTime = 0;
			computeTime = 0;
			directTime = 0;
		}
	};
	GPUQueueTimes m_recordedGPUQueuesTimes[MAX_NR_OF_RECORDED_FRAMES];
	unsigned int m_gpuLoggingCounter = 0;

	void saveRecording();
public:
	static Timer* getInstance();
	void update();
	double getDt();
	double getAverageFPS(int updateInterval); 
	void reset();
	void logGPUtime(UINT64 copyTime, UINT64 computeTime, UINT64 directTime);
};