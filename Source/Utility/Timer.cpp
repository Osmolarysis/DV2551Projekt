#include "Timer.h"
#include "Input.h"
#include <iostream>
#include <fstream>
#include <string>

Timer Timer::m_this;

void Timer::saveRecording()
{
	time_t curr_time;
	tm* curr_tm = new tm;
	char time_string[100];

	time(&curr_time);
	localtime_s(curr_tm, &curr_time);
	strftime(time_string, 50, "%H%M%S", curr_tm);

	std::ofstream file;
	std::string fileName = "Recordings/frameTime_";
	fileName.append(time_string);
	fileName.append(".csv");

	file.open(fileName);

	if (file.is_open()) {
		file << "Frametime,\n";
		for (int i = 0; i < m_nrOfRecordedFrames; i++) {
			file << m_recordedFrameTimes[i] << ",\n";
		}

		std::cout << "Recording saved to: " << fileName << "\n";

		file.close();
	}

	delete curr_tm;
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

		if (m_nrOfRecordedFrames > MAX_NR_OF_RECORDED_FRAMES) {
			m_recording = false;
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
