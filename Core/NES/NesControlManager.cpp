#include "stdafx.h"
#include "NES/NesControlManager.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "Shared/BatteryManager.h"
#include "Shared/Emulator.h"
#include "Shared/KeyManager.h"
#include "Shared/SystemActionManager.h"
#include "NES/Input/NesController.h"
#include "SNES/Input/SnesController.h"
#include "SNES/Input/SnesMouse.h"
#include "NES/Input/Zapper.h"
#include "NES/Input/ArkanoidController.h"
#include "NES/Input/OekaKidsTablet.h"
#include "NES/Input/TwoPlayerAdapter.h"
#include "NES/Input/FourScore.h"
#include "NES/Input/PowerPad.h"
#include "NES/Input/FamilyMatTrainer.h"
#include "NES/Input/KonamiHyperShot.h"
#include "NES/Input/FamilyBasicKeyboard.h"
#include "NES/Input/FamilyBasicDataRecorder.h"
#include "NES/Input/PartyTap.h"
#include "NES/Input/PachinkoController.h"
#include "NES/Input/ExcitingBoxingController.h"
#include "NES/Input/SuborKeyboard.h"
#include "NES/Input/SuborMouse.h"
#include "NES/Input/JissenMahjongController.h"
#include "NES/Input/BarcodeBattlerReader.h"
#include "NES/Input/HoriTrack.h"
#include "NES/Input/BandaiHyperShot.h"
#include "NES/Input/VsZapper.h"
#include "NES/Input/AsciiTurboFile.h"
#include "NES/Input/BattleBox.h"
#include "NES/Input/VirtualBoyController.h"

NesControlManager::NesControlManager(NesConsole* console) : BaseControlManager(console->GetEmulator())
{
	_console = console;
}

NesControlManager::~NesControlManager()
{
}

shared_ptr<BaseControlDevice> NesControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;
	
	ControllerConfig controllers[4];
	NesConfig& cfg = _emu->GetSettings()->GetNesConfig();
	KeyMappingSet keys;
	switch(port) {
		default:
		case 0: keys = cfg.Port1.Keys; break;
		case 1: keys = cfg.Port2.Keys; break;

		//Used by VS system
		case 2: keys = cfg.Port1SubPorts[2].Keys; break;
		case 3: keys = cfg.Port1SubPorts[3].Keys; break;
	}

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::NesController: device.reset(new NesController(_emu, type, port, keys)); break;
		case ControllerType::FamicomController: device.reset(new NesController(_emu, type, port, keys)); break;
		case ControllerType::NesZapper: {
			RomFormat romFormat = _console->GetRomFormat();
			if(romFormat == RomFormat::VsSystem || romFormat == RomFormat::VsDualSystem) {
				device.reset(new VsZapper(_console, port));
			} else {
				device.reset(new Zapper(_console, type, port));
			}
			break;
		}

		case ControllerType::NesArkanoidController: device.reset(new ArkanoidController(_emu, type, port)); break;
		case ControllerType::SnesController: device.reset(new SnesController(_emu, port, keys)); break;
		case ControllerType::PowerPad: device.reset(new PowerPad(_emu, type, port, keys)); break;
		case ControllerType::SnesMouse: device.reset(new SnesMouse(_emu, port)); break;
		case ControllerType::SuborMouse: device.reset(new SuborMouse(_emu, port)); break;
		case ControllerType::VirtualBoyController: device.reset(new VirtualBoyController(_emu, port, keys)); break;

		//Exp port devices
		case ControllerType::FamicomZapper: device.reset(new Zapper(_console, type, BaseControlDevice::ExpDevicePort)); break;
		case ControllerType::FamicomArkanoidController: device.reset(new ArkanoidController(_emu, type, BaseControlDevice::ExpDevicePort)); break;
		case ControllerType::OekaKidsTablet: device.reset(new OekaKidsTablet(_emu)); break;
		case ControllerType::FamilyTrainerMat: device.reset(new FamilyMatTrainer(_emu, keys)); break;
		case ControllerType::KonamiHyperShot: device.reset(new KonamiHyperShot(_emu, keys, cfg.Port2.Keys)); break;
		case ControllerType::FamilyBasicKeyboard: device.reset(new FamilyBasicKeyboard(_emu, keys)); break;
		case ControllerType::PartyTap: device.reset(new PartyTap(_emu, keys)); break;
		case ControllerType::Pachinko: device.reset(new PachinkoController(_emu, keys)); break;
		case ControllerType::ExcitingBoxing: device.reset(new ExcitingBoxingController(_emu, keys)); break;
		case ControllerType::JissenMahjong: device.reset(new JissenMahjongController(_emu, keys)); break;
		case ControllerType::SuborKeyboard: device.reset(new SuborKeyboard(_emu, keys)); break;
		case ControllerType::BarcodeBattler: device.reset(new BarcodeBattlerReader(_emu)); break;
		case ControllerType::HoriTrack: device.reset(new HoriTrack(_emu, keys)); break;
		case ControllerType::BandaiHyperShot: device.reset(new BandaiHyperShot(_console, keys)); break;
		case ControllerType::AsciiTurboFile: device.reset(new AsciiTurboFile(_emu)); break;
		case ControllerType::BattleBox: device.reset(new BattleBox(_emu)); break;
		
		case ControllerType::FourScore: {
			std::copy(cfg.Port1SubPorts, cfg.Port1SubPorts + 4, controllers);
			controllers[0].Keys = cfg.Port1.Keys;
			device.reset(new FourScore(_emu, type, controllers));
			break;
		}

		case ControllerType::TwoPlayerAdapter:
		case ControllerType::FourPlayerAdapter: {
			std::copy(cfg.ExpPortSubPorts, cfg.ExpPortSubPorts + 4, controllers);
			controllers[0].Keys = cfg.ExpPort.Keys;
			if(type == ControllerType::TwoPlayerAdapter) {
				device.reset(new TwoPlayerAdapter(_emu, type, controllers));
			} else {
				device.reset(new FourScore(_emu, type, controllers));
			}
			break;
		}

		default: break;
	}

	return device;
}

void NesControlManager::UpdateControlDevices()
{
	NesConfig& cfg = _emu->GetSettings()->GetNesConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();
	//bool hadKeyboard = HasKeyboard();

	ClearDevices();

	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device = CreateControllerDevice(i == 0 ? cfg.Port1.Type : cfg.Port2.Type, i);
		if(device) {
			RegisterControlDevice(device);
		}
	}

	if(cfg.ExpPort.Type != ControllerType::None) {
		shared_ptr<BaseControlDevice> expDevice = CreateControllerDevice(cfg.ExpPort.Type, BaseControlDevice::ExpDevicePort);
		if(expDevice) {
			RegisterControlDevice(expDevice);
		}
	}

	//TODO
	/*bool hasKeyboard = HasKeyboard();
	if(!hasKeyboard) {
		settings->DisableKeyboardMode();
	} else if(!hadKeyboard && hasKeyboard) {
		settings->EnableKeyboardMode();
	}
	
	if(std::dynamic_pointer_cast<FamilyBasicKeyboard>(expDevice)) {
		//Automatically connect the data recorder if the keyboard is connected
		RegisterControlDevice(shared_ptr<FamilyBasicDataRecorder>(new FamilyBasicDataRecorder(_console)));
	}*/
}

bool NesControlManager::HasKeyboard()
{
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->GetControllerType() == ControllerType::SuborKeyboard || device->GetControllerType() == ControllerType::FamilyBasicKeyboard) {
			return true;
		}
	}
	return false;
}

uint8_t NesControlManager::GetOpenBusMask(uint8_t port)
{
	//"In the NES and Famicom, the top three (or five) bits are not driven, and so retain the bits of the previous byte on the bus. 
	//Usually this is the most significant byte of the address of the controller port - 0x40.
	//Paperboy relies on this behavior and requires that reads from the controller ports return exactly $40 or $41 as appropriate."
	switch(_console->GetNesConfig().ConsoleType) {
		default:
		case NesConsoleType::Nes001:
				return 0xE0;

		case NesConsoleType::Nes101:
			return port == 0 ? 0xE4 : 0xE0;

		case NesConsoleType::Hvc001:
		case NesConsoleType::Hvc101:
			return port == 0 ? 0xF8 : 0xE0;
	}
}

void NesControlManager::RemapControllerButtons()
{
	//Used by VS System games
}

void NesControlManager::UpdateInputState()
{
	if(_isLagging) {
		_lagCounter++;
	} else {
		_isLagging = true;
	}

	BaseControlManager::UpdateInputState();

	//Used by VS System games
	RemapControllerButtons();
}

uint32_t NesControlManager::GetLagCounter()
{
	return _lagCounter;
}

void NesControlManager::ResetLagCounter()
{
	_lagCounter = 0;
}

uint8_t NesControlManager::ReadRam(uint16_t addr)
{
	//Used for lag counter - any frame where the input is read does not count as lag
	_isLagging = false;

	uint8_t value = _console->GetMemoryManager()->GetOpenBus(GetOpenBusMask(addr - 0x4016));
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		if(device->IsConnected()) {
			value |= device->ReadRam(addr);
		}
	}

	return value;
}

void NesControlManager::WriteRam(uint16_t addr, uint8_t value)
{
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		if(device->IsConnected()) {
			device->WriteRam(addr, value);
		}
	}
}

void NesControlManager::Reset(bool softReset)
{
	ResetLagCounter();
}

void NesControlManager::Serialize(Serializer& s)
{
	//Restore controllers that were being used at the time the snapshot was made
	//This is particularly important to ensure proper sync during NetPlay
	ControllerType controllerTypes[4];
	ExpansionPortDevice expansionDevice;
	ConsoleType consoleType;
	bool hasFourScore = false;
	bool useNes101Hvc101Behavior = false;
	uint32_t zapperDetectionRadius = 0;
	if(s.IsSaving()) {
		//TODO
		/*
		expansionDevice = settings->GetExpansionDevice();
		consoleType = settings->GetConsoleType();
		hasFourScore = settings->CheckFlag(EmulationFlags::HasFourScore);
		useNes101Hvc101Behavior = settings->CheckFlag(EmulationFlags::UseNes101Hvc101Behavior);
		zapperDetectionRadius = settings->GetZapperDetectionRadius();
		for(int i = 0; i < 4; i++) {
			controllerTypes[i] = settings->GetControllerType(i);
		}*/
	}

	ArrayInfo<ControllerType> types = { controllerTypes, 4 };
	s.Stream(expansionDevice, consoleType, types, hasFourScore, useNes101Hvc101Behavior, zapperDetectionRadius, _lagCounter, _pollCounter);

	if(!s.IsSaving()) {
		//TODO
		/*
		settings->SetExpansionDevice(expansionDevice);
		settings->SetConsoleType(consoleType);
		for(int i = 0; i < 4; i++) {
			settings->SetControllerType(i, controllerTypes[i]);
		}

		settings->SetZapperDetectionRadius(zapperDetectionRadius);
		settings->SetFlagState(EmulationFlags::HasFourScore, hasFourScore);
		settings->SetFlagState(EmulationFlags::UseNes101Hvc101Behavior, useNes101Hvc101Behavior);
		*/
		UpdateControlDevices();
	}

	for(uint8_t i = 0; i < _controlDevices.size(); i++) {
		s.Stream(_controlDevices[i].get());
	}
}
