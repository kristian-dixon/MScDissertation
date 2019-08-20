#pragma once
#include <ostream>
#include <fstream>
#include <chrono>

class PerformanceCapture
{
public:
	void ToggleRecording();
	bool IsRecordingActive() const { return m_RecordEnabled; };
	void Update(float dt);

private:
	void BeginRecording();
	void StopRecording();


private:
	bool m_RecordEnabled;

	const float m_WriteFrequency = 1.0f;

	std::chrono::system_clock::time_point m_LastWrite;

	std::ofstream m_OutputStream;


};

