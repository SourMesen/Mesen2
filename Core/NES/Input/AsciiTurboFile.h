#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BatteryManager.h"
#include "Shared/Interfaces/IBattery.h"
#include "Utilities/Serializer.h"

class AsciiTurboFile : public BaseControlDevice, public IBattery
{
private:
	static constexpr int FileSize = 0x2000;
	static constexpr int BitCount = FileSize * 8;
	uint8_t _lastWrite = 0;
	uint16_t _position = 0;
	uint8_t _data[AsciiTurboFile::FileSize] = {};
	NesConsole* _console = nullptr;

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SVArray(_data, AsciiTurboFile::FileSize);
		SV(_position); SV(_lastWrite);
	}

public:
	AsciiTurboFile(NesConsole* console) : BaseControlDevice(console->GetEmulator(), ControllerType::AsciiTurboFile, BaseControlDevice::ExpDevicePort)
	{
		_console = console;
	}

	void Init() override
	{
		_console->InitializeRam(_data, AsciiTurboFile::FileSize);
		_emu->GetBatteryManager()->LoadBattery(".turbofile.sav", _data, AsciiTurboFile::FileSize);
	}

	void SaveBattery() override
	{
		_emu->GetBatteryManager()->SaveBattery(".turbofile.sav", _data, AsciiTurboFile::FileSize);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			return ((_data[_position / 8] >> (_position % 8)) & 0x01) << 2;
		}
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		if(!(value & 0x02)) {
			_position = 0;
		}

		if(!(value & 0x04) && (_lastWrite & 0x04)) {
			//Clock, perform write, increase position
			_data[_position / 8] &= ~(1 << (_position % 8));
			_data[_position / 8] |= (value & 0x01) << (_position % 8);
			_position = (_position + 1) & (AsciiTurboFile::BitCount - 1);
		}

		_lastWrite = value;
	}
};