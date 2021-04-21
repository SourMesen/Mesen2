#pragma once
#include "stdafx.h"
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
	uint8_t _data[AsciiTurboFile::FileSize];

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		ArrayInfo<uint8_t> data { _data, AsciiTurboFile::FileSize };
		s.Stream(_position, _lastWrite, data);
	}

public:
	AsciiTurboFile(Emulator* emu) : BaseControlDevice(emu, ControllerType::AsciiTurboFile, BaseControlDevice::ExpDevicePort)
	{
		_emu->GetBatteryManager()->LoadBattery(".tf", _data, AsciiTurboFile::FileSize);
	}

	~AsciiTurboFile()
	{
		SaveBattery();
	}

	void SaveBattery() override
	{
		_emu->GetBatteryManager()->SaveBattery(".tf", _data, AsciiTurboFile::FileSize);
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