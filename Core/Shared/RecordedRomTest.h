#pragma once

#include "pch.h"
#include <deque>
#include "Core/Shared/Interfaces/INotificationListener.h"
#include "Utilities/AutoResetEvent.h"

class VirtualFile;
class Emulator;

enum class RomTestState
{
	Failed,
	Passed,
	PassedWithWarnings
};

struct RomTestResult
{
	RomTestState State;
	int32_t ErrorCode;
};

class RecordedRomTest : public INotificationListener, public std::enable_shared_from_this<RecordedRomTest>
{
private:
	Emulator* _emu;

	bool _inBackground = false;
	bool _recording = false;
	bool _runningTest = false;
	int _badFrameCount = 0;
	bool _isLastFrameGood = false;

	uint8_t _previousHash[16] = {};
	std::deque<uint8_t*> _screenshotHashes;
	std::deque<uint8_t> _repetitionCount;
	uint8_t _currentCount = 0;
	
	string _filename;
	ofstream _file;

	AutoResetEvent _signal;

private:
	void Reset();
	void ValidateFrame();
	void SaveFrame();
	void Save();

public:
	RecordedRomTest(Emulator* console, bool inBackground);
	virtual ~RecordedRomTest();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
	void Record(string filename, bool reset);
	RomTestResult Run(string filename);
	void Stop();
};