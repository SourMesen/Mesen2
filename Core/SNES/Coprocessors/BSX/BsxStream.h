#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class SnesConsole;
class SnesMemoryManager;

class BsxStream : public ISerializable
{
private:
	SnesConsole* _console;
	SnesMemoryManager* _memoryManager;

	ifstream _file;
	tm _tm = {};

	uint16_t _channel = 0;
	uint8_t _prefix = 0;
	uint8_t _data = 0;
	uint8_t _status = 0;

	bool _prefixLatch = false;
	bool _dataLatch = false;
	bool _firstPacket = false;
	uint32_t _fileOffset = 0;
	uint8_t _fileIndex = 0;

	uint16_t _queueLength = 0;
	uint8_t _prefixQueueLength = 0;
	uint8_t _dataQueueLength = 0;

	uint16_t _activeChannel = 0;
	uint8_t _activeFileIndex = 0;

	int64_t _resetDate = -1;
	uint64_t _resetMasterClock = 0;

	void OpenStreamFile();
	bool LoadStreamFile();

	void InitTimeStruct();
	uint8_t GetTime();

public:
	BsxStream();
	void Reset(SnesConsole* console, int64_t customDate);

	uint16_t GetChannel();
	bool NeedUpdate();
	bool FillQueues();

	uint8_t GetPrefixCount();
	uint8_t GetPrefix();
	uint8_t GetData();
	uint8_t GetStatus(bool reset);

	void SetChannelLow(uint8_t value);
	void SetChannelHigh(uint8_t value);
	void SetPrefixLatch(uint8_t value);
	void SetDataLatch(uint8_t value);

	void Serialize(Serializer& s) override;
};
