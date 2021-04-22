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
	const char seperator = '\t';

	//Open file
	std::ofstream file;
	std::string fileName = "Recordings/frameTimes_";
	fileName.append(timeString);
	fileName.append(".csv");
	file.open(fileName);

	//Write all the data
	if (file.is_open()) {
		file << "Index" <<
			seperator << "FrameTime" << 
			seperator << "UpdateTime" << 
			seperator << "BeginFrame()" << 
			seperator << "WaitForPreviousFrame" << 
			seperator << "CPURecord" << 
			seperator << "CopyRecord" << 
			seperator << "ComputeRecord" << 
			seperator << "DirectRecord" << 
			seperator << "ExecuteList" << 
			seperator << "WaitForCopyRecord" << 
			seperator << "WaitForComputeRecord" << 
			seperator << "WaitForDirectRecord" << 
			seperator << "Present" << 
			seperator << "GPUCopyQueue" << 
			seperator << "GPUComputeQueue" << 
			seperator << "GPUDirectQueue\n";
		for (int i = 1; i < m_nrOfRecordedFrames; i++) {
			file << i << 
				seperator << m_CPUprofiling[i].frameTime <<
				seperator << m_CPUprofiling[i].updateTime <<
				seperator << m_CPUprofiling[i].beginFrame <<
				seperator << m_CPUprofiling[i].waitForPreviousFrame <<
				seperator << m_CPUprofiling[i].cpuRecording <<
				seperator << m_CPUprofiling[i].copyRecording <<
				seperator << m_CPUprofiling[i].computeRecording <<
				seperator << m_CPUprofiling[i].directRecording <<
				seperator << m_CPUprofiling[i].executeLists <<
				seperator << m_CPUprofiling[i].waitForCopyRecording <<
				seperator << m_CPUprofiling[i].waitForComputeRecording <<
				seperator << m_CPUprofiling[i].waitForDirectRecording <<
				seperator << m_CPUprofiling[i].present <<
				seperator << m_recordedGPUQueuesTimes[i].copyTime << 
				seperator << m_recordedGPUQueuesTimes[i].computeTime << 
				seperator << m_recordedGPUQueuesTimes[i].directTime << 
				seperator << "\n";
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
		m_CPUprofiling[m_nrOfRecordedFrames++].frameTime = delta.count();

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
		m_frameTimeSum = 0.0;
		m_frameCount = 0.0;
	}

	return m_averageFPS;
}

std::chrono::steady_clock::time_point Timer::timestamp()
{
	return std::chrono::steady_clock::now();
}

void Timer::reset()
{
	m_time = std::chrono::steady_clock::now();
}

void Timer::logGPUtime(UINT64 _copyTime, UINT64 _computeTime, UINT64 _directTime)
{
	if (m_recording) {
		if (m_nrOfRecordedFrames < MAX_NR_OF_RECORDED_FRAMES) {
			m_recordedGPUQueuesTimes[m_nrOfRecordedFrames] = GPUQueueTimes(_copyTime, _computeTime, _directTime);
		}
	}
}

void Timer::logCPUtime(profilingVariable type, std::chrono::steady_clock::time_point before, std::chrono::steady_clock::time_point after)
{
	if (m_recording) {
		std::chrono::duration<double, std::milli> delta = after - before;
		double time = delta.count();
		switch (type)
		{
		case Timer::FRAMETIME:
			m_CPUprofiling[m_nrOfRecordedFrames].frameTime = time;
				break;
		case Timer::UPDATETIME:
			m_CPUprofiling[m_nrOfRecordedFrames].updateTime = time;
			break;
		case Timer::BEGINFRAME:
			m_CPUprofiling[m_nrOfRecordedFrames].beginFrame = time;
			break;
		case Timer::WAITFORPREVIOUSFRAME:
			m_CPUprofiling[m_nrOfRecordedFrames].waitForPreviousFrame = time;
			break;
		case Timer::CPURECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].cpuRecording = time;
			break;
		case Timer::COPYRECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].copyRecording = time;
			break;
		case Timer::COMPUTERECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].computeRecording = time;
			break;
		case Timer::DIRECTRECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].directRecording = time;
			break;
		case Timer::EXECUTELIST:
			m_CPUprofiling[m_nrOfRecordedFrames].executeLists = time;
			break;
		case Timer::WAITFORCOPYRECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].waitForCopyRecording = time;
			break;
		case Timer::WAITFORCOMPUTERECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].waitForComputeRecording = time;
			break;
		case Timer::WAITFORDIRECTRECORD:
			m_CPUprofiling[m_nrOfRecordedFrames].waitForDirectRecording = time;
			break;
		case Timer::PRESENT:
			m_CPUprofiling[m_nrOfRecordedFrames].present = time;
			break;
		default:
			break;
		}
	}
}
