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
	std::chrono::steady_clock::time_point m_time;

	int m_nrOfRecordedFrames = 0;
	bool m_recording = false;

	struct CPUProfiling {
		double frameTime;

		double updateTime;
		double beginFrame;
		double waitForPreviousFrame;
		double cpuRecording;
		double copyRecording;
		double computeRecording;
		double directRecording;

		double executeLists;
		double waitForCopyRecording;
		double waitForComputeRecording;
		double waitForDirectRecording;
	};
	CPUProfiling m_CPUprofiling[MAX_NR_OF_RECORDED_FRAMES];

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
	enum profilingVariable {
		FRAMETIME,
		UPDATETIME,
		BEGINFRAME,
		WAITFORPREVIOUSFRAME,
		CPURECORD,
		COPYRECORD,
		COMPUTERECORD,
		DIRECTRECORD,
		EXECUTELIST,
		WAITFORCOPYRECORD,
		WAITFORCOMPUTERECORD,
		WAITFORDIRECTRECORD
	};
	static Timer* getInstance();
	void update();
	double getDt();
	double getAverageFPS(int updateInterval);
	std::chrono::steady_clock::time_point timestamp();
	void reset();
	void logGPUtime(UINT64 copyTime, UINT64 computeTime, UINT64 directTime);
	void logCPUtime(profilingVariable, std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point);
};