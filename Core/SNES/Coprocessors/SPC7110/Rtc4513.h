#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class Emulator;

class Rtc4513 : public ISerializable
{
private:
	Emulator* _emu;

	uint64_t _lastTime = 0;
	uint8_t _enabled = 0;
	int8_t _mode = -1;
	int8_t _index = -1;
	uint8_t _regs[0x10] = {};

	bool IsReset() { return (_regs[0xF] & 0x01) != 0; }
	bool IsStop() { return (_regs[0xF] & 0x02) != 0; }
	bool IsHold() { return (_regs[0xD] & 0x01) != 0; }

	uint8_t GetSeconds() { return _regs[0] + ((_regs[1] & 0x07) * 10); }
	uint8_t GetMinutes() { return _regs[2] + ((_regs[3] & 0x07) * 10); }
	uint8_t GetHours() { return _regs[4] + ((_regs[5] & 0x03) * 10); }
	
	uint8_t GetDay() { return _regs[6] + ((_regs[7] & 0x03) * 10); }
	uint8_t GetMonth() { return _regs[8] + ((_regs[9] & 0x01) * 10); }
	uint8_t GetYear() { return _regs[10] + (_regs[11] * 10); }
	uint8_t GetDoW() { return _regs[12] & 0x07; }
	
	void UpdateTime();

public:
	Rtc4513(Emulator* emu);
	virtual ~Rtc4513();

	void LoadBattery();
	void SaveBattery();

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};