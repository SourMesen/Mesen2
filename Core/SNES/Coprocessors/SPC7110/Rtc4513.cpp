#include "pch.h"
#include <time.h>
#include "SNES/Coprocessors/SPC7110/Rtc4513.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/BatteryManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

//TODO: Partial implementation
//Missing stuff: most flags e.g: 30ADJ, 24/12, CAL/HW, WRAP, etc.

Rtc4513::Rtc4513(Emulator* emu)
{
	_emu = emu;
}

Rtc4513::~Rtc4513()
{
}

void Rtc4513::LoadBattery()
{
	vector<uint8_t> rtcData = _emu->GetBatteryManager()->LoadBattery(".rtc");
	
	if(rtcData.size() == sizeof(_regs) + sizeof(uint64_t)) {
		memcpy(_regs, rtcData.data(), sizeof(_regs));
		uint64_t time = 0;
		for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
			time <<= 8;
			time |= rtcData[sizeof(_regs) + i];
		}
		_lastTime = time;
	} else {
		_lastTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
}

void Rtc4513::SaveBattery()
{
	vector<uint8_t> rtcData;
	rtcData.resize(sizeof(_regs) + sizeof(uint64_t), 0);

	memcpy(rtcData.data(), _regs, sizeof(_regs));
	uint64_t time = _lastTime;
	for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
		rtcData[sizeof(_regs) + i] = (time >> 56) & 0xFF;
		time <<= 8;
	}

	_emu->GetBatteryManager()->SaveBattery(".rtc", rtcData.data(), (uint32_t)rtcData.size());
}

void Rtc4513::UpdateTime()
{
	if(IsReset()) {
		//Reset seconds to 0
		_regs[0] = 0;
		_regs[1] = 0;
	}

	uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	uint32_t elapsedSeconds = (uint32_t)(currentTime - _lastTime);
	if(elapsedSeconds <= 0) {
		return;
	}

	if(IsStop() || IsReset() || IsHold()) {
		_lastTime = currentTime;
		return;
	}

	std::tm tm = { };
	tm.tm_sec = GetSeconds();
	tm.tm_min = GetMinutes();
	tm.tm_hour = GetHours();
	tm.tm_mday = GetDay();
	tm.tm_mon = GetMonth() - 1;
	tm.tm_year = (GetYear() >= 90 ? 0 : 100) + GetYear();

	std::time_t tt = mktime(&tm);
	if(tt == -1 || GetMonth() == 0) {
		_lastTime = currentTime;
		return;
	}

	int8_t dowGap = 0;
	if(tm.tm_wday != GetDoW()) {
		//The DoW on the RTC can be set to any arbitrary value for a specific date
		//Check the gap between the value set by the game & the real dow for that date
		dowGap = (int8_t)tm.tm_wday - (int8_t)GetDoW();
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

	_regs[0] = newTm.tm_sec % 10;
	_regs[1] = newTm.tm_sec / 10;

	_regs[2] = newTm.tm_min % 10;
	_regs[3] = newTm.tm_min / 10;

	_regs[4] = newTm.tm_hour % 10;
	_regs[5] = newTm.tm_hour / 10;

	_regs[6] = newTm.tm_mday % 10;
	_regs[7] = newTm.tm_mday / 10;

	_regs[8] = (newTm.tm_mon + 1) % 10;
	_regs[9] = (newTm.tm_mon + 1) / 10;

	int year = newTm.tm_year + 1900;
	year -= year >= 2000 ? 2000 : 1900;

	_regs[10] = year % 10;
	_regs[11] = year / 10;
	
	int dow = newTm.tm_wday - dowGap;
	_regs[12] = dow < 0 ? (dow + 7) : (dow % 7);

	_lastTime = currentTime;
}

uint8_t Rtc4513::Read(uint16_t addr)
{
	UpdateTime();

	switch(addr) {
		case 0x4840: break;
		
		case 0x4841: 
			if(_mode == 0x0C) {
				//Read mode
				//LogDebug("Read: " + HexUtilities::ToHex(_index) + " = " + HexUtilities::ToHex(_regs[_index]));
				uint8_t index = _index;
				_index = (_index + 1) & 0x0F;
				return _regs[index];
			} 
			break;
		
		case 0x4842:
			//Ready
			return 0x80;
	}

	return 0;
}

void Rtc4513::Write(uint16_t addr, uint8_t value)
{
	UpdateTime();

	switch(addr) {
		case 0x4840: 
			_enabled = value; 
			if(!(_enabled & 0x01)) {
				_mode = -1;
				_index = -1;

				//Turn off reset ($01) and test ($08) bits when disabled
				_regs[0x0F] &= 0x06;
			}
			break;

		case 0x4841:
			if(_mode == -1) {
				_mode = value & 0x0F;
			} else if(_index == -1) {
				_index = value & 0x0F;
			} else if(_mode == 0x03) {
				//Write mode
				//LogDebug(HexUtilities::ToHex(_index) + " = " + HexUtilities::ToHex(value & 0x0F));
				uint8_t index = _index;
				_index = (_index + 1) & 0x0F;
				_regs[index] = value & 0x0F;
			}
			break;
		
		case 0x4842: break;
	}
}

void Rtc4513::Serialize(Serializer& s)
{
	SVArray(_regs, 0x10);
	SV(_lastTime); SV(_enabled); SV(_mode); SV(_index);
}
