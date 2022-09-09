#pragma once
#include "pch.h"

class TraceLogFileSaver
{
private:
	bool _enabled = false;
	string _outputFilepath;
	string _outputBuffer;
	ofstream _outputFile;

public:
	~TraceLogFileSaver()
	{
		StopLogging();
	}

	void StartLogging(string filename)
	{
		_outputBuffer.clear();
		_outputFile.open(filename, ios::out | ios::binary);
		_enabled = true;
	}

	void StopLogging()
	{
		if(_enabled) {
			_enabled = false;
			if(_outputFile) {
				if(!_outputBuffer.empty()) {
					_outputFile << _outputBuffer;
				}
				_outputFile.close();
			}
		}
	}

	__forceinline bool IsEnabled() { return _enabled; }

	void Log(string& log)
	{
		_outputBuffer += log + '\n';
		if(_outputBuffer.size() > 32768) {
			_outputFile << _outputBuffer;
			_outputBuffer.clear();
		}
	}
};