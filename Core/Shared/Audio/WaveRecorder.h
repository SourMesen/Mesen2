#include "pch.h"

class WaveRecorder
{
private:
	std::ofstream _stream;
	uint32_t _streamSize;
	uint32_t _sampleRate;
	bool _isStereo;
	string _outputFile;

	void WriteHeader();
	void UpdateSizeValues();
	void CloseFile();

public:
	WaveRecorder(string outputFile, uint32_t sampleRate, bool isStereo);
	~WaveRecorder();

	bool WriteSamples(int16_t* samples, uint32_t sampleCount, uint32_t sampleRate, bool isStereo);
};