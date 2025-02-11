#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/NesControlManager.h"
#include "NES/MapperFactory.h"
#include "NES/APU/NesApu.h"
#include "NES/NesCpu.h"
#include "NES/BaseMapper.h"
#include "NES/NesSoundMixer.h"
#include "NES/NesMemoryManager.h"
#include "NES/DefaultNesPpu.h"
#include "NES/NsfPpu.h"
#include "NES/HdPacks/HdAudioDevice.h"
#include "NES/HdPacks/HdData.h"
#include "NES/HdPacks/HdNesPpu.h"
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/HdPacks/HdPackBuilder.h"
#include "NES/HdPacks/HdBuilderPpu.h"
#include "NES/HdPacks/HdVideoFilter.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesNtscFilter.h"
#include "NES/BisqwitNtscFilter.h"
#include "NES/NesConstants.h"
#include "NES/Epsm.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "NES/Mappers/NSF/NsfMapper.h"
#include "NES/Mappers/FDS/Fds.h"
#include "Shared/Emulator.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/SaveStateManager.h"
#include "Shared/CheatManager.h"
#include "Shared/Movies/MovieManager.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Interfaces/IBattery.h"
#include "Shared/EmuSettings.h"
#include "Shared/NotificationManager.h"
#include "Netplay/GameClient.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/Serializer.h"
#include "Utilities/sha1.h"

NesConsole::NesConsole(Emulator* emu)
{
	_emu = emu;
}

NesConsole::~NesConsole()
{
	shared_ptr<HdPackData> hdData = _hdData.lock();
	if(hdData) {
		hdData->CancelLoad();
	}
}

Emulator* NesConsole::GetEmulator()
{
	return _emu;
}

NesConfig& NesConsole::GetNesConfig()
{
	return _emu->GetSettings()->GetNesConfig();
}

void NesConsole::ProcessCpuClock() 
{
	if(_mapper->HasCpuClockHook()) {
		_mapper->ProcessCpuClock();
	}

	_apu->ProcessCpuClock();
	if(_controlManager->HasPendingWrites()) {
		_controlManager->ProcessWrites();
	}
}

Epsm* NesConsole::GetEpsm()
{
	return _mapper->GetEpsm();
}

NesConsole* NesConsole::GetVsMainConsole()
{
	return _vsMainConsole;
}

NesConsole* NesConsole::GetVsSubConsole()
{
	return _vsSubConsole.get();
}

bool NesConsole::IsVsMainConsole()
{
	return _vsMainConsole == nullptr;
}

void NesConsole::Serialize(Serializer& s)
{
	SV(_cpu);
	SV(_ppu);
	SV(_memoryManager);
	SV(_apu);
	SV(_mapper);

	if(s.GetFormat() != SerializeFormat::Map) {
		SV(_mixer);
	}

	if(_hdAudioDevice) {
		//For HD packs), save the state of the bgm playback
		SV(_hdAudioDevice);
	}

	if(_vsSubConsole) {
		//For VS Dualsystem, the sub console's savestate is appended to the end of the file
		SV(_vsSubConsole);
	}
	
	SV(_controlManager);

	if(!s.IsSaving()) {
		UpdateRegion(true);
	}
}

void NesConsole::Reset()
{
	_memoryManager->Reset(true);
	
	_ppu->Reset(true);
	_apu->Reset(true);
	_cpu->Reset(true, _region);
	_controlManager->Reset(true);
	_mixer->Reset();
	if(_vsSubConsole) {
		_vsSubConsole->Reset();
	}
	_mapper->OnAfterResetPowerOn();
}

LoadRomResult NesConsole::LoadRom(VirtualFile& romFile)
{
	RomData romData;

	LoadHdPack(romFile);

	LoadRomResult result = LoadRomResult::UnknownType;
	unique_ptr<BaseMapper> mapper = MapperFactory::InitializeFromFile(this, romFile, romData, result);
	if(mapper) {
		if(!_vsMainConsole && romData.Info.VsType == VsSystemType::VsDualSystem) {
			//Create 2nd console (sub) dualsystem games
			_vsSubConsole.reset(new NesConsole(_emu));
			_vsSubConsole->_vsMainConsole = this;
			result = _vsSubConsole->LoadRom(romFile);
			if(result != LoadRomResult::Success) {
				return result;
			}
		}
		
		if(GetNesConfig().AutoConfigureInput && romData.Info.InputType != GameInputType::Unspecified) {
			//Auto-configure the inputs (if option is enabled)
			InitializeInputDevices(romData.Info.InputType, romData.Info.System);
		}

		_mapper.swap(mapper);
		_mixer.reset(new NesSoundMixer(this));
		_memoryManager.reset(new NesMemoryManager(this, _mapper.get()));
		_cpu.reset(new NesCpu(this));
		_apu.reset(new NesApu(this));

		if(romData.Info.System == GameSystem::VsSystem) {
			_controlManager.reset(new VsControlManager(this));
		} else {
			_controlManager.reset(new NesControlManager(this));
		}

		if(_hdData) {
			_ppu.reset(new HdNesPpu(this, _hdData.get()));
		} else if(dynamic_cast<NsfMapper*>(_mapper.get())) {
			//Disable most of the PPU for NSFs
			_ppu.reset(new NsfPpu(this));
		} else {
			_ppu.reset(new DefaultNesPpu(this));
		}

		_mapper->InitSpecificMapper(romData);

		if(_mapper->GetEpsm()) {
			_memoryManager->RegisterIODevice(_mapper->GetEpsm());
		}
		_memoryManager->RegisterIODevice(_ppu.get());
		_memoryManager->RegisterIODevice(_apu.get());
		_memoryManager->RegisterIODevice(_controlManager.get());
		_memoryManager->RegisterIODevice(_mapper.get());

		if(_hdData) {
			_hdAudioDevice.reset(new HdAudioDevice(_emu, _hdData.get()));
			_memoryManager->RegisterIODevice(_hdAudioDevice.get());
		} else {
			_hdAudioDevice.reset();
		}

		UpdateRegion();

		_mixer->Reset();
		
		_ppu->Reset(false);
		_apu->Reset(false);
		_memoryManager->Reset(false);
		_controlManager->Reset(false);
		_cpu->Reset(false, _region);
		_mapper->OnAfterResetPowerOn();
	}
    return result;
}

void NesConsole::LoadHdPack(VirtualFile& romFile)
{
	_hdData.reset();
	if(GetNesConfig().EnableHdPacks) {
		_hdData.reset(new HdPackData());
		if(!HdPackLoader::LoadHdNesPack(romFile, *_hdData.get())) {
			_hdData.reset();
		} else {
			auto result = _hdData->PatchesByHash.find(romFile.GetSha1Hash());
			if(result != _hdData->PatchesByHash.end()) {
				VirtualFile patchFile = result->second;
				romFile.ApplyPatch(patchFile);
			}

			shared_ptr<HdPackData> data = _hdData.lock();
			if(data) {
				thread asyncLoadData([data]() {
					data->LoadAsync();
				});
				asyncLoadData.detach();
			}
		}
	}
}

void NesConsole::UpdateRegion(bool forceUpdate)
{
	ConsoleRegion region = GetNesConfig().Region;
	if(region == ConsoleRegion::Auto) {
		switch(_mapper->GetRomInfo().System) {
			case GameSystem::NesPal: region = ConsoleRegion::Pal; break;
			case GameSystem::Dendy: region = ConsoleRegion::Dendy; break;
			default: region = ConsoleRegion::Ntsc; break;
		}
	}

	if(_vsSubConsole) {
		_vsSubConsole->UpdateRegion(forceUpdate);
	}

	if(_region != region || forceUpdate) {
		_region = region;

		_cpu->SetMasterClockDivider(_region);
		_mapper->SetRegion(_region);
		_ppu->UpdateTimings(_region);
		_apu->SetRegion(_region);
		_mixer->SetRegion(_region);
	}
}

void NesConsole::RunFrame()
{
	UpdateRegion();

	uint32_t frame = _ppu->GetFrameCount();

	if(_nextFrameOverclockDisabled) {
		//Disable overclocking for the next frame
		//This is used by the DMC when a sample is playing
		_ppu->UpdateTimings(_region, false);
		_nextFrameOverclockDisabled = false;
	}

	while(frame == _ppu->GetFrameCount()) {
		_cpu->Exec();
		if(_vsSubConsole) {
			RunVsSubConsole();
		}
	}

	_apu->EndFrame();

	if(!_nextFrameOverclockDisabled) {
		//Re-update timings to allow overclocking
		_ppu->UpdateTimings(_region, true);
	}
}

void NesConsole::RunVsSubConsole()
{
	int64_t cycleGap;
	while(true) {
		//Run the sub console until it catches up to the main CPU
		cycleGap = (int64_t)(_cpu->GetCycleCount() - _vsSubConsole->_cpu->GetCycleCount());
		if(cycleGap > 5 || _ppu->GetFrameCount() > _vsSubConsole->_ppu->GetFrameCount()) {
			_vsSubConsole->_cpu->Exec();
		} else {
			break;
		}
	}
}

void NesConsole::SetNextFrameOverclockStatus(bool disabled)
{
	//Disable overclocking for the next frame
	//This is used by the DMC when a sample is playing
	_nextFrameOverclockDisabled = disabled;
}

BaseControlManager* NesConsole::GetControlManager()
{
	return _controlManager.get();
}

double NesConsole::GetFps()
{
	UpdateRegion();
	if(_region == ConsoleRegion::Ntsc) {
		return 60.0988118623484;
	} else {
		return 50.0069789081886;
	}
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
	PpuFrameInfo frame;
	frame.FrameBuffer = (uint8_t*)_ppu->GetScreenBuffer(false);
	frame.Width = NesConstants::ScreenWidth;
	frame.Height = NesConstants::ScreenHeight;
	frame.FrameBufferSize = frame.Width * frame.Height * sizeof(uint16_t);
	frame.FrameCount = _ppu->GetFrameCount();
	frame.FirstScanline = -1;
	frame.ScanlineCount = _ppu->GetScanlineCount();
	frame.CycleCount = 341;
	return frame;
}

ConsoleType NesConsole::GetConsoleType()
{
	return ConsoleType::Nes;
}

ConsoleRegion NesConsole::GetRegion()
{
	return _region;
}

vector<CpuType> NesConsole::GetCpuTypes()
{
	return { CpuType::Nes };
}

AddressInfo NesConsole::GetAbsoluteAddress(AddressInfo& relAddress)
{
	if(relAddress.Type == MemoryType::NesMemory) {
		return _mapper->GetAbsoluteAddress(relAddress.Address);
	} else {
		return _mapper->GetPpuAbsoluteAddress(relAddress.Address);
	}
}

AddressInfo NesConsole::GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType)
{
	return _mapper->GetRelativeAddress(absAddress);
}

void NesConsole::GetConsoleState(BaseState& baseState, ConsoleType consoleType)
{
	NesState& state = (NesState&)baseState;

	state.ClockRate = GetMasterClockRate();
	state.Cpu = _cpu->GetState();
	_ppu->GetState(state.Ppu);
	state.Cartridge = _mapper->GetState();
	state.Apu = _apu->GetState();
}

uint64_t NesConsole::GetMasterClock()
{
	return _cpu->GetCycleCount();
}

uint32_t NesConsole::GetMasterClockRate()
{
	return NesConstants::GetClockRate(_region);
}

void NesConsole::SaveBattery()
{
	if(_mapper) {
		_mapper->SaveBattery();
	}
	
	if(_controlManager) {
		_controlManager->SaveBattery();
	}
}

ShortcutState NesConsole::IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam)
{
	bool isRunning = _emu->IsRunning();
	bool isNetplayClient = _emu->GetGameClient()->Connected();
	bool isMoviePlaying = _emu->GetMovieManager()->Playing();
	RomFormat romFormat = GetRomFormat();
	
	switch(shortcut) {
		case EmulatorShortcut::FdsEjectDisk:
		case EmulatorShortcut::FdsInsertNextDisk:
		case EmulatorShortcut::FdsSwitchDiskSide:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::Fds);

		case EmulatorShortcut::FdsInsertDiskNumber:
			if(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::Fds) {
				Fds* fds = dynamic_cast<Fds*>(_mapper.get());
				return (ShortcutState)(fds && shortcutParam < fds->GetSideCount());
			}
			return ShortcutState::Disabled;

		case EmulatorShortcut::VsInsertCoin1:
		case EmulatorShortcut::VsInsertCoin2:
		case EmulatorShortcut::VsServiceButton:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && (romFormat == RomFormat::VsSystem || romFormat == RomFormat::VsDualSystem));

		case EmulatorShortcut::VsInsertCoin3:
		case EmulatorShortcut::VsInsertCoin4:
		case EmulatorShortcut::VsServiceButton2:
			return (ShortcutState)(isRunning && !isNetplayClient && !isMoviePlaying && romFormat == RomFormat::VsDualSystem);
	}

	return ShortcutState::Default;
}

BaseVideoFilter* NesConsole::GetVideoFilter(bool getDefaultFilter)
{
	if(getDefaultFilter || GetRomFormat() == RomFormat::Nsf) {
		return new NesDefaultVideoFilter(_emu);
	} else if(_hdData && !_hdPackBuilder) {
		return new HdVideoFilter(this, _emu, _hdData.get());
	} else {
		VideoFilterType filterType = _emu->GetSettings()->GetVideoConfig().VideoFilter;

		switch(filterType) {
			case VideoFilterType::NtscBlargg: return new NesNtscFilter(_emu);
			case VideoFilterType::NtscBisqwit: return new BisqwitNtscFilter(_emu);
			default: return new NesDefaultVideoFilter(_emu);
		}
	}
}

string NesConsole::GetHash(HashType hashType)
{
	if(hashType == HashType::Sha1Cheat) {
		ConsoleMemoryInfo prgRom = _emu->GetMemory(MemoryType::NesPrgRom);
		if(prgRom.Size && prgRom.Memory) {
			return SHA1::GetHash((uint8_t*)prgRom.Memory, prgRom.Size);
		}
	}

	return "";
}

RomFormat NesConsole::GetRomFormat()
{
	return _mapper->GetRomInfo().Format;
}

AudioTrackInfo NesConsole::GetAudioTrackInfo()
{
	NsfMapper* nsfMapper = dynamic_cast<NsfMapper*>(_mapper.get());
	if(nsfMapper) {
		return nsfMapper->GetAudioTrackInfo();
	}
	return {};
}

void NesConsole::ProcessAudioPlayerAction(AudioPlayerActionParams p)
{
	NsfMapper* nsfMapper = dynamic_cast<NsfMapper*>(_mapper.get());
	if(nsfMapper) {
		return nsfMapper->ProcessAudioPlayerAction(p);
	}
}

uint8_t NesConsole::DebugRead(uint16_t addr)
{
	return _memoryManager->DebugRead(addr);
}

void NesConsole::DebugWrite(uint16_t addr, uint8_t value, bool disableSideEffects)
{
	_memoryManager->DebugWrite(addr, value, disableSideEffects);
}

uint8_t NesConsole::DebugReadVram(uint16_t addr)
{
	if(addr >= 0x3F00) {
		return _ppu->ReadPaletteRam(addr);
	} else {
		return _mapper->DebugReadVram(addr);
	}
}

void NesConsole::DebugWriteVram(uint16_t addr, uint8_t value)
{
	if(addr >= 0x3F00) {
		_ppu->WritePaletteRam(addr, value);
	} else {
		_mapper->DebugWriteVram(addr, value);
	}
}

void NesConsole::InitializeInputDevices(GameInputType inputType, GameSystem system)
{
	ControllerType port1 = ControllerType::NesController;
	ControllerType port2 = ControllerType::NesController;
	ControllerType expDevice = ControllerType::None;

	auto log = [](string text) {
		MessageManager::Log(text);
	};

	bool isFamicom = (system == GameSystem::Famicom || system == GameSystem::FDS || system == GameSystem::Dendy);

	if(inputType == GameInputType::VsZapper) {
		//VS Duck Hunt, etc. need the zapper in the first port
		log("[Input] VS Zapper connected");
		port1 = ControllerType::NesZapper;
	} else if(inputType == GameInputType::Zapper) {
		log("[Input] Zapper connected");
		if(isFamicom) {
			expDevice = ControllerType::FamicomZapper;
		} else {
			port2 = ControllerType::NesZapper;
		}
	} else if(inputType == GameInputType::FourScore) {
		log("[Input] Four score connected");
		port1 = ControllerType::FourScore;
		port2 = ControllerType::FourScore;
	} else if(inputType == GameInputType::FourPlayerAdapter) {
		log("[Input] Four player adapter connected");
		expDevice = ControllerType::TwoPlayerAdapter;
	} else if(inputType == GameInputType::ArkanoidControllerFamicom) {
		log("[Input] Arkanoid controller (Famicom) connected");
		expDevice = ControllerType::FamicomArkanoidController;
	} else if(inputType == GameInputType::ArkanoidControllerNes) {
		log("[Input] Arkanoid controller (NES) connected");
		port2 = ControllerType::NesArkanoidController;
	} else if(inputType == GameInputType::DoubleArkanoidController) {
		log("[Input] 2 arkanoid controllers (NES) connected");
		port1 = ControllerType::NesArkanoidController;
		port2 = ControllerType::NesArkanoidController;
	} else if(inputType == GameInputType::OekaKidsTablet) {
		log("[Input] Oeka Kids Tablet connected");
		expDevice = ControllerType::OekaKidsTablet;
	} else if(inputType == GameInputType::KonamiHyperShot) {
		log("[Input] Konami Hyper Shot connected");
		expDevice = ControllerType::KonamiHyperShot;
	} else if(inputType == GameInputType::FamilyBasicKeyboard) {
		log("[Input] Family Basic Keyboard connected");
		expDevice = ControllerType::FamilyBasicKeyboard;
	} else if(inputType == GameInputType::PartyTap) {
		log("[Input] Party Tap connected");
		expDevice = ControllerType::PartyTap;
	} else if(inputType == GameInputType::PachinkoController) {
		log("[Input] Pachinko controller connected");
		expDevice = ControllerType::Pachinko;
	} else if(inputType == GameInputType::ExcitingBoxing) {
		log("[Input] Exciting Boxing controller connected");
		expDevice = ControllerType::ExcitingBoxing;
	} else if(inputType == GameInputType::SuborKeyboardMouse1) {
		log("[Input] Subor mouse connected");
		log("[Input] Subor keyboard connected");
		expDevice = ControllerType::SuborKeyboard;
		port2 = ControllerType::SuborMouse;
	} else if(inputType == GameInputType::JissenMahjong) {
		log("[Input] Jissen Mahjong controller connected");
		expDevice = ControllerType::JissenMahjong;
	} else if(inputType == GameInputType::BarcodeBattler) {
		log("[Input] Barcode Battler barcode reader connected");
		expDevice = ControllerType::BarcodeBattler;
	} else if(inputType == GameInputType::BandaiHypershot) {
		log("[Input] Bandai Hyper Shot gun connected");
		expDevice = ControllerType::BandaiHyperShot;
	} else if(inputType == GameInputType::BattleBox) {
		log("[Input] Battle Box connected");
		expDevice = ControllerType::BattleBox;
	} else if(inputType == GameInputType::TurboFile) {
		log("[Input] Ascii Turbo File connected");
		expDevice = ControllerType::AsciiTurboFile;
	} else if(inputType == GameInputType::FamilyTrainerSideA) {
		log("[Input] Family Trainer mat connected (Side A)");
		expDevice = ControllerType::FamilyTrainerMatSideA;
	} else if(inputType == GameInputType::FamilyTrainerSideB) {
		log("[Input] Family Trainer mat connected (Side B)");
		expDevice = ControllerType::FamilyTrainerMatSideB;
	} else if(inputType == GameInputType::PowerPadSideA) {
		log("[Input] Power Pad connected (Side A)");
		port2 = ControllerType::PowerPadSideA;
	} else if(inputType == GameInputType::PowerPadSideB) {
		log("[Input] Power Pad connected (Side B)");
		port2 = ControllerType::PowerPadSideB;
	} else if(inputType == GameInputType::SnesControllers) {
		log("[Input] 2 SNES controllers connected");
		port1 = ControllerType::SnesController;
		port2 = ControllerType::SnesController;
	} else {
		log("[Input] 2 NES controllers connected");
	}

	isFamicom = (system == GameSystem::Famicom || system == GameSystem::FDS || system == GameSystem::Dendy);

	NesConfig& cfg = GetNesConfig();
	cfg.Port1.Type = port1;
	cfg.Port2.Type = port2;
	cfg.ExpPort.Type = expDevice;

	if(port1 == ControllerType::FourScore) {
		cfg.Port1SubPorts[0].Type = ControllerType::NesController;
		cfg.Port1SubPorts[1].Type = ControllerType::NesController;
		cfg.Port1SubPorts[2].Type = ControllerType::NesController;
		cfg.Port1SubPorts[3].Type = ControllerType::NesController;
	} else if(expDevice == ControllerType::TwoPlayerAdapter) {
		cfg.ExpPortSubPorts[0].Type = ControllerType::NesController;
		cfg.ExpPortSubPorts[1].Type = ControllerType::NesController;
	}
	_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::RequestConfigChange);
}

void NesConsole::ProcessCheatCode(InternalCheatCode& code, uint32_t addr, uint8_t& value)
{
	if(code.Type == CheatType::NesGameGenie && addr >= 0xC020) {
		if(GetNesConfig().DisableGameGenieBusConflicts || _mapper->HasDefaultWorkRam()) {
			return;
		}

		AddressInfo absAddr = _mapper->GetAbsoluteAddress(addr - 0x8000);
		if(absAddr.Address >= 0) {
			//Game Genie causes a bus conflict when the cartridge maps anything below $8000
			//Only processed when addr >= $C020 because the mapper implementation never maps anything below $4020
			value &= _mapper->DebugReadRam(addr - 0x8000);
		}
	}
}

void NesConsole::InitializeRam(void* data, uint32_t length)
{
	EmuSettings* settings = _emu->GetSettings();
	settings->InitializeRam(settings->GetNesConfig().RamPowerOnState, data, length);
}

DipSwitchInfo NesConsole::GetDipSwitchInfo()
{
	DipSwitchInfo info = {};
	info.DipSwitchCount = _mapper->GetMapperDipSwitchCount();
	info.DatabaseId = _mapper->GetRomInfo().Hash.PrgCrc32;
	return info;
}

void NesConsole::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::ExecuteShortcut) {
		ExecuteShortcutParams* params = (ExecuteShortcutParams*)parameter;
		switch(params->Shortcut) {
			default: break;
			case EmulatorShortcut::StartRecordHdPack: StartRecordingHdPack(*(HdPackBuilderOptions*)params->ParamPtr); break;
			case EmulatorShortcut::StopRecordHdPack: StopRecordingHdPack(); break;
		}
	}
}

void NesConsole::StartRecordingHdPack(HdPackBuilderOptions options)
{
	auto lock = _emu->AcquireLock();

	_emu->GetVideoDecoder()->WaitForAsyncFrameDecode();

	std::stringstream saveState;
	_emu->Serialize(saveState, false, 0);

	_hdPackBuilder.reset();
	_hdPackBuilder.reset(new HdPackBuilder(_emu, _ppu->GetPpuModel(), !_mapper->HasChrRom(), options));

	_memoryManager->UnregisterIODevice(_ppu.get());
	_ppu.reset(new HdBuilderPpu(this, _hdPackBuilder.get(), options.ChrRamBankSize));
	_memoryManager->RegisterIODevice(_ppu.get());

	_emu->Deserialize(saveState, SaveStateManager::FileFormatVersion, false);
	_emu->GetSoundMixer()->StopAudio();

	_emu->GetVideoDecoder()->ForceFilterUpdate();
}

void NesConsole::StopRecordingHdPack()
{
	if(_hdPackBuilder) {
		auto lock = _emu->AcquireLock();
		
		_emu->GetVideoDecoder()->WaitForAsyncFrameDecode();

		std::stringstream saveState;
		_emu->Serialize(saveState, false, 0);

		_memoryManager->UnregisterIODevice(_ppu.get());
		if(_hdData) {
			_ppu.reset(new HdNesPpu(this, _hdData.get()));
		} else {
			_ppu.reset(new DefaultNesPpu(this));
		}
		_memoryManager->RegisterIODevice(_ppu.get());
		_hdPackBuilder.reset();

		_emu->Deserialize(saveState, SaveStateManager::FileFormatVersion, false);
		_emu->GetSoundMixer()->StopAudio();
		_emu->GetVideoDecoder()->ForceFilterUpdate();
	}
}