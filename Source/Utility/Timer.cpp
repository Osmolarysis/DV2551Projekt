#include "Timer.h"
#include "Input.h"
#include <iostream>
#include <fstream>
#include <string>

Timer Timer::m_this;

void Timer::saveRecording()
{
	//Get a time string to name file
	time_t currentTime;
	tm* timeStruct = new tm;
	char timeString[100];
	time(&currentTime);
	localtime_s(timeStruct, &currentTime);
	strftime(timeString, 50, "%H%M%S", timeStruct);
	const char seperator = ';';

	//Open file
	std::ofstream file;
	std::string fileName = "Recordings/frameTimes_";
	fileName.append(timeString);
	fileName.append(".csv");
	file.open(fileName);

	//Write all the data
	if (file.is_open()) {
		file << "Index" << seperator << "FrameTime" << seperator << "GPUCopyQueue" << seperator << "GPUComputeQueue" << seperator << "GPUDirectQueue\n";
		for (int i = 0; i < m_nrOfRecordedFrames; i++) {
			file << i + 1 << seperator << m_recordedFrameTimes[i] << seperator << m_recordedGPUQueuesTimes[i].copyTime << seperator << m_recordedGPUQueuesTimes[i].computeTime << seperator << m_recordedGPUQueuesTimes[i].directTime << seperator << "\n";
		}

		std::cout << "Recording saved to: " << fileName << "\n";
		file.close();
	}

	delete timeStruct;
}

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
	Input* input = Input::getInstance();
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	std::chrono::duration<double, std::milli> delta = now - m_time;
	m_elapsedTime = delta.count() / 1000.0;

	if (m_elapsedTime > m_maxDt)
		m_maxDt = m_elapsedTime;

	m_frameTimeSum += delta.count();
	m_frameCount += 1.0;

	m_time = now;

	if (input->keyPressed(DirectX::Keyboard::Keys::R)) {
		m_recording = !m_recording;
		if (m_recording) {
			printf("Recording started\n");
		}
		else {
			printf("Recording stopped\n");
			saveRecording();
			m_nrOfRecordedFrames = 0;
		}
	}

	if (m_recording) {
		m_recordedFrameTimes[m_nrOfRecordedFrames++] = m_elapsedTime;

		if (m_nrOfRecordedFrames >= MAX_NR_OF_RECORDED_FRAMES) {
			m_recording = false;
			printf("Recording stopped (max frames reached)\n");
			saveRecording();
			m_nrOfRecordedFrames = 0;
		}
	}
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

void Timer::logGPUtime(UINT64 _copyTime, UINT64 _computeTime, UINT64 _directTime)
{
	if (m_recording) {
		//std::cout << _copyTime << "\t" << _computeTime << "\t" << _directTime << std::endl;
		if (m_gpuLoggingCounter <= MAX_NR_OF_RECORDED_FRAMES) {
			m_recordedGPUQueuesTimes[m_gpuLoggingCounter] = GPUQueueTimes(_copyTime, _computeTime, _directTime);
			m_gpuLoggingCounter++;
		}
	}
}
