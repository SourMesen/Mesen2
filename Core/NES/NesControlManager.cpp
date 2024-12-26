#include "pch.h"
#include "NES/NesControlManager.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "Shared/MessageManager.h"
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
#include "NES/Epsm.h"

NesControlManager::NesControlManager(NesConsole* console) : BaseControlManager(console->GetEmulator(), CpuType::Nes)
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
		case BaseControlDevice::ExpDevicePort: keys = cfg.ExpPort.Keys; break;

		//Used by VS system
		case 2: keys = cfg.Port1SubPorts[2].Keys; break;
		case 3: keys = cfg.Port1SubPorts[3].Keys; break;
	}

	switch(type) {
		case ControllerType::None: break;
		
		case ControllerType::NesController:
		case ControllerType::FamicomController:
		case ControllerType::FamicomControllerP2:
			device.reset(new NesController(_emu, type, port, keys));
			break;

		case ControllerType::NesZapper: {
			RomFormat romFormat = _console->GetRomFormat();
			if(romFormat == RomFormat::VsSystem || romFormat == RomFormat::VsDualSystem) {
				device.reset(new VsZapper(_console, port, keys));
			} else {
				device.reset(new Zapper(_console, type, port, keys));
			}
			break;
		}

		case ControllerType::NesArkanoidController: device.reset(new ArkanoidController(_emu, type, port, keys)); break;
		case ControllerType::SnesController: device.reset(new SnesController(_emu, port, keys)); break;
		
		case ControllerType::PowerPadSideA:
		case ControllerType::PowerPadSideB:
			device.reset(new PowerPad(_emu, type, port, keys));
			break;

		case ControllerType::SnesMouse: device.reset(new SnesMouse(_emu, port, keys)); break;
		case ControllerType::SuborMouse: device.reset(new SuborMouse(_emu, port, keys)); break;
		case ControllerType::VirtualBoyController: device.reset(new VirtualBoyController(_emu, port, keys)); break;

		//Exp port devices
		case ControllerType::FamicomZapper: device.reset(new Zapper(_console, type, BaseControlDevice::ExpDevicePort, keys)); break;
		case ControllerType::FamicomArkanoidController: device.reset(new ArkanoidController(_emu, type, BaseControlDevice::ExpDevicePort, keys)); break;
		case ControllerType::OekaKidsTablet: device.reset(new OekaKidsTablet(_emu, keys)); break;
		
		case ControllerType::FamilyTrainerMatSideA:
		case ControllerType::FamilyTrainerMatSideB:
			device.reset(new FamilyMatTrainer(_emu, type, keys));
			break;

		case ControllerType::KonamiHyperShot: device.reset(new KonamiHyperShot(_emu, keys)); break;
		case ControllerType::FamilyBasicKeyboard: device.reset(new FamilyBasicKeyboard(_emu, keys)); break;
		case ControllerType::PartyTap: device.reset(new PartyTap(_emu, keys)); break;
		case ControllerType::Pachinko: device.reset(new PachinkoController(_emu, keys)); break;
		case ControllerType::ExcitingBoxing: device.reset(new ExcitingBoxingController(_emu, keys)); break;
		case ControllerType::JissenMahjong: device.reset(new JissenMahjongController(_emu, keys)); break;
		case ControllerType::SuborKeyboard: device.reset(new SuborKeyboard(_emu, keys)); break;
		case ControllerType::BarcodeBattler: device.reset(new BarcodeBattlerReader(_emu)); break;
		case ControllerType::HoriTrack: device.reset(new HoriTrack(_emu, keys)); break;
		case ControllerType::BandaiHyperShot: device.reset(new BandaiHyperShot(_console, keys)); break;
		case ControllerType::AsciiTurboFile: device.reset(new AsciiTurboFile(_console)); break;
		case ControllerType::BattleBox: device.reset(new BattleBox(_console)); break;
		
		case ControllerType::FourScore: {
			std::copy(cfg.Port1SubPorts, cfg.Port1SubPorts + 4, controllers);
			//Use the p1/p2 bindings for the first 2 ports (the UI does this, too)
			controllers[0].Keys = cfg.Port1.Keys;
			controllers[1].Keys = cfg.Port2.Keys;
			device.reset(new FourScore(_emu, type, 0, controllers));
			break;
		}

		case ControllerType::TwoPlayerAdapter:
		case ControllerType::FourPlayerAdapter: {
			std::copy(cfg.ExpPortSubPorts, cfg.ExpPortSubPorts + 4, controllers);
			controllers[0].Keys = cfg.ExpPort.Keys;
			if(type == ControllerType::TwoPlayerAdapter) {
				device.reset(new TwoPlayerAdapter(_emu, type, controllers));
			} else {
				device.reset(new FourScore(_emu, type, BaseControlDevice::ExpDevicePort, controllers));
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

	bool hadKeyboard = IsKeyboardConnected();

	SaveBattery();

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
			
			if(std::dynamic_pointer_cast<FamilyBasicKeyboard>(expDevice)) {
				//Automatically connect the data recorder if the family basic keyboard is connected
				RegisterControlDevice(shared_ptr<FamilyBasicDataRecorder>(new FamilyBasicDataRecorder(_emu)));
			}
		}
	}

	if(!hadKeyboard && IsKeyboardConnected()) {
		MessageManager::DisplayMessage("Input", "KeyboardModeEnabled");
	}
}

bool NesControlManager::IsKeyboardConnected()
{
	return HasControlDevice(ControllerType::FamilyBasicKeyboard) || HasControlDevice(ControllerType::SuborKeyboard);
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
	BaseControlManager::UpdateInputState();

	//Used by VS System games
	RemapControllerButtons();
}

void NesControlManager::SaveBattery()
{
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		shared_ptr<IBattery> batteryDevice = std::dynamic_pointer_cast<IBattery>(device);
		if(batteryDevice) {
			batteryDevice->SaveBattery();
		}
	}
}

uint8_t NesControlManager::ReadRam(uint16_t addr)
{
	SetInputReadFlag();

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
	//The OUT pins are only updated at the start of PUT cycles
	_writeAddr = addr;
	_writeValue = value;
	_writePending = (_console->GetMasterClock() & 0x01) ? 1 : 2;
}

void NesControlManager::ProcessWrites()
{
	if(_writePending && --_writePending == 0) {
		if(_console->GetEpsm() && _writeAddr == 0x4016) {
			_console->GetEpsm()->Write(_console->GetMemoryManager()->GetOpenBus(), _writeValue);
		}

		for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
			if(device->IsConnected()) {
				device->WriteRam(_writeAddr, _writeValue);
			}
		}
	}
}

void NesControlManager::Reset(bool softReset)
{
}

void NesControlManager::Serialize(Serializer& s)
{
	BaseControlManager::Serialize(s);
	SV(_writeAddr);
	SV(_writeValue);
	SV(_writePending);

	if(!s.IsSaving()) {
		UpdateControlDevices();
	}

	for(uint8_t i = 0; i < _controlDevices.size(); i++) {
		SVI(_controlDevices[i]);
	}
}
