#include "pch.h"
#include "SNES/Coprocessors/BSX/BsxStream.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

BsxStream::BsxStream()
{
}

void BsxStream::Reset(SnesConsole* console, int64_t resetDate)
{
	_console = console;
	_memoryManager = console->GetMemoryManager();

	_file.close();

	_channel = 0;
	_prefix = 0;
	_data = 0;
	_status = 0;

	_prefixLatch = false;
	_dataLatch = false;
	_firstPacket = false;
	_fileOffset = 0;
	_fileIndex = 0;

	_queueLength = 0;
	_prefixQueueLength = 0;
	_dataQueueLength = 0;

	_activeChannel = 0;
	_activeFileIndex = 0;

	_resetDate = resetDate;
	_resetMasterClock = 0;

	_tm = {};
}

uint16_t BsxStream::GetChannel()
{
	return _channel;
}

bool BsxStream::NeedUpdate()
{
	return _queueLength > 0;
}

bool BsxStream::FillQueues()
{
	if(_queueLength > 0) {
		_queueLength--;
		if(_prefixLatch && _prefixQueueLength < 0x80) {
			_prefixQueueLength++;
		}
		if(_dataLatch && _dataQueueLength < 0x80) {
			_dataQueueLength++;
		}
	}
	return NeedUpdate();
}

void BsxStream::OpenStreamFile()
{
	_file.close();
	string filename = "BSX" + HexUtilities::ToHex(_activeChannel) + "-" + std::to_string(_activeFileIndex) + ".bin";
	string folder = FolderUtilities::CombinePath(FolderUtilities::GetHomeFolder(), "Satellaview");
	_file.open(FolderUtilities::CombinePath(folder, filename), ios::binary);
}

bool BsxStream::LoadStreamFile()
{
	_activeChannel = _channel;
	_activeFileIndex = _fileIndex;

	OpenStreamFile();

	if(_file) {
		_firstPacket = true;
		_file.seekg(0, ios::end);
		_queueLength = (uint16_t)std::ceil(_file.tellg() / 22.0);
		_file.seekg(0, ios::beg);
		_fileIndex++;
		return true;
	} else {
		if(_fileIndex > 0) {
			//Go back to file #0 and try again
			_fileIndex = 0;
			if(LoadStreamFile()) {
				return true;
			}
		}

		//Couldn't load data for the specified channel
		_prefix |= 0x0F;
		return false;
	}
}

uint8_t BsxStream::GetPrefixCount()
{
	if(!_prefixLatch || !_dataLatch) {
		//Stream is disabled
		return 0;
	}

	if(_prefixQueueLength == 0 && _dataQueueLength == 0) {
		//Queue is empty, try to load in new data
		_fileOffset = 0;
		if(_channel == 0) {
			//Time channel
			_queueLength = 1;
			_firstPacket = true;
		} else {
			LoadStreamFile();
		}
	}

	return _prefixQueueLength;
}

uint8_t BsxStream::GetPrefix()
{
	if(!_prefixLatch) {
		return 0;
	}

	if(_prefixQueueLength > 0) {
		_prefix = 0;
		if(_firstPacket) {
			_prefix |= 0x10;
			_firstPacket = false;
		}

		_prefixQueueLength--;
		if(_queueLength == 0 && _prefixQueueLength == 0) {
			//Last packet
			_prefix |= 0x80;
		}
	}

	_status |= _prefix;

	return _prefix;
}

uint8_t BsxStream::GetData()
{
	if(!_dataLatch) {
		return 0;
	}

	if(_dataQueueLength > 0) {
		if(_channel == 0) {
			//Return Time
			_data = GetTime();
		} else if(_file) {
			//Read byte from stream file
			char byte;
			_file.get(byte);
			_data = (uint8_t)byte;
		}

		_fileOffset++;
		if(_fileOffset % 22 == 0) {
			//Finished reading current packet
			_dataQueueLength--;
		}
	}

	return _data;
}

uint8_t BsxStream::GetStatus(bool reset)
{
	uint8_t status = _status;
	if(reset) {
		_status = 0;
	}
	return status;
}

void BsxStream::SetChannelLow(uint8_t value)
{
	if((_channel & 0xFF) != 0xFF) {
		_fileIndex = 0;
	}
	_channel = (_channel & 0xFF00) | value;
}

void BsxStream::SetChannelHigh(uint8_t value)
{
	if((_channel >> 8) != (value & 0x3F)) {
		_fileIndex = 0;
	}
	_channel = (_channel & 0xFF) | ((value & 0x3F) << 8);
}

void BsxStream::SetPrefixLatch(uint8_t value)
{
	_prefixLatch = (value != 0);
	_prefixQueueLength = 0;
}

void BsxStream::SetDataLatch(uint8_t value)
{
	_dataLatch = (value != 0);
	_dataQueueLength = 0;
}

void BsxStream::InitTimeStruct()
{
	time_t dateTime = _resetDate + ((_memoryManager->GetMasterClock() - _resetMasterClock) / _console->GetMasterClockRate());

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	localtime_s(&_tm, &dateTime);
#else
	localtime_r(&dateTime, &_tm);
#endif

	_tm.tm_wday++;
	_tm.tm_mon++;
	_tm.tm_year += 1900;
}

uint8_t BsxStream::GetTime()
{
	if(_fileOffset == 0) {
		InitTimeStruct();
	}

	switch(_fileOffset) {
		case 0: return 0x00; //Data Group ID / Repetition
		case 1: return 0x00; //Data Group Link / Continuity
		case 2: return 0x00; //Data Group Size (24-bit)
		case 3: return 0x00;
		case 4: return 0x10;
		case 5: return 0x01; //Must be 0x01
		case 6: return 0x01; //Amount of packets (1)
		case 7: return 0x00; //Offset (24-bit)
		case 8: return 0x00;
		case 9: return 0x00;
		case 10: return _tm.tm_sec;
		case 11: return _tm.tm_min;
		case 12: return _tm.tm_hour;
		case 13: return _tm.tm_wday;
		case 14: return _tm.tm_mday;
		case 15: return _tm.tm_mon;
		case 16: return _tm.tm_year >> 0;
		case 17: return _tm.tm_year >> 8;
		default: return 0x00;
	}
}

void BsxStream::Serialize(Serializer& s)
{
	SV(_channel); SV(_prefix); SV(_data); SV(_status); SV(_prefixLatch); SV(_dataLatch); SV(_firstPacket); SV(_fileOffset); SV(_fileIndex);
	SV(_queueLength); SV(_prefixQueueLength); SV(_dataQueueLength); SV(_resetDate); SV(_resetMasterClock); SV(_activeChannel); SV(_activeFileIndex);

	if(!s.IsSaving()) {
		InitTimeStruct();
		OpenStreamFile();

		if(_file) {
			//Seek back to the previous location
			_file.seekg(_fileOffset, ios::beg);
		}
	}
}
