#include "stdafx.h"
#include "PCE/PceScsiBus.h"
#include "PCE/PceConsole.h"
#include "PCE/PceCdRom.h"
#include "PCE/PceCdAudioPlayer.h"
#include "Shared/CdReader.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"

using namespace ScsiSignal;

PceScsiBus::PceScsiBus(PceConsole* console, PceCdRom* cdrom, DiscInfo& disc)
{
	_disc = &disc;
	_console = console;
	_cdrom = cdrom;
}

void PceScsiBus::SetPhase(ScsiPhase phase)
{
	if(_phase == phase) {
		return;
	}

	_stateChanged = true;
	_phase = phase;
	ClearSignals(Bsy, Cd, Io, Msg, Req);
	
	//LogDebug("[SCSI] Phase changed: " + HexUtilities::ToHex((int)phase));

	switch(_phase) {
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
	_phase = ScsiPhase::BusFree;
	_dataPort = 0;
	_discReading = false;
	_dataTransfer = false;
	_dataTransferDone = false;
	_cmdBuffer.clear();
	_dataBuffer.clear();
	_cdrom->GetAudioPlayer().Stop();
	//LogDebug("[SCSI] Reset");
}

void PceScsiBus::SetStatusMessage(ScsiStatus status, uint8_t data)
{
	_messageData = data;
	_statusDone = false;
	_messageDone = false;
	_dataPort = (uint8_t)status;
	SetPhase(ScsiPhase::Status);
}

void PceScsiBus::ProcessStatusPhase()
{
	if(_signals[Req] && _signals[Ack]) {
		ClearSignals(Req);
		_statusDone = true;
	} else if(!_signals[Req] && !_signals[Ack] && _statusDone) {
		_statusDone = false;
		_dataPort = _messageData;
		SetPhase(ScsiPhase::MessageIn);
	}
}

void PceScsiBus::ProcessMessageInPhase()
{
	if(_signals[Req] && _signals[Ack]) {
		ClearSignals(Req);
		_messageDone = true;
	} else if(!_signals[Req] && !_signals[Ack] && _messageDone) {
		_messageDone = false;
		SetPhase(ScsiPhase::BusFree);
	}
}

void PceScsiBus::ProcessDataInPhase()
{
	if(_signals[Req] && _signals[Ack]) {
		ClearSignals(Req);
	} else if(!_signals[Req] && !_signals[Ack]) {
		if(_dataBuffer.size() > 0) {
			_dataPort = _dataBuffer.front();
			_dataBuffer.pop_front();
			SetSignals(Req);
		} else {
			_cdrom->ClearIrqSource(PceCdRomIrqSource::DataTransferReady);
			if(_dataTransferDone) {
				_dataTransfer = false;
				_dataTransferDone = false;
				_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
			}
			SetStatusMessage(ScsiStatus::Good, 0);
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
		case ScsiCommand::TestUnitReady: SetStatusMessage(ScsiStatus::Good, 0); break;
		case ScsiCommand::Read: CmdRead(); break;
		case ScsiCommand::AudioStartPos: CmdAudioStartPos(); break;
		case ScsiCommand::AudioEndPos: CmdAudioEndPos(); break;
		case ScsiCommand::Pause: CmdPause(); break;
		case ScsiCommand::ReadSubCodeQ: CmdReadSubCodeQ(); break;
		case ScsiCommand::ReadToc: CmdReadToc(); break;
	}
}

void PceScsiBus::ProcessCommandPhase()
{
	if(_signals[Req] && _signals[Ack]) {
		ClearSignals(Req);
		_cmdBuffer.push_back(_dataPort);
	} else if(!_signals[Req] && !_signals[Ack] && _cmdBuffer.size() > 0) {
		ScsiCommand cmd = (ScsiCommand)_cmdBuffer[0];
		uint8_t cmdSize = GetCommandSize(cmd);
		if(cmdSize == 0) {
			//Unknown/unsupported command
			LogDebug("[SCSI] Unknown command: " + HexUtilities::ToHex(_cmdBuffer[0]));
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

void PceScsiBus::CmdRead()
{
	uint32_t sector = _cmdBuffer[3] | (_cmdBuffer[2] << 8) | ((_cmdBuffer[1] & 0x1F) << 16);
	uint8_t sectorsToRead = _cmdBuffer[4];

	_discReading = true;
	_dataTransfer = true;
	_sector = sector;
	_sectorsToRead = sectorsToRead;
	_readStartClock = _console->GetMasterClock();
	_cdrom->GetAudioPlayer().Stop();
	LogDebug("[SCSI] Read sector: " + std::to_string(_sector) + " to " + std::to_string(_sector + _sectorsToRead));
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
	LogDebug("[SCSI] CMD: Audio start pos");

	uint32_t startSector = GetAudioLbaPos();

	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	if(_cmdBuffer[1] == 0) {
		player.Play(startSector);
		player.Stop();
	} else {
		player.Play(startSector);
	}

	SetStatusMessage(ScsiStatus::Good, 0);
	_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferDone);
}

void PceScsiBus::CmdAudioEndPos()
{
	LogDebug("[SCSI] CMD: Audio end pos");

	uint32_t endSector = GetAudioLbaPos();
	
	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	switch(_cmdBuffer[1]) {
		case 0: player.Stop(); break;
		case 1: player.SetEndPosition(endSector, CdPlayEndBehavior::Loop); break;
		case 2: player.SetEndPosition(endSector, CdPlayEndBehavior::Irq); break;
		case 3: player.SetEndPosition(endSector, CdPlayEndBehavior::Stop); break;
	}
	
	SetStatusMessage(ScsiStatus::Good, 0);
}

void PceScsiBus::CmdPause()
{
	LogDebug("[SCSI] CMD: Audio pause");
	_cdrom->GetAudioPlayer().Stop();
	SetStatusMessage(ScsiStatus::Good, 0);
}

void PceScsiBus::CmdReadSubCodeQ()
{
	LogDebug("[SCSI] CMD: Read Sub Code Q");
	
	//TODO, improve
	_dataBuffer.clear();

	PceCdAudioPlayer& player = _cdrom->GetAudioPlayer();
	
	uint32_t sector = player.IsPlaying() ? player.GetCurrentSector() : _sector;
	uint32_t track = _disc->GetTrack(sector);

	uint32_t sectorGap = _disc->Tracks[track].FirstSector - sector;
	DiscPosition relPos = DiscPosition::FromLba(sectorGap);
	DiscPosition absPos = DiscPosition::FromLba(sector);

	_dataBuffer.push_back(_cdrom->GetAudioPlayer().IsPlaying() ? 0 : 3);

	_dataBuffer.push_back(0); //??
	_dataBuffer.push_back(CdReader::ToBcd(track + 1)); //track number
	_dataBuffer.push_back(1); //index number
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Minutes));
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Seconds));
	_dataBuffer.push_back(CdReader::ToBcd(relPos.Frames));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Minutes));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Seconds));
	_dataBuffer.push_back(CdReader::ToBcd(absPos.Frames));

	SetPhase(ScsiPhase::DataIn);
}

void PceScsiBus::CmdReadToc()
{
	LogDebug("[SCSI] CMD: Read ToC");
	switch(_cmdBuffer[1]) {
		case 0:
			//Number of tracks
			_dataBuffer.clear();
			_dataBuffer.push_back(1);
			_dataBuffer.push_back(CdReader::ToBcd((uint8_t)_disc->Tracks.size()));
			SetPhase(ScsiPhase::DataIn);
			break;

		case 1: {
			//Total disc length
			_dataBuffer.clear();
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Minutes));
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Seconds));
			_dataBuffer.push_back(CdReader::ToBcd(_disc->EndPosition.Frames));
			SetPhase(ScsiPhase::DataIn);
			break;
		}

		case 2: {
			uint8_t track = CdReader::FromBcd(_cmdBuffer[2]);
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

			SetPhase(ScsiPhase::DataIn);
			break;
		}

		default:
			LogDebug("[SCSI] CMD Read ToC: Unsupported parameters");
			break;
	}
}

void PceScsiBus::ProcessDiscRead()
{
	if(_discReading && _dataBuffer.empty() && _console->GetMasterClock() - _readStartClock > 175000) {
		//read disc data
		_dataBuffer.clear();
		_disc->ReadDataSector(_sector, _dataBuffer);

		_sector = (_sector + 1) & 0x1FFFFF;
		_sectorsToRead--;

		_cdrom->SetIrqSource(PceCdRomIrqSource::DataTransferReady);

		if(_sectorsToRead == 0) {
			_discReading = false;
			_dataTransferDone = true;
		}

		SetPhase(ScsiPhase::DataIn);
	}
}

uint8_t PceScsiBus::GetStatus()
{
	return (
		(_signals[Io] ? 0x08 : 0) |
		(_signals[Cd] ? 0x10 : 0) |
		(_signals[Msg] ? 0x20 : 0) |
		(_signals[Req] ? 0x40 : 0) |
		(_signals[Bsy] ? 0x80 : 0)
	);
}

void PceScsiBus::SetDataPort(uint8_t data)
{
	//LogDebug("[SCSI] CPU data port write: " + HexUtilities::ToHex(data));
	_dataPort = data;
}

uint8_t PceScsiBus::GetDataPort()
{
	//LogDebug("[SCSI] CPU data port read: " + HexUtilities::ToHex(_dataPort));
	return _dataPort;
}

bool PceScsiBus::CheckSignal(::ScsiSignal::ScsiSignal signal)
{
	return _signals[signal];
}

void PceScsiBus::SetSignalValue(::ScsiSignal::ScsiSignal signal, bool val)
{
	if(_signals[signal] != val) {
		_signals[signal] = val;
		_stateChanged = true;
	}
}

void PceScsiBus::Exec()
{
	ProcessDiscRead();

	if(!_stateChanged) {
		return;
	}

	if(_signals[Rst]) {
		Reset();
		return;
	}

	do {
		_stateChanged = false;

		if(!_signals[Bsy] && _signals[Sel]) {
			SetPhase(ScsiPhase::Command);
		} else if(!_signals[Ack] && _signals[Atn] && !_signals[Req]) {
			SetPhase(ScsiPhase::MessageOut);
		} else {
			switch(_phase) {
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
