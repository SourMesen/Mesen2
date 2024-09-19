#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class Emulator;

struct GbaRtcState
{
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t DoW;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;

	uint8_t Status;
	uint8_t IntHour; //unimplemented
	uint8_t IntMinute; //unimplemented

	bool TestMode; //unimplemented
};

class GbaRtc final : public ISerializable
{
private:
	enum class Command : uint8_t
	{
		Reset,
		Status,
		DateTime,
		Time,
		Alarm1,
		Alarm2,
		TestStart,
		TestEnd
	};

	Emulator* _emu = nullptr;

	GbaRtcState _state = {};
	uint64_t _lastUpdateTime = 0;

	uint8_t _bitCounter = 0;
	uint8_t _command = 0;
	uint8_t _clk = 0;

	uint64_t _dataOut = 0;
	uint8_t _dataOutSize = 0;

	uint64_t _dataIn = 0;
	uint8_t _dataInSize = 0;

	uint8_t _bitOut = 0;
	bool _chipSelect = false;

	uint8_t SanitizeData(uint8_t value, uint8_t maxValue, uint8_t fixedValue);
	void FromDateTime(uint64_t data, bool includeYmd);
	uint64_t ToDateTime();

	uint8_t GetCommandLength(Command cmd);

	void ProcessDataIn(uint8_t value);
	void ProcessDataOut();
	void ProcessCommand();
	
	void Reset();
	void UpdateTime();

public:
	GbaRtc(Emulator* emu);

	void LoadBattery();
	void SaveBattery();

	uint8_t Read();
	void Write(uint8_t value);

	void Serialize(Serializer& s) override;
};
