#pragma once
#include "stdafx.h"
#include "PCE/PceTypes.h"
#include "Utilities/VirtualFile.h"
#include "Shared/MessageManager.h"
#include "Shared/CdReader.h"

class PceConsole;
class PceCdRom;

namespace ScsiSignal
{
	enum ScsiSignal
	{
		Ack,
		Atn,
		Bsy,
		Cd,
		Io,
		Msg,
		Req,
		Rst,
		Sel,
	};
}

enum class ScsiStatus
{
	Good = 0x00,
};

enum class ScsiCommand
{
	TestUnitReady = 0x00,
	RequestSense = 0x03,
	Read = 0x08,
	AudioStartPos = 0xD8,
	AudioEndPos = 0xD9,
	Pause = 0xDA,
	ReadSubCodeQ = 0xDD,
	ReadToc = 0xDE
};

class PceScsiBus
{
private:
	DiscInfo* _disc = nullptr;
	PceConsole* _console = nullptr;
	PceCdRom* _cdrom = nullptr;

	PceScsiBusState _state = {};
	bool _stateChanged = false;
	uint64_t _readStartClock = 0;

	vector<uint8_t> _cmdBuffer;
	deque<uint8_t> _dataBuffer;

	void SetSignals() {}
	void ClearSignals() {}

	template<typename T, typename... T2>
	void SetSignals(T signal, T2... signals)
	{
		SetSignalValue(signal, true);
		SetSignals(signals...);
	}

	template<typename T, typename... T2>
	void ClearSignals(T signal, T2... signals)
	{
		SetSignalValue(signal, false);
		ClearSignals(signals...);
	}

	void SetPhase(ScsiPhase phase);

	void Reset();

	void SetStatusMessage(ScsiStatus status, uint8_t data);

	void ProcessStatusPhase();
	void ProcessMessageInPhase();
	void ProcessDataInPhase();

	uint8_t GetCommandSize(ScsiCommand cmd);
	void ExecCommand(ScsiCommand cmd);
	void ProcessCommandPhase();
	void CmdRead();
	
	uint32_t GetAudioLbaPos();
	void CmdAudioStartPos();
	void CmdAudioEndPos();
	void CmdPause();

	void CmdReadSubCodeQ();
	void CmdReadToc();

	void ProcessDiscRead();

public:
	PceScsiBus(PceConsole* console, PceCdRom* cdRom, DiscInfo& disc);

	PceScsiBusState& GetState() { return _state; }

	uint8_t GetStatus();
	bool IsDataTransferInProgress() { return _state.DataTransfer; }
	
	void SetDataPort(uint8_t data);
	uint8_t GetDataPort();

	bool CheckSignal(::ScsiSignal::ScsiSignal signal);
	void SetSignalValue(::ScsiSignal::ScsiSignal signal, bool val);

	void Exec();
};
