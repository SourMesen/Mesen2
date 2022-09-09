#include "pch.h"
#include "PCE/CdRom/PceCdRom.h"
#include "PCE/PceConsole.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/CdRom/PceCdAudioPlayer.h"
#include "PCE/PceTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/BatteryManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"

using namespace ScsiSignal;

PceCdRom::PceCdRom(Emulator* emu, PceConsole* console, DiscInfo& disc) : _disc(disc), _scsi(console, this, _disc), _adpcm(console, emu, this, &_scsi), _audioFader(console), _audioPlayer(emu, this, _disc)
{
	_emu = emu;
	_console = console;

	_emu->GetSoundMixer()->RegisterAudioProvider(&_audioPlayer);
	_emu->GetSoundMixer()->RegisterAudioProvider(&_adpcm);

	//Initialize save ram
	_saveRamSize = 0x2000;
	_saveRam = new uint8_t[_saveRamSize];
	_console->InitializeRam(_saveRam, _saveRamSize);
	_emu->RegisterMemory(MemoryType::PceSaveRam, _saveRam, _saveRamSize);

	_saveRam[0] = 0x48;
	_saveRam[1] = 0x55;
	_saveRam[2] = 0x42;
	_saveRam[3] = 0x4D;
	_saveRam[4] = 0x00;
	_saveRam[5] = 0xA0;
	_saveRam[6] = 0x10;
	_saveRam[7] = 0x80;

	_emu->GetBatteryManager()->LoadBattery(".sav", _saveRam, _saveRamSize);

	//Initialize cdrom work ram
	_cdromRamSize = 0x10000;
	_cdromRam = new uint8_t[_cdromRamSize];
	_console->InitializeRam(_cdromRam, _cdromRamSize);
	_emu->RegisterMemory(MemoryType::PceCdromRam, _cdromRam, _cdromRamSize);
}

PceCdRom::~PceCdRom()
{
	_emu->GetSoundMixer()->UnregisterAudioProvider(&_audioPlayer);
	_emu->GetSoundMixer()->UnregisterAudioProvider(&_adpcm);

	delete[] _saveRam;
	delete[] _cdromRam;
}

void PceCdRom::SaveBattery()
{
	if(_saveRamSize > 0) {
		_emu->GetBatteryManager()->SaveBattery(".sav", _saveRam, _saveRamSize);
	}
}

void PceCdRom::InitMemoryBanks(uint8_t* readBanks[0x100], uint8_t* writeBanks[0x100], MemoryType bankMemType[0x100], uint8_t* unmappedBank)
{
	for(int i = 0; i < 8; i++) {
		//80 - 87
		readBanks[0x80 + i] = _cdromRam + ((i * 0x2000) % _cdromRamSize);
		writeBanks[0x80 + i] = _cdromRam + ((i * 0x2000) % _cdromRamSize);
		bankMemType[0x80 + i] = MemoryType::PceCdromRam;
	}

	//F7 - BRAM
	if(_state.BramLocked) {
		readBanks[0xF7] = unmappedBank;
		writeBanks[0xF7] = unmappedBank;
		bankMemType[0xF7] = MemoryType::None;
	} else {
		readBanks[0xF7] = _saveRam;
		writeBanks[0xF7] = _saveRam;
		bankMemType[0xF7] = MemoryType::PceSaveRam;
	}
}

void PceCdRom::SetIrqSource(PceCdRomIrqSource src)
{
	//LogDebug("Set IRQ source: " + HexUtilities::ToHex((uint8_t)src));
	if((_state.ActiveIrqs & (uint8_t)src) == 0) {
		_state.ActiveIrqs |= (uint8_t)src;
		UpdateIrqState();
	}
}

void PceCdRom::ClearIrqSource(PceCdRomIrqSource src)
{
	//LogDebug("Clear IRQ source: " + HexUtilities::ToHex((uint8_t)src));
	if(_state.ActiveIrqs & (uint8_t)src) {
		_state.ActiveIrqs &= ~(uint8_t)src;
		UpdateIrqState();
	}
}

void PceCdRom::UpdateIrqState()
{
	if((_state.EnabledIrqs & _state.ActiveIrqs) != 0) {
		_console->GetMemoryManager()->SetIrqSource(PceIrqSource::Irq2);
	} else {
		_console->GetMemoryManager()->ClearIrqSource(PceIrqSource::Irq2);
	}
}

void PceCdRom::Write(uint16_t addr, uint8_t value)
{
	switch(addr & 0x3FF) {
		case 0x00: //SCSI Control
			_scsi.SetSignalValue(Sel, true);
			_scsi.UpdateState();
			_scsi.SetSignalValue(Sel, false);
			_scsi.UpdateState();
			break;

		case 0x01: //CDC/SCSI Command
			_scsi.SetDataPort(value);
			_scsi.UpdateState();
			break;

		case 0x02: //ACK
			_scsi.SetSignalValue(Ack, (value & 0x80) != 0);
			_scsi.UpdateState();

			_state.EnabledIrqs = value & 0x7C;
			UpdateIrqState();
			break;

		case 0x03: break; //Readonly

		case 0x04: {
			//Reset
			bool reset = (value & 0x02) != 0;
			_scsi.SetSignalValue(Rst, reset);
			_scsi.UpdateState();
			if(reset) {
				//Clear enabled IRQs flags for SCSI drive (SubChannel + DataTransferDone + DataTransferReady)
				_state.EnabledIrqs &= 0x8F;
				UpdateIrqState();
			}
			break;
		}

		case 0x05:
		case 0x06:
			//Readonly
			break;

		case 0x07:
			//BRAM unlock
			if((value & 0x80) != 0) {
				_state.BramLocked = false;
				_console->GetMemoryManager()->UpdateCdRomBanks();
			}
			break;

		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E:
			_adpcm.Write(addr, value);
			break;

		case 0x0F:
			_audioFader.Write(value);
			break;
	}
}

uint8_t PceCdRom::Read(uint16_t addr)
{
	switch(addr & 0x3FF) {
		case 0x00: return _scsi.GetStatus();
		case 0x01: return _scsi.GetDataPort();
		case 0x02: return _state.EnabledIrqs | (_scsi.CheckSignal(Ack) ? 0x80 : 0);
		case 0x03:
			_state.BramLocked = true;
			_console->GetMemoryManager()->UpdateCdRomBanks();
			_state.ReadRightChannel = !_state.ReadRightChannel;

			return (
				_state.ActiveIrqs |
				(_state.ReadRightChannel ? 0 : 0x02)
			);

		case 0x04: return _scsi.CheckSignal(Rst) ? 0x02 : 0;

		case 0x05: return (uint8_t)(_state.ReadRightChannel ? _audioPlayer.GetRightSample() : _audioPlayer.GetLeftSample());
		case 0x06: return (uint8_t)((_state.ReadRightChannel ? _audioPlayer.GetRightSample() : _audioPlayer.GetLeftSample()) >> 8);
			
		case 0x07: return _state.BramLocked ? 0 : 0x80;

		case 0x08: {
			uint8_t val = _scsi.GetDataPort();
			if(_scsi.CheckSignal(Req) && _scsi.CheckSignal(Io) && !_scsi.CheckSignal(Cd)) {
				_scsi.SetAckWithAutoClear();
				_scsi.UpdateState();
			}
			return val;
		}

		case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E:
			return _adpcm.Read(addr);

		case 0xC0: case 0xC1: case 0xC2: case 0xC3: 
			if(_emu->GetSettings()->GetPcEngineConfig().CdRomType == PceCdRomType::CdRom) {
				return 0xFF;
			} else {
				constexpr uint8_t superCdRomSignature[4] = { 0x00, 0xAA, 0x55, 0x03 };
				return superCdRomSignature[addr & 0x03];
			}

		default:
			LogDebug("Read unknown CDROM register: " + HexUtilities::ToHex(addr));
			break;
	}

	return 0xFF;
}

void PceCdRom::Serialize(Serializer& s)
{
	SV(_state.ActiveIrqs);
	SV(_state.BramLocked);
	SV(_state.EnabledIrqs);
	SV(_state.ReadRightChannel);

	SVArray(_saveRam, _saveRamSize);
	SVArray(_cdromRam, _cdromRamSize);

	SV(_scsi);
	SV(_adpcm);
	SV(_audioPlayer);
	SV(_audioFader);
}
