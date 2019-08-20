#include "PerformanceCapture.h"
#include <ctime>
#include <string>
#include <fstream>
#include <chrono>

using namespace std;
void PerformanceCapture::ToggleRecording()
{
	m_RecordEnabled = !m_RecordEnabled;

	if(m_RecordEnabled)
	{
		BeginRecording();
	}
	else
	{
		StopRecording();
	}
}


void PerformanceCapture::Update(float dt)
{
	if(m_RecordEnabled)
	{
		auto now = std::chrono::system_clock::now();
		std::chrono::duration<float> diff = now - m_LastWrite;

		if(diff.count() > m_WriteFrequency)
		{
			m_OutputStream << 1.f / dt << endl;

			m_LastWrite = now;
		}
	}
}

void PerformanceCapture::BeginRecording()
{
	//Create output file - filename will be "PerformanceCapture_LOCALTIME.csv
	//https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm

	time_t now = time(0);
	char* date = ctime(&now);

	
	string filename = "PerformanceCapture_";
	if(date != nullptr)
	{
		//filename += string(date);
	}

	m_OutputStream = ofstream(filename + ".csv");

	//Add file formatting
	if(m_OutputStream.is_open())
	{
		m_OutputStream << "Frames Per Second" << endl;
	}

	m_LastWrite = std::chrono::system_clock::now();
}

void PerformanceCapture::StopRecording()
{
	m_OutputStream.close();
}
