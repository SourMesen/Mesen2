#pragma once
#include "stdafx.h"

class TraceLogFileSaver
{
private:
	bool _logToFile;
	string _outputFilepath;
	string _outputBuffer;
	ofstream _outputFile;

public:
	void StartLogging(string filename)
	{
		_outputBuffer.clear();
		_outputFile.open(filename, ios::out | ios::binary);
		_logToFile = true;
	}

	void StopLogging()
	{
		if(_logToFile) {
			_logToFile = false;
			if(_outputFile) {
				if(!_outputBuffer.empty()) {
					_outputFile << _outputBuffer;
				}
				_outputFile.close();
			}
		}
	}

	/*void LogExtraInfo(const char* log, uint32_t cycleCount)
	{
		if(_logToFile) {
			//Flush current buffer
			_outputFile << _outputBuffer;
			_outputBuffer.clear();
			_outputFile << "[" << log << " - Cycle: " << std::to_string(cycleCount) << "]" << (_options.UseWindowsEol ? "\r\n" : "\n");
		}
	}*/
};