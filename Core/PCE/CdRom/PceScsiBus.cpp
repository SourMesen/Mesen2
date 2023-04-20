#include "pch.h"
#include "PCE/CdRom/PceScsiBus.h"
#include "PCE/CdRom/PceCdRom.h"
#include "PCE/CdRom/PceCdAudioPlayer.h"
#include "PCE/PceConsole.h"
#include "PCE/PceTypes.h"
#include "Shared/CdReader.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/Serializer.h"

using namespace ScsiSignal;

PceScsiBus::PceScsiBus(Emulator* emu, PceConsole* console, PceCdRom* cdrom, DiscInfo& disc)
{
	_emu = emu;
	_disc = &disc;
	_console = console;
	_cdrom = cdrom;
}

void PceScsiBus::SetPhase(ScsiPhase phase)
{
	if(_state.Phase == phase) {
		return;
	}

	_stateChanged = true;
	_needExec = true;
	_state.Phase = phase;
	ClearSignals(Bsy, Cd, Io, Msg, Req);
	
	//LogDebug("[SCSI] Phase changed: " + string(magic_enum::enum_name(phase)));

	switch(_state.Phase) {
		case ScsiPhase::BusFree: _cdrom->ClearIrqSource(PceCdRomIrqSource::DataTransferDone); break;
		case ScsiPhase::Command: SetSignals(Bsy, Cd, Req); break;
		case ScsiPhase::DataIn: SetSignals(Bsy, Io); break;
		case ScsiPhase::DataOut: SetSignals(Bsy, Req); break;
		case ScsiPhase::MessageIn: SetSignals(Bsy, Cd, Io, Msg, Req); break;
		case ScsiPhase::MessageOut: SetSignals(Bsy, Cd, Msg, Req); break;
		case ScsiPhase::Status: SetSignals(Bsy, Cd, Io, Req); break;
	}
}

void PceScsiBus::Reset()
{
	ClearSignals(Ack, Atn, Cd, Io, Msg, Req);
	_state.Phase = ScsiPhase::BusFree;
	_state.DataPort = 0;
	_state.DataTransferDone = false;
	_readSectorCounter = 0;
	_cmdBuffer.clear();
	_dataBuffer.clear();
	_cdrom->GetAudioPlayer().Stop();
	//LogDebug("[SCSI] Reset");
}

void PceScsiBus::SetStatusMessage(ScsiStatus status, uint8_t data)
{
	_state.MessageData = data;
	_state.StatusDone = false;
	_state.MessageDone = false;
	_state.DataPort = (uint8_t)status;
	SetPhase(ScsiPhase::Status);
}

void PceScsiBus::ProcessStatusPhase()
{
	if(_state.Signals[Req] && _state.Signals[Ack]) {
		ClearSignals(Req);
		_state.StatusDone = true;
	} else if(!_state.Signals[Req] && !_state.Signals[Ack] && _state.StatusDone) {
		_state.StatusDone = false;
		_state.DataPort = _state.MessageData;
		SetPhase(ScsiPhase::MessageIn);
	}
}

void PceScsiBus::ProcessMessageInPhase()
{
	if(_state.Signals[Req] && _state.Signals[Ack]) {
		ClearSignals(Req);
		_state.MessageDone = true;
	} else if(!_state.Signals[Req] && !_state.Signals[Ack] && _state.MessageDone) {
		_state.MessageDone = false;
		SetPhase(ScsiPhase::BusFree);
	}
}

void PceScsiBus::ProcessDataInPhase()
{
	if(_state.Signals[Req] && _state.Signals[Ack]) {
		ClearSignals(Req);
	} else if(!_state.Signals[Req] && !_state.Signals[Ack]) {
		if(_dataBuffer.size() > 0) {
			_state.DataPort = _dataBuffer.front();
			_dataBuffer.pop_front();
			SetSignals(Req);
		} else {
			_cdrom->ClearIrqSource(PceCdRomIrqSource::DataTransferReady);
			if(_state.DataTransferDone) {
				_state.DataTransferDone = false;
				_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
				SetStatusMessage(ScsiStatus::Good, 0);
			}
		}
	}
}

uint8_t PceScsiBus::GetCommandSize(ScsiCommand cmd)
{
	switch(cmd) {
		case ScsiCommand::TestUnitReady:
		case ScsiCommand::Read:
			return 6;

		case ScsiCommand::AudioStartPos:
		case ScsiCommand::AudioEndPos:
		case ScsiCommand::Pause:
		case ScsiCommand::ReadSubCodeQ:
		case ScsiCommand::ReadToc:
			return 10;

		default:
			return 0;
	}
}

void PceScsiBus::ExecCommand(ScsiCommand cmd)
{
	switch(cmd) {
		case ScsiCommand::TestUnitReady: CmdTestReadyUnit(); break;
		case ScsiCommand::Read: CmdRead(); break;
		case ScsiCommand::AudioStartPos: CmdAudioStartPos(); break;
		case ScsiCommand::AudioEndPos: CmdAudioEndPos(); break;
		case ScsiCommand::Pause: CmdPause(); break;
		case ScsiCommand::ReadSubCodeQ: CmdReadSubCodeQ(); break;
		case ScsiCommand::ReadToc: CmdReadToc(); break;
	}
}

void PceScsiBus::LogCommand(string msg)
{
	if(!_emu->IsDebugging()) {
		return;
	}

	msg = "[SCSI] CMD: " + msg + " - ";
	for(size_t i = 0, len = _cmdBuffer.size(); i < len; i++) {
		msg += " $" + HexUtilities::ToHex(_cmdBuffer[i]);
	}
	_emu->DebugLog(msg);
}

void PceScsiBus::ProcessCommandPhase()
{
	if(_state.Signals[Req] && _state.Signals[Ack]) {
		ClearSignals(Req);
		_cmdBuffer.push_back(_state.DataPort);
	} else if(!_state.Signals[Req] && !_state.Signals[Ack] && _cmdBuffer.size() > 0) {
		ScsiCommand cmd = (ScsiCommand)_cmdBuffer[0];
		uint8_t cmdSize = GetCommandSize(cmd);
		if(cmdSize == 0) {
			//Unknown/unsupported command
			if(_emu->IsDebugging()) {
				LogCommand("Unknown command - " + HexUtilities::ToHex(_cmdBuffer[0]));
			}
			SetStatusMessage(ScsiStatus::Good, 0);
		} else if(cmdSize <= _cmdBuffer.size()) {
			//All bytes received - command has been processed
			//LogDebug("[SCSI] Command recv: " + HexUtilities::ToHex(_cmdBuffer, ' '));
			ExecCommand(cmd);
			_cmdBuffer.clear();
		} else {
			//Command still requires more byte to be executed
			SetSignals(Req);
		}
	}
}

int64_t PceScsiBus::GetSeekTime(uint32_t startLba, uint32_t targetLba)
{
	//This should yield the same seek times as the code in ares?
	int gap = std::abs((int)startLba - (int)targetLba);
	int frameCount = 17 + std::pow(std::sqrt(gap), 0.99) * 0.3;
	return _console->GetMasterClockRate() * frameCount / 75;
}

void PceScsiBus::CmdRead()
{
	uint32_t sector = _cmdBuffer[3] | (_cmdBuffer[2] << 8) | ((_cmdBuffer[1] & 0x1F) << 16);
	uint8_t sectorsToRead = _cmdBuffer[4];

	if(sectorsToRead == 0) {
		LogCommand("Read - No sectors to read");
		SetStatusMessage(ScsiStatus::Good, 0);
		return;
	}

	_readSectorCounter = GetSeekTime(_state.Sector, sector);
	_needExec = true;
	_state.Sector = sector;
	_state.SectorsToRead = sectorsToRead;

	_cdrom->GetAudioPlayer().SetIdle();
	if(_emu->IsDebugging()) {
		LogCommand("Read - Sector: " + std::to_string(_state.Sector) + " to " + std::to_string(_state.Sector + _state.SectorsToRead - 1));
	}
}

uint32_t PceScsiBus::GetAudioLbaPos()
{
	switch(_cmdBuffer[9] & 0xC0) {
		case 0x00:
			return (_cmdBuffer[3] << 16) | (_cmdBuffer[4] << 8) | _cmdBuffer[5];

		case 0x40: {
			DiscPosition pos;
			pos.Minutes = CdReader::FromBcd(_cmdBuffer[2]);
			pos.Seconds = CdReader::FromBcd(_cmdBuffer[3]);
			pos.Frames = CdReader::FromBcd(_cmdBuffer[4]);
			return pos.ToLba() - 150;
		}

		case 0x80: {
			uint8_t track = CdReader::FromBcd(_cmdBuffer[2]);
			int32_t sector = _disc->GetTrackFirstSector(track - 1);
			return sector >= 0 ? sector : 0;
		}
	}
	
	LogDebug("[SCSI] CMD: Audio pos - invalid param");
	return 0;
}

void PceScsiBus::CmdAudioStartPos()
{
	uint32_t startSector = GetAudioLbaPos();

	if(_emu->IsDebugging()) {
		LogCommand("Audio Start Position - " + std::to_string(startSector));
	}

	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	if(_cmdBuffer[1] == 0) {
		player.Play(startSector, true);
	} else {
		player.Play(startSector, false);
	}

	SetStatusMessage(ScsiStatus::Good, 0);
	_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
}

void PceScsiBus::CmdAudioEndPos()
{
	uint32_t endSector = GetAudioLbaPos();

	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	switch(_cmdBuffer[1]) {
		case 0: player.Stop(); break;
		case 1: player.SetEndPosition(endSector, CdPlayEndBehavior::Loop); break;
		case 2: player.SetEndPosition(endSector, CdPlayEndBehavior::Irq); break;
		case 3: player.SetEndPosition(endSector, CdPlayEndBehavior::Stop); break;
	}

	if(_emu->IsDebugging()) {
		LogCommand("Audio End Position - " + std::to_string(endSector));
	}

	SetStatusMessage(ScsiStatus::Good, 0);
}

void PceScsiBus::CmdPause()
{
	if(_emu->IsDebugging()) {
		LogCommand("Audio Pause");
	}
	_cdrom->GetAudioPlayer().Pause();
	SetStatusMessage(ScsiStatus::Good, 0);
}

void PceScsiBus::CmdTestReadyUnit()
{
	if(_emu->IsDebugging()) {
		LogCommand("Test Ready Unit");
	}
	SetStatusMessage(ScsiStatus::Good, 0);
}

void PceScsiBus::CmdReadSubCodeQ()
{
	if(_emu->IsDebugging()) {
		LogCommand("Read Sub Code Q");
	}
	
	_dataBuffer.clear();

	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	
	CdAudioStatus audioStatus = player.GetStatus();
	uint32_t sector = audioStatus != CdAudioStatus::Inactive ? player.GetCurrentSector() : _state.Sector;
	uint32_t track = _disc->GetTrack(sector);

	bool isData = _disc->Tracks[track].Format != TrackFormat::Audio;
	uint8_t adrControl = (
		0x01 | //ADR - 1 = "sub-channel Q encodes current position data"
		(isData ? 0x40 : 0x00) //Control field - Bit 2: clear = audio track, set = data track
	);

	uint32_t sectorGap = _disc->Tracks[track].FirstSector - sector;
	DiscPosition relPos = DiscPosition::FromLba(sectorGap);
	DiscPosition absPos = DiscPosition::FromLba(sector);

	_dataBuffer.push_back((uint8_t)audioStatus);

	_dataBuffer.push_back(adrControl); //ADR + Control 
	_dataBuffer.push_back(CdReader::ToBcd(track + 1)); //track number
	_dataBuffer.push_back(1); //index number
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Minutes));
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Seconds));
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Frames));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Minutes));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Seconds));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Frames));

	_state.DataTransferDone = true;
	SetPhase(ScsiPhase::DataIn);
}

void PceScsiBus::CmdReadToc()
{
	switch(_cmdBuffer[1]) {
		case 0:
			//Number of tracks
			if(_emu->IsDebugging()) {
				LogCommand("Read ToC - Number of Tracks");
			}

			_dataBuffer.clear();
			_dataBuffer.push_back(1);
			_dataBuffer.push_back(CdReader::ToBcd((uint8_t)_disc->Tracks.size()));
			_state.DataTransferDone = true;
			SetPhase(ScsiPhase::DataIn);
			break;

		case 1: {
			//Total disc length
			if(_emu->IsDebugging()) {
				LogCommand("Read ToC - Disc Length");
			}

			_dataBuffer.clear();
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Minutes));
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Seconds));
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Frames));
			_state.DataTransferDone = true;
			SetPhase(ScsiPhase::DataIn);
			break;
		}

		case 2: {
			uint8_t track = CdReader::FromBcd(_cmdBuffer[2]);
			if(_emu->IsDebugging()) {
				LogCommand("Read ToC - Track #" + std::to_string(track) + " Length");
			}

			if(track == 0) {
				track = 1;
			}

			DiscPosition pos;
			if(track > _disc->Tracks.size()) {
				pos = _disc->EndPosition;
			} else {
				pos = DiscPosition::FromLba(_disc->Tracks[track - 1].StartPosition.ToLba() + 150);
			}

			_dataBuffer.clear();
			_dataBuffer.push_back(CdReader::ToBcd(pos.Minutes));
			_dataBuffer.push_back(CdReader::ToBcd(pos.Seconds));
			_dataBuffer.push_back(CdReader::ToBcd(pos.Frames));
			if(track > _disc->Tracks.size() || _disc->Tracks[track - 1].Format == TrackFormat::Audio) {
				_dataBuffer.push_back(0);
			} else {
				_dataBuffer.push_back(4);
			}

			_state.DataTransferDone = true;
			SetPhase(ScsiPhase::DataIn);
			break;
		}

		default:
			if(_emu->IsDebugging()) {
				LogCommand("Read ToC - Unsupported parameters - " + HexUtilities::ToHex(_cmdBuffer[1]));
			}
			break;
	}
}

void PceScsiBus::ProcessDiscRead()
{
	if(_readSectorCounter > 0) {
		_readSectorCounter -= 3;

		if(_readSectorCounter <= 0) {
			if(_dataBuffer.empty()) {
				//read disc data
				_dataBuffer.clear();
				_disc->ReadDataSector(_state.Sector, _dataBuffer);

				LogDebug("[SCSI] Sector #" + std::to_string(_state.Sector) + " finished reading.");

				_state.Sector = (_state.Sector + 1) & 0x1FFFFF;
				_state.SectorsToRead--;

				//Mark state as changed because this will cause the Req signal
				//to be set on the next run of ProcessDataInPhase()
				//Otherwise bios code will wait on Req in an infinite loop
				_stateChanged = true;
				_needExec = true;

				_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferReady);

				if(_state.SectorsToRead == 0) {
					_state.DataTransferDone = true;
					_readSectorCounter = 0;
					LogDebug("[SCSI] Read operation done");
				} else {
					_state.DataTransferDone = false;
					_readSectorCounter = (int64_t)((double)_console->GetMasterClockRate() * 2048 / PceScsiBus::ReadBytesPerSecond);
				}

				SetPhase(ScsiPhase::DataIn);
			} else {
				LogDebug("[SCSI] Read sector done but buffer not empty, delay");
				_readSectorCounter = (int64_t)((double)_console->GetMasterClockRate() * 2048 / PceScsiBus::ReadBytesPerSecond);
				_needExec = true;
			}
		}
	}
}

uint8_t PceScsiBus::GetStatus()
{
	return (
		(_state.Signals[Io] ? 0x08 : 0) |
		(_state.Signals[Cd] ? 0x10 : 0) |
		(_state.Signals[Msg] ? 0x20 : 0) |
		(_state.Signals[Req] ? 0x40 : 0) |
		(_state.Signals[Bsy] ? 0x80 : 0)
	);
}

void PceScsiBus::SetDataPort(uint8_t data)
{
	//LogDebug("[SCSI] CPU data port write: " + HexUtilities::ToHex(data));
	_state.DataPort = data;
}

uint8_t PceScsiBus::GetDataPort()
{
	//LogDebug("[SCSI] CPU data port read: " + HexUtilities::ToHex(_state.DataPort));
	return _state.DataPort;
}

bool PceScsiBus::CheckSignal(::ScsiSignal::ScsiSignal signal)
{
	return _state.Signals[signal];
}

void PceScsiBus::SetSignalValue(::ScsiSignal::ScsiSignal signal, bool val)
{
	if(_state.Signals[signal] != val) {
		_state.Signals[signal] = val;
		_stateChanged = true;
		_needExec = true;
	}
}

void PceScsiBus::SetAckWithAutoClear()
{
	//Set the Ack signal for 60 master clocks (and then clear it again)
	//Used after manually reading a byte of data (or after ADPCM DMA reads one)
	SetSignals(Ack);
	_ackClearCounter = 20*3;
	_needExec = true;
}

void PceScsiBus::UpdateState()
{
	if(!_stateChanged) {
		return;
	}

	if(_state.Signals[Rst]) {
		Reset();
		return;
	}

	do {
		_stateChanged = false;

		if(!_state.Signals[Bsy] && _state.Signals[Sel]) {
			SetPhase(ScsiPhase::Command);
		} else if(!_state.Signals[Ack] && _state.Signals[Atn] && !_state.Signals[Req]) {
			SetPhase(ScsiPhase::MessageOut);
		} else {
			switch(_state.Phase) {
				case ScsiPhase::Command: ProcessCommandPhase(); break;
				case ScsiPhase::DataIn: ProcessDataInPhase(); break;
				//case ScsiPhase::DataOut: ProcessDataOutPhase(); break;
				case ScsiPhase::MessageIn: ProcessMessageInPhase(); break;
				//case ScsiPhase::MessageOut: ProcessMessageOutPhase(); break;
				case ScsiPhase::Status: ProcessStatusPhase(); break;
			}
		}
	} while(_stateChanged);
}

void PceScsiBus::Exec()
{
	if(_ackClearCounter > 0) {
		_ackClearCounter -= 3;
		if(_ackClearCounter == 0) {
			SetSignalValue(Ack, false);
		}
	}

	if(_readSectorCounter > 0) {
		ProcessDiscRead();
	}

	if(_stateChanged) {
		UpdateState();
	}

	_needExec = _stateChanged || _readSectorCounter > 0 || _ackClearCounter > 0;
}

void PceScsiBus::Serialize(Serializer& s)
{
	for(int i = 0; i < 9; i++) {
		SVI(_state.Signals[i]);
	}

	SV(_state.Phase);
	SV(_state.StatusDone);
	SV(_state.MessageDone);
	SV(_state.MessageData);
	SV(_state.DataPort);
	SV(_state.DataTransferDone);
	SV(_state.Sector);
	SV(_state.SectorsToRead);
	
	SV(_stateChanged);
	SV(_readSectorCounter);
	SV(_ackClearCounter);
	SV(_cmdBuffer);

	if(s.IsSaving()) {
		vector<uint8_t> dataBuffer = vector<uint8_t>(_dataBuffer.begin(), _dataBuffer.end());
		SV(dataBuffer);
	} else {
		vector<uint8_t> dataBuffer;
		SV(dataBuffer);		
		_dataBuffer.clear();
		_dataBuffer.insert(_dataBuffer.end(), dataBuffer.begin(), dataBuffer.end());

		_needExec = true;
	}
}
