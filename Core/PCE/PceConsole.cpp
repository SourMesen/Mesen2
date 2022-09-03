#include "stdafx.h"
#include "Shared/SettingTypes.h"
#include "PCE/PceConsole.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceNtscFilter.h"
#include "PCE/PceCpu.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceVpc.h"
#include "PCE/PcePsg.h"
#include "PCE/PceTimer.h"
#include "PCE/PceConstants.h"
#include "PCE/IPceMapper.h"
#include "PCE/PceArcadeCard.h"
#include "PCE/PceSf2RomMapper.h"
#include "Utilities/Serializer.h"
#include "Utilities/CRC32.h"
#include "MemoryType.h"
#include "FirmwareHelper.h"

PceConsole::PceConsole(Emulator* emu)
{
	_emu = emu;
}

void PceConsole::Reset()
{
	//The PC Engine has no reset button, behave like power cycle
	_emu->ReloadRom(true);
}

LoadRomResult PceConsole::LoadRom(VirtualFile& romFile)
{
	PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();
	PceConsoleType consoleType = cfg.ConsoleType;

	vector<uint8_t> romData;
	uint32_t crc32 = 0;

	bool cdromUnitEnabled = false;
	if(romFile.GetFileExtension() == ".cue") {
		if(!FirmwareHelper::LoadPceSuperCdFirmware(_emu, romData)) {
			return LoadRomResult::Failure;
		}

		DiscInfo disc = {};
		if(!CdReader::LoadCue(romFile, disc)) {
			return LoadRomResult::Failure;
		}

		_cdrom.reset(new PceCdRom(_emu, this, disc));
		_romFormat = RomFormat::PceCdRom;
		cdromUnitEnabled = true;
	} else {
		romFile.ReadFile(romData);
		crc32 = CRC32::GetCRC(romData);
		if((romData.size() % 0x2000) == 512) {
			//File probably has header, discard it
			romData.erase(romData.begin(), romData.begin() + 512);
		}

		if(consoleType == PceConsoleType::Auto && IsSuperGrafxCard(crc32)) {
			consoleType = PceConsoleType::SuperGrafx;
		}

		cdromUnitEnabled = cfg.EnableCdRomForHuCardGames;
		
		if(cdromUnitEnabled || !cfg.DisableCdRomSaveRamForHuCardGames) {
			//CD-ROM is used for save ram for non-cd-rom games
			DiscInfo emptyDisc = {};
			_cdrom.reset(new PceCdRom(_emu, this, emptyDisc));
		}

		_romFormat = RomFormat::Pce;
	}

	uint32_t cardRamSize = 0;
	if(_cdrom) {
		if(cfg.CdRomType == PceCdRomType::Arcade) {
			_mapper.reset(new PceArcadeCard(this, _emu));
		}
		if(cfg.CdRomType == PceCdRomType::SuperCdRom || cfg.CdRomType == PceCdRomType::Arcade) {
			cardRamSize = 0x30000;
		}
	} else if(romData.size() > 0x80 * 0x2000) {
		//For ROMs over 1MB, assume this is the SF2 board
		_mapper.reset(new PceSf2RomMapper(this));
	} else if(IsPopulousCard(crc32)) {
		cardRamSize = 0x8000;
	}

	_controlManager.reset(new PceControlManager(_emu));
	_vce.reset(new PceVce(_emu, this));
	_vpc.reset(new PceVpc(_emu, this, _vce.get()));
	
	_vdc.reset(new PceVdc(_emu, this, _vpc.get(), _vce.get(), false));
	if(consoleType == PceConsoleType::SuperGrafx) {
		_vdc2.reset(new PceVdc(_emu, this, _vpc.get(), _vce.get(), true));
	}

	_vpc->ConnectVdc(_vdc.get(), _vdc2.get());

	_timer.reset(new PceTimer(this));
	_psg.reset(new PcePsg(_emu, this));
	_memoryManager.reset(new PceMemoryManager(_emu, this, _vpc.get(), _vce.get(), _controlManager.get(), _psg.get(), _timer.get(), _mapper.get(), _cdrom.get(), romData, cardRamSize, cdromUnitEnabled));
	_cpu.reset(new PceCpu(_emu, _memoryManager.get()));

	MessageManager::Log("-----------------");
	MessageManager::Log("Loaded: " + romFile.GetFileName());
	MessageManager::Log("-----------------");

	return LoadRomResult::Success;
}

bool PceConsole::IsPopulousCard(uint32_t crc32)
{
	return crc32 == 0x083C956A;
}

bool PceConsole::IsSuperGrafxCard(uint32_t crc32)
{
	//These are the 5 SuperGrafx-exclusive games
	return crc32 == 0xB486A8ED || crc32 == 0x1F041166 || crc32 == 0x3B13AF61 || crc32 == 0x4C2126B0 || crc32 == 0x8C4588E2;
}

void PceConsole::RunFrame()
{
	uint32_t frameCount = _vdc->GetFrameCount();
	while(frameCount == _vdc->GetFrameCount()) {
		_cpu->Exec();
	}
}

void PceConsole::SaveBattery()
{
	if(_cdrom) {
		_cdrom->SaveBattery();
	}
}

BaseControlManager* PceConsole::GetControlManager()
{
	return _controlManager.get();
}

ConsoleRegion PceConsole::GetRegion()
{
	return ConsoleRegion::Ntsc;
}

ConsoleType PceConsole::GetConsoleType()
{
	return ConsoleType::PcEngine;
}

vector<CpuType> PceConsole::GetCpuTypes()
{
	return { CpuType::Pce };
}

PceCpu* PceConsole::GetCpu()
{
	return _cpu.get();
}

PceVdc* PceConsole::GetVdc()
{
	return _vdc.get();
}

PceVce* PceConsole::GetVce()
{
	return _vce.get();
}

PceVpc* PceConsole::GetVpc()
{
	return _vpc.get();
}

PcePsg* PceConsole::GetPsg()
{
	return _psg.get();
}

PceMemoryManager* PceConsole::GetMemoryManager()
{
	return _memoryManager.get();
}

uint64_t PceConsole::GetMasterClock()
{
	return _memoryManager->GetState().CycleCount;
}

uint32_t PceConsole::GetMasterClockRate()
{
	return PceConstants::MasterClockRate;
}

double PceConsole::GetFps()
{
	//Varies depending on VCE setting (263 or 262 scanlines)
	return (double)PceConstants::MasterClockRate / PceConstants::ClockPerScanline / _vce->GetState().ScanlineCount;
}

BaseVideoFilter* PceConsole::GetVideoFilter()
{
	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

	switch(filterType) {
		case VideoFilterType::NtscBlargg:
		case VideoFilterType::NtscBisqwit:
			return new PceNtscFilter(_emu);

		default:
			return new PceDefaultVideoFilter(_emu);
	}
}

PpuFrameInfo PceConsole::GetPpuFrame()
{
	PpuFrameInfo frame = {};
	PceVdcState& state = _vdc->GetState();
	frame.FrameCount = state.FrameCount;
	frame.CycleCount = PceConstants::ClockPerScanline;

	frame.FirstScanline = 0;
	frame.ScanlineCount = PceConstants::ScanlineCount;

	frame.FrameBuffer = (uint8_t*)_vpc->GetScreenBuffer();
	//TODO - height/width/scanlinecount vary based on VDC settings
	frame.Height = PceConstants::InternalOutputHeight;
	frame.Width = PceConstants::InternalOutputWidth;
	frame.FrameBufferSize = PceConstants::MaxScreenWidth * (PceConstants::ScreenHeight + 1) * sizeof(uint16_t);
	return frame;
}

RomFormat PceConsole::GetRomFormat()
{
	return _romFormat;
}

AudioTrackInfo PceConsole::GetAudioTrackInfo()
{
	//TODO HES file support
	return {};
}

void PceConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
}

AddressInfo PceConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	if(relAddress.Type == MemoryType::PceMemory) {
		return _memoryManager->GetAbsoluteAddress(relAddress.Address);
	}
	return { -1, MemoryType::None };
}

AddressInfo PceConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	return _memoryManager->GetRelativeAddress(absAddress);
}

PceVideoState PceConsole::GetVideoState()
{
	PceVideoState state;
	state.Vdc = _vdc->GetState();
	state.Vce = _vce->GetState();
	state.Vpc = _vpc->GetState();

	if(_vdc2) {
		state.Vdc2 = _vdc2->GetState();
	} else {
		state.Vdc2 = {};
	}
	return state;
}

void PceConsole::GetConsoleState(BaseState& baseState, ConsoleType consoleType)
{
	PceState& state = (PceState&)baseState;
	state.Cpu = _cpu->GetState();
	state.Video = GetVideoState();
	state.MemoryManager = _memoryManager->GetState();
	state.Timer = _timer->GetState();
	state.Psg = _psg->GetState();
	for(int i = 0; i < 6; i++) {
		state.PsgChannels[i] = _psg->GetChannelState(i);
	}

	state.HasArcadeCard = false;
	if(_mapper && dynamic_cast<PceArcadeCard*>(_mapper.get())) {
		state.ArcadeCard = ((PceArcadeCard*)_mapper.get())->GetState();
		state.HasArcadeCard = true;
	}

	if(_cdrom) {
		state.CdRom = _cdrom->GetState();
		state.Adpcm = _cdrom->GetAdpcmState();
		state.AudioFader = _cdrom->GetAudioFaderState();
		state.CdPlayer = _cdrom->GetCdPlayerState();
		state.ScsiDrive = _cdrom->GetScsiState();
	}

	state.IsSuperGrafx = _vdc2 != nullptr;
	state.HasCdRom = _cdrom != nullptr;
}

void PceConsole::Serialize(Serializer& s)
{
	SV(_cpu);
	SV(_vdc);
	if(_vdc2) {
		SV(_vdc2);
	}
	SV(_vpc);
	SV(_vce);
	SV(_psg);
	SV(_memoryManager);
	SV(_controlManager);
	SV(_timer);

	if(_cdrom) {
		SV(_cdrom);
	}

	if(_mapper) {
		SV(_mapper);
	}
}

void PceConsole::InitializeRam(void* data, uint32_t length)
{
	EmuSettings* settings = _emu->GetSettings();
	settings->InitializeRam(settings->GetPcEngineConfig().RamPowerOnState, data, length);
}