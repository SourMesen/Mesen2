#pragma once

#include "stdafx.h"
#include <deque>
#include "INotificationListener.h"
#include "../Utilities/AutoResetEvent.h"

class VirtualFile;
class Console;
class Ppu;

class RecordedRomTest : public INotificationListener, public std::enable_shared_from_this<RecordedRomTest>
{
private:
	shared_ptr<Console> _console;
	Ppu* _ppu;

	bool _recording = false;
	bool _runningTest = false;
	int _badFrameCount = 0;

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
	RecordedRomTest(shared_ptr<Console> console = nullptr);
	virtual ~RecordedRomTest();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
	void Record(string filename, bool reset);
	int32_t Run(string filename);
	void Stop();
};