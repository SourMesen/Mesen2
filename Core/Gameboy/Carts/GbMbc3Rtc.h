#include "pch.h"
#include <time.h>
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbMbc3Rtc : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	uint8_t _regs[5] = {};
	uint8_t _latchedRegs[5] = {};
	uint8_t _latchValue = 0;
	uint64_t _lastMasterClock = 0;
	uint64_t _tickCounter = 0;

public:
	bool IsRunning() { return (_regs[4] & 0x40) == 0; }
	uint8_t GetSeconds() { return _regs[0]; }
	uint8_t GetMinutes() { return _regs[1]; }
	uint8_t GetHours() { return _regs[2]; }
	uint16_t GetDayCount() { return _regs[3] | ((_regs[4] & 0x01) << 8); }

	GbMbc3Rtc(Emulator* emu)
	{
		_emu = emu;
		_lastMasterClock = 0;
		LoadBattery();
	}

	~GbMbc3Rtc()
	{
		SaveBattery();
	}

	void LoadBattery()
	{
		vector<uint8_t> rtcData = _emu->GetBatteryManager()->LoadBattery(".rtc");

		if(rtcData.size() == sizeof(_regs) + sizeof(uint64_t)) {
			memcpy(_regs, rtcData.data(), sizeof(_regs));
			uint64_t time = 0;
			for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
				time <<= 8;
				time |= rtcData[sizeof(_regs) + i];
			}

			int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - time;
			if(elapsedMs > 0) {
				//Run clock forward based on how much time has passed since the game was turned off
				RunForDuration(elapsedMs / 1000);
			}
		}
	}

	void SaveBattery()
	{
		vector<uint8_t> rtcData;
		rtcData.resize(sizeof(_regs) + sizeof(uint64_t), 0);

		memcpy(rtcData.data(), _regs, sizeof(_regs));
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		for(uint32_t i = 0; i < sizeof(uint64_t); i++) {
			rtcData[sizeof(_regs) + i] = (time >> 56) & 0xFF;
			time <<= 8;
		}

		_emu->GetBatteryManager()->SaveBattery(".rtc", rtcData.data(), (uint32_t)rtcData.size());
	}

	void UpdateTime()
	{
		uint64_t elaspedClocks = _emu->GetMasterClock() - _lastMasterClock;
		uint64_t clocksPerTick = _emu->GetMasterClockRate() / 32768;
		if(IsRunning()) {
			_tickCounter += elaspedClocks / clocksPerTick;
			while(_tickCounter >= 32768) {
				RunForDuration(1);
				_tickCounter -= 32768;
			}
		}
		
		uint64_t remainder = elaspedClocks % clocksPerTick;
		_lastMasterClock = _emu->GetMasterClock() - remainder;
	}

	void RunForDuration(int64_t seconds)
	{
		while(seconds > 0) {
			int64_t maxAmount = _regs[0] >= 60 ? 1 : std::min<int64_t>(60 - _regs[0], seconds);

			seconds -= maxAmount;
			_regs[0] = (_regs[0] + maxAmount) & 0x3F;

			if(GetSeconds() == 60) {
				_regs[0] = 0;
				_regs[1] = (_regs[1] + 1) & 0x3F;

				if(GetMinutes() == 60) {
					_regs[1] = 0;
					_regs[2] = (_regs[2] + 1) & 0x1F;

					if(GetHours() == 24) {
						_regs[2] = 0;

						if(GetDayCount() == 0xFF) {
							_regs[3] = 0;
							_regs[4] |= 0x01;
						} else if(GetDayCount() == 0x1FF) {
							_regs[3] = 0;
							_regs[4] &= ~0x01;
							_regs[4] |= 0x80; //set overflow
						}
					}
				}
			}
		}
	}

	void LatchData()
	{
		//Despite documentation saying otherwise, the latch-rtc-test only works if latching
		//is done on every write (rather than latching on a 0->1 transition on bit 0)
		UpdateTime();
		memcpy(_latchedRegs, _regs, sizeof(_regs));
	}

	uint8_t Read(uint16_t addr)
	{
		switch(addr) {
			case 0x08: return _latchedRegs[0]; //Seconds
			case 0x09: return _latchedRegs[1]; //Minutes
			case 0x0A: return _latchedRegs[2]; //Hours
			case 0x0B: return _latchedRegs[3]; //Day counter
			case 0x0C: return _latchedRegs[4]; //Day counter (upper bit) + carry/halt flags
		}

		return 0xFF;
	}

	void Write(uint16_t addr, uint8_t value)
	{
		UpdateTime();

		switch(addr) {
			case 0x08:
				//Seconds
				_regs[0] = value & 0x3F;

				//Reset timer
				_lastMasterClock = _emu->GetMasterClock();
				_tickCounter = 0;
				break;

			case 0x09: _regs[1] = value & 0x3F; break; //Minutes
			case 0x0A: _regs[2] = value & 0x1F; break; //Hours
			case 0x0B: _regs[3] = value; break; //Day counter
			case 0x0C: _regs[4] = value & 0xC1; break; //Day counter (upper bit) + carry/halt flags
		}
	}

	void Serialize(Serializer& s)
	{
		SVArray(_regs, 5);
		SVArray(_latchedRegs, 5);
		SV(_latchValue);
		SV(_lastMasterClock);
		SV(_tickCounter);
	}
};