#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;
class PceConsole;
class PceCdRom;
struct DiscInfo;

namespace ScsiSignal
{
	enum ScsiSignal
	{
		Ack,
		Atn, //Unused
		Bsy, //Set when an operation is in progress
		Cd, //Set when the current transfer contains commands/messages/status (cleared when actual data)
		Io, //Set when drive is sending data to the software (cleared when receiving)
		Msg, //Set when a message is being sent/received
		Req, //Set when drive is waiting for the software to reply
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

enum class ScsiUpdateType
{
	SetCmdPhase,
	SetReqSignal,
	ClearReqSignal,
	SetDataInPhase,
	SetTransferDoneIrq,
	SetGoodStatus
};

class PceScsiBus : public ISerializable
{
private:
	constexpr static int ReadBytesPerSecond = 153600; //75 frames/sectors of 2048 bytes per second, like audio
	
	Emulator* _emu = nullptr;
	DiscInfo* _disc = nullptr;
	PceConsole* _console = nullptr;
	PceCdRom* _cdrom = nullptr;

	PceScsiBusState _state = {};
	bool _stateChanged = false;
	
	int64_t _readSectorCounter = 0;
	int32_t _ackClearCounter = 0;
	int32_t _updateCounter = 0;
	ScsiUpdateType _updateType = {};
	bool _needExec = true;

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

	void ProcessStatusPhase();
	void ProcessMessageInPhase();
	void QueueDriveUpdate(ScsiUpdateType action, uint32_t delay);
	void ProcessDataInPhase();

	uint8_t GetCommandSize(ScsiCommand cmd);
	void ExecCommand(ScsiCommand cmd);
	void LogCommand(string msg);
	void ProcessCommandPhase();
	
	int64_t GetSeekTime(uint32_t startLba, uint32_t targetLba);
	uint64_t GetSectorLoadTime();
	void CmdRead();
	
	uint32_t GetAudioLbaPos();
	void CmdAudioStartPos();
	void CmdAudioEndPos();
	void CmdPause();
	void CmdTestReadyUnit();
	void CmdReadSubCodeQ();
	void CmdReadToc();

	void ProcessDiscRead();

public:
	PceScsiBus(Emulator* emu, PceConsole* console, PceCdRom* cdRom, DiscInfo& disc);

	void SetStatusMessage(ScsiStatus status, uint8_t data, uint32_t length = 1);

	PceScsiBusState& GetState() { return _state; }

	uint8_t GetStatus();
	bool IsDataBlockReady() { return _dataBuffer.size() > 0; }
	
	void SetDataPort(uint8_t data);
	uint8_t GetDataPort();

	bool CheckSignal(::ScsiSignal::ScsiSignal signal);
	void SetSignalValue(::ScsiSignal::ScsiSignal signal, bool val);

	void SetAckWithAutoClear();

	void UpdateState();
	__noinline void Exec();
	
	bool NeedExec() { return _needExec; }

	void Serialize(Serializer& s) override;
};
