#include "stdafx.h"
#include "SNES/Console.h"
#include "SNES/Cpu.h"
#include "SNES/Ppu.h"
#include "SNES/Spc.h"
#include "SNES/InternalRegisters.h"
#include "SNES/ControlManager.h"
#include "SNES/MemoryManager.h"
#include "SNES/DmaController.h"
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
#include "SNES/Coprocessors/MSU1/Msu1.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IControlManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/Timer.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/PlatformUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "EventType.h"

Console::Console(Emulator* emu)
{
	_emu = emu;
	_settings = emu->GetSettings();
}

Console::~Console()
{
}

void Console::Initialize()
{
}

void Console::Release()
{
}

void Console::RunFrame()
{
	_frameRunning = true;

	while(_frameRunning) {
		_cpu->Exec();
	}
}

void Console::OnBeforeRun()
{
	_memoryManager->IncMasterClockStartup();

	//TODO?
	//_controlManager->UpdateInputState();
}

void Console::ProcessEndOfFrame()
{
#ifndef LIBRETRO
	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_emu->ProcessEndOfFrame();

	_controlManager->UpdateControlDevices();
	_controlManager->UpdateInputState();
	_internalRegisters->ProcessAutoJoypadRead();
#endif
	_frameRunning = false;
}

void Console::RunSingleFrame()
{
	//Used by Libretro
	/*_emulationThreadId = std::this_thread::get_id();
	_isRunAheadFrame = false;

	_controlManager->UpdateInputState();
	_internalRegisters->ProcessAutoJoypadRead();

	RunFrame();

	_cart->RunCoprocessors();
	if(_cart->GetCoprocessor()) {
		_cart->GetCoprocessor()->ProcessEndOfFrame();
	}

	_controlManager->UpdateControlDevices();*/
}

void Console::Stop()
{
}

void Console::Reset()
{
	_dmaController->Reset();
	_internalRegisters->Reset();
	_memoryManager->Reset();
	_spc->Reset();
	_ppu->Reset();
	_cart->Reset();
	//_controlManager->Reset();

	//Reset cart before CPU to ensure correct memory mappings when fetching reset vector
	_cpu->Reset();
}

bool Console::LoadRom(VirtualFile& romFile)
{
	bool result = false;
	EmulationConfig orgConfig = _settings->GetEmulationConfig(); //backup emulation config (can be temporarily overriden to control the power on RAM state)
	shared_ptr<BaseCartridge> cart = BaseCartridge::CreateCartridge(this, romFile);
	if(cart) {
		_cart = cart;
		
		UpdateRegion();

		_internalRegisters.reset(new InternalRegisters());
		_memoryManager.reset(new MemoryManager());
		_ppu.reset(new Ppu(_emu, this));
		_controlManager.reset(new ControlManager(this));
		_dmaController.reset(new DmaController(_memoryManager.get()));
		_spc.reset(new Spc(this));

		_msu1.reset(Msu1::Init(romFile, _spc.get()));

		if(_cart->GetSpcData()) {
			//TODO
    		_spc->LoadSpcFile(_cart->GetSpcData());
			_spcPlaylist = FolderUtilities::GetFilesInFolder(romFile.GetFolderPath(), { ".spc" }, false);
			std::sort(_spcPlaylist.begin(), _spcPlaylist.end());
			auto result = std::find(_spcPlaylist.begin(), _spcPlaylist.end(), romFile.GetFilePath());
			_spcTrackNumber = (uint32_t)std::distance(_spcPlaylist.begin(), result);
		}

		_cpu.reset(new Cpu(this));
		_memoryManager->Initialize(this);
		_internalRegisters->Initialize(this);

		_ppu->PowerOn();
		_cpu->PowerOn();

		_controlManager->UpdateControlDevices();
				
		UpdateRegion();

		result = true;
	}

	_settings->SetEmulationConfig(orgConfig);
	return result;
}

void Console::Init()
{
}

uint64_t Console::GetMasterClock()
{
	return _memoryManager->GetMasterClock();
}

uint32_t Console::GetMasterClockRate()
{
	return _masterClockRate;
}

ConsoleRegion Console::GetRegion()
{
	return _region;
}

ConsoleType Console::GetConsoleType()
{
	return ConsoleType::Snes;
}

void Console::UpdateRegion()
{
	switch(_settings->GetSnesConfig().Region) {
		case ConsoleRegion::Auto: _region = _cart->GetRegion(); break;

		default:
		case ConsoleRegion::Ntsc: _region = ConsoleRegion::Ntsc; break;
		case ConsoleRegion::Pal: _region = ConsoleRegion::Pal; break;
	}

	_masterClockRate = _region == ConsoleRegion::Pal ? 21281370 : 21477270;
}

double Console::GetFps()
{
	if(_region == ConsoleRegion::Ntsc) {
		return _settings->GetVideoConfig().IntegerFpsMode ? 60.0 : 60.0988118623484;
	} else {
		return _settings->GetVideoConfig().IntegerFpsMode ? 50.0 : 50.00697796826829;
	}
}

PpuFrameInfo Console::GetPpuFrame()
{
	//TODO null checks
	PpuFrameInfo frame = {};
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer();
	frame.Width = 256;
	frame.Height = 239;
	frame.FrameCount = _ppu->GetFrameCount();
	return frame;
}

vector<CpuType> Console::GetCpuTypes()
{
	return { CpuType::Cpu, CpuType::Spc };
}

void Console::SaveBattery()
{
	if(_cart) {
		_cart->SaveBattery();
	}
}

BaseVideoFilter* Console::GetVideoFilter()
{
	VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;
	if(filterType == VideoFilterType::NTSC) {
		return new SnesNtscFilter(_emu);
	} else {
		return new SnesDefaultVideoFilter(_emu);
	}
}

RomFormat Console::GetRomFormat()
{
	return _cart->GetSpcData() ? RomFormat::Spc : RomFormat::Sfc;
}

AudioTrackInfo Console::GetAudioTrackInfo()
{
	AudioTrackInfo track = {};
	SpcFileData* spc = _cart->GetSpcData();
	if(spc) {
		track.Artist = spc->Artist;
		track.Comment = spc->Comment;
		track.GameTitle = spc->GameTitle;
		track.SongTitle = spc->SongTitle;
		track.Position = _ppu->GetFrameCount() / GetFps();
		track.Length = -1;
		track.TrackNumber = _spcTrackNumber + 1;
		track.TrackCount = (uint32_t)_spcPlaylist.size();

	}
	return track;
}

void Console::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	if(_spcTrackNumber >= 0) {
		int i = (int)_spcTrackNumber;
		switch(p.Action) {
			case AudioPlayerAction::PrevTrack: i--; break;
			case AudioPlayerAction::NextTrack: i++; break;
		}

		if(i < 0) {
			i = (int)_spcPlaylist.size() - 1;
		} else if(i >= _spcPlaylist.size()) {
			i = 0;
		}

		_emu->LoadRom(VirtualFile(_spcPlaylist[i]), VirtualFile());
	}
}

double Console::GetFrameDelay()
{
	UpdateRegion();
	switch(_region) {
		default:
		case ConsoleRegion::Ntsc: return _settings->GetVideoConfig().IntegerFpsMode ? 16.6666666666666666667 : 16.63926405550947;
		case ConsoleRegion::Pal: return _settings->GetVideoConfig().IntegerFpsMode ? 20 : 19.99720882631146;
	}
}

void Console::Serialize(Serializer& s)
{
	s.Stream(_cpu.get());
	s.Stream(_memoryManager.get());
	s.Stream(_ppu.get());
	s.Stream(_dmaController.get());
	s.Stream(_internalRegisters.get());
	s.Stream(_cart.get());
	s.Stream(_controlManager.get());
	s.Stream(_spc.get());
	if(_msu1) {
		s.Stream(_msu1.get());
	}
}

shared_ptr<Cpu> Console::GetCpu()
{
	return _cpu;
}

shared_ptr<Ppu> Console::GetPpu()
{
	return _ppu;
}

shared_ptr<Spc> Console::GetSpc()
{
	return _spc;
}

shared_ptr<BaseCartridge> Console::GetCartridge()
{
	return _cart;
}

shared_ptr<MemoryManager> Console::GetMemoryManager()
{
	return _memoryManager;
}

shared_ptr<InternalRegisters> Console::GetInternalRegisters()
{
	return _internalRegisters;
}

shared_ptr<IControlManager> Console::GetControlManager()
{
	return _controlManager;
}

shared_ptr<DmaController> Console::GetDmaController()
{
	return _dmaController;
}

shared_ptr<Msu1> Console::GetMsu1()
{
	return _msu1;
}

Emulator* Console::GetEmulator()
{
	return _emu;
}

bool Console::IsRunning()
{
	return _cpu != nullptr;
}

AddressInfo Console::GetAbsoluteAddress(AddressInfo relAddress)
{
	if(relAddress.Type == SnesMemoryType::CpuMemory) {
		if(_memoryManager->IsRegister(relAddress.Address)) {
			return { relAddress.Address & 0xFFFF, SnesMemoryType::Register };
		} else {
			return _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
		}
	} else if(relAddress.Type == SnesMemoryType::SpcMemory) {
		return _spc->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::Sa1Memory) {
		return _cart->GetSa1()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::GsuMemory) {
		return _cart->GetGsu()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::Cx4Memory) {
		return _cart->GetCx4()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::NecDspMemory) {
		return { relAddress.Address, SnesMemoryType::DspProgramRom };
	} else if(relAddress.Type == SnesMemoryType::GameboyMemory) {
		return _cart->GetGameboy()->GetAbsoluteAddress(relAddress.Address);
	}

	throw std::runtime_error("Unsupported address type");
}

AddressInfo Console::GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
{
	MemoryMappings* mappings = nullptr;
	switch(cpuType) {
		case CpuType::Cpu: mappings = _memoryManager->GetMemoryMappings(); break;
		case CpuType::Spc: break;
		case CpuType::NecDsp: break;
		case CpuType::Sa1: mappings = _cart->GetSa1()->GetMemoryMappings(); break;
		case CpuType::Gsu: mappings = _cart->GetGsu()->GetMemoryMappings(); break;
		case CpuType::Cx4: mappings = _cart->GetCx4()->GetMemoryMappings(); break;
		case CpuType::Gameboy: break;
	}

	switch(absAddress.Type) {
		case SnesMemoryType::PrgRom:
		case SnesMemoryType::WorkRam:
		case SnesMemoryType::SaveRam:
		{
			if(!mappings) {
				throw std::runtime_error("Unsupported cpu type");
			}

			uint8_t startBank = 0;
			//Try to find a mirror close to where the PC is
			if(cpuType == CpuType::Cpu) {
				if(absAddress.Type == SnesMemoryType::WorkRam) {
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

		case SnesMemoryType::SpcRam:
		case SnesMemoryType::SpcRom:
			return { _spc->GetRelativeAddress(absAddress), SnesMemoryType::SpcMemory };

		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
		case SnesMemoryType::GbBootRom:
			return { _cart->GetGameboy()->GetRelativeAddress(absAddress), SnesMemoryType::GameboyMemory };

		case SnesMemoryType::DspProgramRom:
			return { absAddress.Address, SnesMemoryType::NecDspMemory };

		case SnesMemoryType::Register:
			return { absAddress.Address & 0xFFFF, SnesMemoryType::Register };

		default:
			return { -1, SnesMemoryType::Register };
	}
}