#include "pch.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/Spc.h"
#include "SNES/InternalRegisters.h"
#include "SNES/SnesControlManager.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesDmaController.h"
#include "SNES/BaseCartridge.h"
#include "SNES/RamHandler.h"
#include "SNES/CartTypes.h"
#include "SNES/SpcFileData.h"
#include "SNES/SnesDefaultVideoFilter.h"
#include "SNES/SnesNtscFilter.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbPpu.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "SNES/SnesState.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Coprocessors/MSU1/Msu1.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "Shared/Emulator.h"
#include "Shared/TimingInfo.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Shared/EventType.h"
#include "SNES/RegisterHandlerA.h"
#include "SNES/RegisterHandlerB.h"
#include "Utilities/ArchiveReader.h"

SnesConsole::SnesConsole(Emulator* emu)
{
	_emu = emu;
	_settings = emu->GetSettings();
}

SnesConsole::~SnesConsole()
{
}

void SnesConsole::Initialize()
{
}

void SnesConsole::Release()
{
}

void SnesConsole::RunFrame()
{
	UpdateRegion();

	_frameRunning = true;

	while(_frameRunning) {
		_cpu->Exec();
	}

	_spc->ProcessEndFrame();
}

void SnesConsole::ProcessEndOfFrame()
{
	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}
	
	//Run the SPC at least once per frame to prevent issues (buffer overflow)
	//when a very long DMA transfer is running across multiple frames.
	//(RunFrame above can run more than one frame in this scenario, which could cause crashes)
	_spc->ProcessEndFrame();

	_emu->ProcessEndOfFrame();

	_controlManager->UpdateControlDevices();
	_controlManager->UpdateInputState();
	_internalRegisters->SetAutoJoypadReadClock();
	_frameRunning = false;
}

void SnesConsole::Reset()
{
	_dmaController->Reset();
	_internalRegisters->Reset();
	_memoryManager->Reset();
	_spc->Reset();
	_ppu->Reset();
	_cart->Reset();
	_controlManager->Reset(true);

	//Reset cart before CPU to ensure correct memory mappings when fetching reset vector
	_cpu->Reset();

	//After reset, run the PPU/etc ahead of the CPU (simulates delay CPU takes to get out of reset)
	_memoryManager->IncMasterClockStartup();
}

LoadRomResult SnesConsole::LoadRom(VirtualFile& romFile)
{
	SnesConfig config = _settings->GetSnesConfig();
	LoadRomResult loadResult = LoadRomResult::UnknownType;

	unique_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(this, romFile);
	if(cart) {
		_cart.swap(cart);
		
		UpdateRegion();

		_internalRegisters.reset(new InternalRegisters());
		_memoryManager.reset(new SnesMemoryManager());
		_ppu.reset(new SnesPpu(_emu, this));
		_controlManager.reset(new SnesControlManager(this));
		_dmaController.reset(new SnesDmaController(_memoryManager.get()));
		_spc.reset(new Spc(this));

		_msu1.reset(Msu1::Init(_emu, romFile, _spc.get()));

		if(_cart->GetSpcData()) {
			if(!LoadSpcFile(romFile)) {
				return LoadRomResult::Failure;
			}
		}

		_cpu.reset(new SnesCpu(this));
		_memoryManager->Initialize(this);
		_internalRegisters->Initialize(this);

		_ppu->PowerOn();
		_cpu->PowerOn();
		
		//After power on, run the PPU/etc ahead of the CPU (simulates delay CPU takes to get out of reset)
		_memoryManager->IncMasterClockStartup();

		loadResult = LoadRomResult::Success;
	}

	//Loading a cartridge can alter the SNES ram init settings - restore their original values here
	_settings->SetSnesConfig(config);
	return loadResult;
}

bool SnesConsole::LoadSpcFile(VirtualFile& romFile)
{
	_spc->LoadSpcFile(_cart->GetSpcData());
	if(romFile.IsArchive()) {
		string archivePath = romFile.GetFilePath();
		unique_ptr<ArchiveReader> reader = ArchiveReader::GetReader(archivePath);
		if(reader) {
			for(string& spcFile : reader->GetFileList({ ".spc" })) {
				_spcPlaylist.push_back((string)VirtualFile(archivePath, spcFile));
			}
		} else {
			return false;
		}
	} else {
		_spcPlaylist = FolderUtilities::GetFilesInFolder(romFile.GetFolderPath(), { ".spc" }, false);
	}

	std::sort(_spcPlaylist.begin(), _spcPlaylist.end());
	auto result = std::find(_spcPlaylist.begin(), _spcPlaylist.end(), (string)romFile);
	if(result == _spcPlaylist.end()) {
		_spcPlaylist.push_back((string)romFile);
	}
	_spcTrackNumber = (uint32_t)std::distance(_spcPlaylist.begin(), result);
	return true;
}

uint64_t SnesConsole::GetMasterClock()
{
	return _memoryManager->GetMasterClock();
}

uint32_t SnesConsole::GetMasterClockRate()
{
	return _masterClockRate;
}

ConsoleRegion SnesConsole::GetRegion()
{
	return _region;
}

ConsoleType SnesConsole::GetConsoleType()
{
	return ConsoleType::Snes;
}

void SnesConsole::UpdateRegion()
{
	switch(_settings->GetSnesConfig().Region) {
		case ConsoleRegion::Auto: _region = _cart->GetRegion(); break;

		default:
		case ConsoleRegion::Ntsc: _region = ConsoleRegion::Ntsc; break;
		case ConsoleRegion::Pal: _region = ConsoleRegion::Pal; break;
	}

	_masterClockRate = _region == ConsoleRegion::Pal ? 21281370 : 21477270;
}

double SnesConsole::GetFps()
{
	if(_region == ConsoleRegion::Ntsc) {
		return 60.0988118623484;
	} else {
		return 50.0069789081886;
	}
}

PpuFrameInfo SnesConsole::GetPpuFrame()
{
	PpuFrameInfo frame = {};
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer();
	frame.Width = _ppu->IsHighResOutput() ? 512 : 256;
	frame.Height = _ppu->IsHighResOutput() ? 478 : 239;
	frame.FrameBufferSize = frame.Width * frame.Height * sizeof(uint16_t);
	frame.FrameCount = _ppu->GetFrameCount();
	frame.FirstScanline = 0;
	frame.ScanlineCount = _ppu->GetVblankEndScanline() + 1;
	frame.CycleCount = 341;
	return frame;
}

TimingInfo SnesConsole::GetTimingInfo(CpuType cpuType)
{
	if(cpuType == CpuType::Gameboy && _cart->GetGameboy()) {
		return _cart->GetGameboy()->GetTimingInfo(cpuType);
	}
	return IConsole::GetTimingInfo(cpuType);
}

vector<CpuType> SnesConsole::GetCpuTypes()
{
	vector<CpuType> cpuTypes = { CpuType::Snes, CpuType::Spc };
	if(_cart->GetGsu()) {
		cpuTypes.push_back(CpuType::Gsu);
	} else if(_cart->GetDsp()) {
		cpuTypes.push_back(CpuType::NecDsp);
	} else if(_cart->GetCx4()) {
		cpuTypes.push_back(CpuType::Cx4);
	} else if(_cart->GetGameboy()) {
		cpuTypes.push_back(CpuType::Gameboy);
	} else if(_cart->GetSa1()) {
		cpuTypes.push_back(CpuType::Sa1);
	} else if(_cart->GetSt018()) {
		cpuTypes.push_back(CpuType::St018);
	}
	return cpuTypes;
}

void SnesConsole::SaveBattery()
{
	if(_cart) {
		_cart->SaveBattery();
	}
}

BaseVideoFilter* SnesConsole::GetVideoFilter(bool getDefaultFilter)
{
	if(getDefaultFilter || GetRomFormat() == RomFormat::Spc) {
		return new SnesDefaultVideoFilter(_emu);
	}

	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;
	
	switch(filterType) {
		case VideoFilterType::NtscBlargg:
		case VideoFilterType::NtscBisqwit:
			return new SnesNtscFilter(_emu);

		default:
			return new SnesDefaultVideoFilter(_emu);
	}
}

RomFormat SnesConsole::GetRomFormat()
{
	if(_cart->GetGameboy()) {
		return RomFormat::Gb;
	} else if(_cart->GetSpcData()) {
		return RomFormat::Spc;
	}
	return RomFormat::Sfc;
}

AudioTrackInfo SnesConsole::GetAudioTrackInfo()
{
	AudioTrackInfo track = {};
	SpcFileData* spc = _cart->GetSpcData();
	if(spc) {
		track.Artist = spc->Artist;
		track.Comment = spc->Comment;
		track.GameTitle = spc->GameTitle;
		track.SongTitle = spc->SongTitle;
		track.Position = _ppu->GetFrameCount() / GetFps();
		track.Length = spc->TrackLength + (spc->FadeLength / 1000.0);
		track.FadeLength = spc->FadeLength / 1000.0;
		track.TrackNumber = _spcTrackNumber + 1;
		track.TrackCount = (uint32_t)_spcPlaylist.size();

	}
	return track;
}

void SnesConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	if(_spcTrackNumber >= 0) {
		int i = (int)_spcTrackNumber;
		switch(p.Action) {
			case AudioPlayerAction::NextTrack: i++; break;
			case AudioPlayerAction::PrevTrack:
				if(GetAudioTrackInfo().Position < 2) {
					i--;
				}
				break;

			case AudioPlayerAction::SelectTrack: i = (int)p.TrackNumber; break;
		}

		if(i < 0) {
			i = (int)_spcPlaylist.size() - 1;
		} else if(i >= _spcPlaylist.size()) {
			i = 0;
		}

		//Asynchronously move to the next file
		//Can't do this in the current thread in some contexts (e.g when track reaches end)
		//because this is called from the emulation thread.
		thread switchTrackTask([this, i]() {
			_emu->LoadRom(VirtualFile(_spcPlaylist[i]), VirtualFile());
		});
		switchTrackTask.detach();
	}
}

void SnesConsole::Serialize(Serializer& s)
{
	SV(_cpu);
	SV(_memoryManager);
	SV(_ppu);
	SV(_dmaController);
	SV(_internalRegisters);
	SV(_cart);
	SV(_spc);
	if(_msu1) {
		SV(_msu1);
	}
	SV(_controlManager);
}

SaveStateCompatInfo SnesConsole::ValidateSaveStateCompatibility(ConsoleType stateConsoleType)
{
	if(stateConsoleType == ConsoleType::Gameboy) {
		return { true, "cart.gameboy.", "", {
			//Keep all clock counters as-is when loading a GB/GBC state
			//in a SGB core - this allows it to keep running in sync with
			//the SNES core properly.
			"apu.clockCounter", "apu.prevClockCount",
			"cpu.cycleCount",
			"memoryManager.apuCycleCount"
		} };
	}

	return {};
}

SnesCpu* SnesConsole::GetCpu()
{
	return _cpu.get();
}

SnesPpu* SnesConsole::GetPpu()
{
	return _ppu.get();
}

Spc* SnesConsole::GetSpc()
{
	return _spc.get();
}

BaseCartridge* SnesConsole::GetCartridge()
{
	return _cart.get();
}

SnesMemoryManager* SnesConsole::GetMemoryManager()
{
	return _memoryManager.get();
}

InternalRegisters* SnesConsole::GetInternalRegisters()
{
	return _internalRegisters.get();
}

BaseControlManager* SnesConsole::GetControlManager()
{
	return _controlManager.get();
}

SnesDmaController* SnesConsole::GetDmaController()
{
	return _dmaController.get();
}

Msu1* SnesConsole::GetMsu1()
{
	return _msu1.get();
}

Emulator* SnesConsole::GetEmulator()
{
	return _emu;
}

bool SnesConsole::IsRunning()
{
	return _cpu != nullptr;
}

AddressInfo SnesConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	static AddressInfo unmapped = { -1, MemoryType::None };

	switch(relAddress.Type) {
		case MemoryType::SnesMemory:
			if(_memoryManager->IsRegister(relAddress.Address)) {
				return { relAddress.Address & 0xFFFF, MemoryType::SnesRegister };
			} else {
				return _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
			}
		
		case MemoryType::SpcMemory: return _spc->GetAbsoluteAddress(relAddress.Address);
		case MemoryType::Sa1Memory: return _cart->GetSa1() ? _cart->GetSa1()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address) : unmapped;
		case MemoryType::GsuMemory: return _cart->GetGsu() ? _cart->GetGsu()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address) : unmapped;
		case MemoryType::Cx4Memory: return _cart->GetCx4() ? _cart->GetCx4()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address) : unmapped;
		case MemoryType::St018Memory: return _cart->GetSt018() ? _cart->GetSt018()->GetArmAbsoluteAddress(relAddress.Address) : unmapped;
		case MemoryType::NecDspMemory: return { relAddress.Address, MemoryType::DspProgramRom };
		case MemoryType::GameboyMemory: return _cart->GetGameboy() ? _cart->GetGameboy()->GetAbsoluteAddress(relAddress.Address) : unmapped;
		default: return unmapped;
	}
}

AddressInfo SnesConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	static AddressInfo unmapped = { -1, MemoryType::None };

	MemoryMappings* mappings = nullptr;
	switch(cpuType) {
		case CpuType::Snes: mappings = _memoryManager->GetMemoryMappings(); break;
		case CpuType::Spc: break;
		case CpuType::NecDsp: break;
		case CpuType::Sa1: mappings = _cart->GetSa1() ? _cart->GetSa1()->GetMemoryMappings() : nullptr; break;
		case CpuType::Gsu: mappings = _cart->GetGsu() ? _cart->GetGsu()->GetMemoryMappings() : nullptr; break;
		case CpuType::Cx4: mappings = _cart->GetCx4() ? _cart->GetCx4()->GetMemoryMappings() : nullptr; break;
		case CpuType::St018: break;
		case CpuType::Gameboy: break;
		default: return unmapped;
	}

	switch(absAddress.Type) {
		case MemoryType::SnesPrgRom:
		case MemoryType::SnesWorkRam:
		case MemoryType::SnesSaveRam:
		case MemoryType::SufamiTurboFirmware:
		{
			if(!mappings) {
				return unmapped;
			}

			uint8_t startBank = 0;
			//Try to find a mirror close to where the PC is
			if(cpuType == CpuType::Snes) {
				if(absAddress.Type == MemoryType::SnesWorkRam) {
					startBank = 0x7E;
				} else {
					startBank = _cpu->GetState().K & 0xC0;
				}
			} else if(cpuType == CpuType::Sa1) {
				startBank = (_cart->GetSa1()->GetCpuState().K & 0xC0);
			} else if(cpuType == CpuType::Gsu) {
				startBank = (_cart->GetGsu()->GetState().ProgramBank & 0xC0);
			}

			return { mappings->GetRelativeAddress(absAddress, startBank), DebugUtilities::GetCpuMemoryType(cpuType) };
		}

		case MemoryType::SpcRam:
		case MemoryType::SpcRom:
			return { _spc->GetRelativeAddress(absAddress), MemoryType::SpcMemory };

		case MemoryType::GbPrgRom:
		case MemoryType::GbWorkRam:
		case MemoryType::GbCartRam:
		case MemoryType::GbHighRam:
		case MemoryType::GbBootRom:
			return _cart->GetGameboy() ? AddressInfo { _cart->GetGameboy()->GetRelativeAddress(absAddress), MemoryType::GameboyMemory } : unmapped;

		case MemoryType::St018PrgRom:
		case MemoryType::St018DataRom:
		case MemoryType::St018WorkRam:
			return _cart->GetSt018() ? AddressInfo { _cart->GetSt018()->GetArmRelativeAddress(absAddress), MemoryType::St018Memory } : unmapped;

		case MemoryType::DspProgramRom:
			return { absAddress.Address, MemoryType::NecDspMemory };

		case MemoryType::SnesRegister:
			return { absAddress.Address & 0xFFFF, MemoryType::SnesMemory };

		default:
			return unmapped;
	}
}

void SnesConsole::GetConsoleState(BaseState& baseState, ConsoleType consoleType)
{
	if(consoleType == ConsoleType::Gameboy) {
		_cart->GetGameboy()->GetConsoleState(baseState, consoleType);
		return;
	}

	SnesState& state = (SnesState&)baseState;
	state.MasterClock = GetMasterClock();
	state.WramPosition = _memoryManager->GetWramPosition();
	state.Cpu = _cpu->GetState();
	_ppu->GetState(state.Ppu, false);
	state.Spc = _spc->GetState();
	state.Dsp = _spc->GetDspState();

	state.Dma = _dmaController->GetState();
	state.InternalRegs = _internalRegisters->GetState();
	state.Alu = _internalRegisters->GetAluState();

	if(_cart->GetDsp()) {
		state.NecDsp = _cart->GetDsp()->GetState();
	}
	if(_cart->GetSa1()) {
		state.Sa1 = _cart->GetSa1()->GetState();
	}
	if(_cart->GetGsu()) {
		state.Gsu = _cart->GetGsu()->GetState();
	}
	if(_cart->GetCx4()) {
		state.Cx4 = _cart->GetCx4()->GetState();
	}
	if(_cart->GetSt018()) {
		state.St018 = _cart->GetSt018()->GetState();
	}
}

void SnesConsole::InitializeRam(void* data, uint32_t length)
{
	EmuSettings* settings = _emu->GetSettings();
	RamState state;
	if(_cart) {
		state = _cart->GetRamPowerOnState();
	} else {
		state = settings->GetSnesConfig().RamPowerOnState;
	}
	settings->InitializeRam(state, data, length);
}