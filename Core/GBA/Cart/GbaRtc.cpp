#include "pch.h"
#include "GBA/Cart/GbaRtc.h"
#include "Utilities/Serializer.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"

GbaRtc::GbaRtc(Emulator* emu)
{
	_emu = emu;

	_state.Month = 1;
	_state.Day = 1;
	_state.Status = 0x02;
	_state.IntHour = 0x80;

	_lastUpdateTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint8_t GbaRtc::SanitizeData(uint8_t value, uint8_t maxValue, uint8_t fixedValue)
{
	if(value > maxValue || (value & 0x0F) >= 0x0A || (value & 0xF0) >= 0xA0) {
		//Invalid values
		return fixedValue;
	}

	return value;
}

void GbaRtc::FromDateTime(uint64_t data, bool includeYmd)
{
	UpdateTime();

	//TODOGBA process invalid days (e.g feb 30)
	if(includeYmd) {
		_state.Year = SanitizeData(data & 0xFF, 0x99, 0);
		_state.Month = SanitizeData((data >> 8) & 0x1F, 0x12, 1);
		_state.Day = SanitizeData((data >> 16) & 0x3F, 0x31, 1);
		_state.DoW = SanitizeData((data >> 24) & 0x07, 0x06, 0);
	}
	_state.Minute = SanitizeData((data >> 40) & 0x7F, 0x59, 0);
	_state.Second = SanitizeData((data >> 48) & 0x7F, 0x59, 0);

	bool is24hMode = _state.Status & 0x40;
	uint8_t hourData = (data >> 32) & 0xBF;
	if(is24hMode) {
		_state.Hour = SanitizeData(hourData & 0x3F, 0x23, 0);
		if(_state.Hour >= 0x12) {
			_state.Hour |= 0x80; //Set PM flag
		}
	} else {
		_state.Hour = SanitizeData(hourData & 0x3F, 0x11, 0) | (hourData & 0x80);
	}
}

uint64_t GbaRtc::ToDateTime()
{
	UpdateTime();
	return _state.Year | (_state.Month << 8) | (_state.Day << 16) | (_state.DoW << 24) | ((uint64_t)_state.Hour << 32) | ((uint64_t)_state.Minute << 40) | ((uint64_t)_state.Second << 48);
}

uint8_t GbaRtc::GetCommandLength(Command cmd)
{
	switch(cmd) {
		default:
		case Command::Reset: return 0;
		case Command::Status: return 8;
		case Command::DateTime: return 8 * 7;
		case Command::Time: return 8 * 3;
		case Command::Alarm1: return 8 * 2;
		case Command::Alarm2: return 8;
		case Command::TestStart: return 0;
		case Command::TestEnd: return 0;
	}
}

void GbaRtc::ProcessDataIn(uint8_t value)
{
	uint64_t bit = (value & 0x02) >> 1;
	Command cmd = (Command)((_command & 0x0F) >> 1);

	_dataIn |= (bit << (GetCommandLength(cmd) - _dataInSize));
	_dataInSize--;

	if(_dataInSize == 0) {
		switch(cmd) {
			case Command::Status:
				_state.Status &= 0x80;
				_state.Status |= (_dataIn & 0x6A);
				break;

			case Command::DateTime: FromDateTime(_dataIn, true); break;
			case Command::Time: FromDateTime(_dataIn << 32, false); break;

			default:
				//TODOGBA not implemented
				break;
		}
		_command = 0;
	}
}

void GbaRtc::ProcessDataOut()
{
	_bitOut = _dataOut & 0x01;
	_dataOut >>= 1;
	_dataOutSize--;
	if(_dataOutSize == 0) {
		_command = 0;
	}
}

void GbaRtc::ProcessCommand()
{
	Command cmd = (Command)((_command & 0x0F) >> 1);
	uint8_t length = GetCommandLength(cmd);
	bool read = _command & 0x01;
	if(read) {
		_dataOutSize = length;

		if(_emu->IsDebugging()) {
			_emu->DebugLog("[RTC] Read command: " + string(magic_enum::enum_name<Command>(cmd)));
		}

		switch(cmd) {
			case Command::Reset: Reset(); break;
			case Command::Status: _dataOut = _state.Status; break;
			case Command::DateTime: _dataOut = ToDateTime(); break;
			case Command::Time: _dataOut = ToDateTime() >> 32; break;
			default:
				//TODOGBA not implemented
				_dataOut = 0;
				break;
		}
	} else {
		if(_emu->IsDebugging()) {
			_emu->DebugLog("[RTC] Write command: " + string(magic_enum::enum_name<Command>(cmd)));
		}

		if(cmd == Command::Reset) {
			Reset();
		}

		_dataInSize = length;
		_dataIn = 0;
	}
}

void GbaRtc::Reset()
{
	_state = {};
	_state.Month = 1;
	_state.Day = 1;
}

void GbaRtc::UpdateTime()
{
	uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	uint32_t elapsedSeconds = (uint32_t)(currentTime - _lastUpdateTime);
	if(elapsedSeconds <= 0) {
		return;
	}

	std::tm tm = {};
	tm.tm_sec = (_state.Second & 0x0F) + ((_state.Second >> 4) * 10);
	tm.tm_min = (_state.Minute & 0x0F) + ((_state.Minute >> 4) * 10);
	int hour = _state.Hour & 0x3F;
	tm.tm_hour = (hour & 0x0F) + ((hour >> 4) * 10);
	if(tm.tm_hour < 12 && (_state.Hour & 0x80)) {
		tm.tm_hour += 12;
	}
	tm.tm_mday = (_state.Day & 0x0F) + ((_state.Day >> 4) * 10);

	int month = _state.Month - 1;
	tm.tm_mon = (month & 0x0F) + ((month >> 4) * 10);
	tm.tm_year = 100 + (_state.Year & 0x0F) + ((_state.Year >> 4) * 10);

	std::time_t tt = mktime(&tm);
	if(tt == -1) {
		//Invalid time
		_lastUpdateTime = currentTime;
		return;
	}

	int8_t dowGap = 0;
	if(tm.tm_wday != _state.DoW) {
		//The DoW on the RTC can be set to any arbitrary value for a specific date
		//Check the gap between the value set by the game & the real dow for that date
		dowGap = (int8_t)tm.tm_wday - (int8_t)_state.DoW;
	}

	std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(tt);
	timePoint += std::chrono::seconds((uint32_t)elapsedSeconds);

	std::time_t newTime = system_clock::to_time_t(timePoint);
	std::tm newTm;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	localtime_s(&newTm, &newTime);
#else
	localtime_r(&newTime, &newTm);
#endif

	_state.Second = (newTm.tm_sec % 10) + ((newTm.tm_sec / 10) << 4);
	_state.Minute = (newTm.tm_min % 10) + ((newTm.tm_min / 10) << 4);

	bool is24hMode = _state.Status & 0x40;
	if(is24hMode) {
		_state.Hour = (newTm.tm_hour % 10) + ((newTm.tm_hour / 10) << 4);
	} else {
		hour = newTm.tm_hour >= 12 ? newTm.tm_hour - 12 : newTm.tm_hour;
		_state.Hour = (hour % 10) + ((hour / 10) << 4);
	}

	if(newTm.tm_hour >= 12) {
		//Set PM flag (in both 12h and 24h modes)
		_state.Hour |= 0x80;
	}

	_state.Day = (newTm.tm_mday % 10) + ((newTm.tm_mday / 10) << 4);

	month = newTm.tm_mon + 1;
	_state.Month = (month % 10) + ((month / 10) << 4);

	int year = newTm.tm_year - 100;
	_state.Year = (year % 10) + ((year / 10) << 4);

	int dow = newTm.tm_wday - dowGap;
	_state.DoW = dow < 0 ? (dow + 7) : (dow % 7);

	_lastUpdateTime = currentTime;
}

void GbaRtc::LoadBattery()
{
	vector<uint8_t> rtcData = _emu->GetBatteryManager()->LoadBattery(".rtc");

	if(rtcData.size() == sizeof(_state) + sizeof(uint64_t)) {
		_state.Year = rtcData[0];
		_state.Month = rtcData[1];
		_state.Day = rtcData[2];
		_state.DoW = rtcData[3];
		_state.Hour = rtcData[4];
		_state.Minute = rtcData[5];
		_state.Second = rtcData[6];
		_state.Status = rtcData[7];
		_state.IntHour = rtcData[8];
		_state.IntMinute = rtcData[9];

		uint64_t time = 0;
		for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
			time <<= 8;
			time |= rtcData[sizeof(_state) + i];
		}
		_lastUpdateTime = time;
	} else {
		_lastUpdateTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
}

void GbaRtc::SaveBattery()
{
	vector<uint8_t> rtcData;

	rtcData.push_back(_state.Year);
	rtcData.push_back(_state.Month);
	rtcData.push_back(_state.Day);
	rtcData.push_back(_state.DoW);
	rtcData.push_back(_state.Hour);
	rtcData.push_back(_state.Minute);
	rtcData.push_back(_state.Second);
	rtcData.push_back(_state.Status);
	rtcData.push_back(_state.IntHour);
	rtcData.push_back(_state.IntMinute);

	rtcData.resize(sizeof(_state) + sizeof(uint64_t), 0);

	uint64_t time = _lastUpdateTime;
	for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
		rtcData[sizeof(_state) + i] = (time >> 56) & 0xFF;
		time <<= 8;
	}

	_emu->GetBatteryManager()->SaveBattery(".rtc", rtcData.data(), (uint32_t)rtcData.size());
}

uint8_t GbaRtc::Read()
{
	if(_chipSelect) {
		return (_chipSelect << 2) | (_bitOut << 1) | _clk;
	} else {
		return 0;
	}
}

void GbaRtc::Write(uint8_t value)
{
	uint8_t clk = value & 0x01;

	bool chipSelect = value & 0x04;

	if(!chipSelect || !_chipSelect) {
		_command = 0;
		_bitCounter = 0;
		_clk = 0;
		_dataOut = 0;
		_dataOutSize = 0;
		_dataIn = 0;
		_dataInSize = 0;
		_bitOut = 0;
		_chipSelect = chipSelect;
		return;
	}

	if(clk && !_clk) {
		if(_dataOutSize) {
			ProcessDataOut();
		} else if(_dataInSize) {
			ProcessDataIn(value);
		} else {
			_command <<= 1;
			_command |= (value & 0x02) >> 1;
			_bitCounter++;

			if(_bitCounter == 8) {
				if((_command & 0xF0) == 0x60) {
					ProcessCommand();
				} else {
					//invalid command
					_command = 0;
				}

				_bitCounter = 0;
			}
		}
	}
	_clk = clk;
}

void GbaRtc::Serialize(Serializer& s)
{
	SV(_bitCounter);
	SV(_command);
	SV(_clk);
	SV(_dataOut);
	SV(_dataOutSize);
	SV(_dataIn);
	SV(_dataInSize);
	SV(_bitOut);
	SV(_chipSelect);
	SV(_lastUpdateTime);

	SV(_state.Year);
	SV(_state.Month);
	SV(_state.Day);
	SV(_state.DoW);
	SV(_state.Hour);
	SV(_state.Minute);
	SV(_state.Second);
	SV(_state.Status);
	SV(_state.IntHour);
	SV(_state.IntMinute);
	SV(_state.TestMode);
}
