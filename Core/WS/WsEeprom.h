#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"

class WsConsole;
class Emulator;

enum class WsEepromCommand
{
	Read,
	Write,
	Erase,
	WriteDisable,
	WriteEnable,
	EraseAll,
	WriteAll,
	Unknown
};

class WsEeprom final : public ISerializable
{
private:
	WsEepromState _state = {};
	WsConsole* _console = nullptr;
	Emulator* _emu = nullptr;
	uint8_t* _data = nullptr;
	bool _isInternal = false;

	WsEepromCommand GetCommand();
	WsEepromSize GetSize();
	uint16_t GetCommandAddress();

	void WriteValue(uint16_t addr, uint16_t value);

	string ConvertToEepromString(string in);
	void InitInternalEepromData();

public:
	WsEeprom(Emulator* emu, WsConsole* console, WsEepromSize size, uint8_t* eepromData, bool isInternal);

	WsEepromState& GetState() { return _state; }

	void WritePort(uint8_t port, uint8_t value);
	uint8_t ReadPort(uint8_t port);

	void LoadBattery();
	void SaveBattery();

	void Serialize(Serializer& s);
};