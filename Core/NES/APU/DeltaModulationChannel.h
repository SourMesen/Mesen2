#pragma once
#include "pch.h"
#include "NES/APU/ApuTimer.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"

class NesConsole;

class DeltaModulationChannel : public INesMemoryHandler, public ISerializable
{
private:	
	static constexpr uint16_t _dmcPeriodLookupTableNtsc[16] = { 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54 };
	static constexpr uint16_t _dmcPeriodLookupTablePal[16] = { 398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118,  98,  78,  66,  50 };

	NesConsole* _console;
	ApuTimer _timer;

	uint16_t _sampleAddr = 0;
	uint16_t _sampleLength = 0;
	uint8_t _outputLevel = 0;
	bool _irqEnabled = false;
	bool _loopFlag = false;

	uint16_t _currentAddr = 0;
	uint16_t _bytesRemaining = 0;
	uint8_t _readBuffer = 0;
	bool _bufferEmpty = true;

	uint8_t _shiftRegister = 0;
	uint8_t _bitsRemaining = 0;
	bool _silenceFlag = true;
	bool _needToRun = false;
	uint8_t _disableDelay = 0;
	uint8_t _transferStartDelay = 0;

	uint8_t _lastValue4011 = 0;

	void InitSample();

public:
	DeltaModulationChannel(NesConsole* console);

	void Run(uint32_t targetCycle);

	void Reset(bool softReset);
	void Serialize(Serializer& s) override;

	bool IrqPending(uint32_t cyclesToRun);
	bool NeedToRun();
	bool GetStatus();
	void GetMemoryRanges(MemoryRanges &ranges) override;
	void WriteRam(uint16_t addr, uint8_t value) override;
	uint8_t ReadRam(uint16_t addr) override { return 0; }
	void EndFrame();

	void SetEnabled(bool enabled);
	void ProcessClock();
	void StartDmcTransfer();
	uint16_t GetDmcReadAddress();
	void SetDmcReadBuffer(uint8_t value);

	uint8_t GetOutput() { return _timer.GetLastOutput(); }
	ApuDmcState GetState();
};