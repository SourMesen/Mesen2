#include "stdafx.h"
#include "NES/NesControlManager.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "NES/Input/NesController.h"
#include "SNES/Input/SnesController.h"
#include "Shared/EmuSettings.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "Shared/BatteryManager.h"
#include "Shared/Emulator.h"
#include "Shared/KeyManager.h"
#include "Shared/SystemActionManager.h"
/*#include "Zapper.h"
#include "ArkanoidController.h"
#include "OekaKidsTablet.h"
#include "FourScore.h"
#include "SnesController.h"
#include "SnesMouse.h"
#include "PowerPad.h"
#include "FamilyMatTrainer.h"
#include "KonamiHyperShot.h"
#include "FamilyBasicKeyboard.h"
#include "FamilyBasicDataRecorder.h"
#include "PartyTap.h"
#include "PachinkoController.h"
#include "ExcitingBoxingController.h"
#include "SuborKeyboard.h"
#include "SuborMouse.h"
#include "JissenMahjongController.h"
#include "BarcodeBattlerReader.h"
#include "HoriTrack.h"
#include "BandaiHyperShot.h"
#include "VsZapper.h"
#include "AsciiTurboFile.h"
#include "BattleBox.h"
#include "VbController.h"*/

NesControlManager::NesControlManager(shared_ptr<NesConsole> console, shared_ptr<BaseControlDevice> mapperControlDevice) : BaseControlManager(console->GetEmulator())
{
	_console = console;
	_mapperControlDevice = mapperControlDevice;
}

NesControlManager::~NesControlManager()
{
}

ControllerType NesControlManager::GetControllerType(uint8_t port)
{
	return _emu->GetSettings()->GetNesConfig().Controllers[port].Type;
}

shared_ptr<BaseControlDevice> NesControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;
	
	NesConfig cfg = _emu->GetSettings()->GetNesConfig();

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::NesController: device.reset(new NesController(_emu, port, cfg.Controllers[port].Keys)); break;
		//case ControllerType::Zapper: device.reset(new Zapper(console, port)); break;
		//case ControllerType::ArkanoidController: device.reset(new ArkanoidController(console, port)); break;
		case ControllerType::SnesController: device.reset(new SnesController(_emu, port, cfg.Controllers[port].Keys)); break;
		/*case ControllerType::PowerPad: device.reset(new PowerPad(console, port, console->GetSettings()->GetControllerKeys(port))); break;
		case ControllerType::SnesMouse: device.reset(new SnesMouse(console, port)); break;
		case ControllerType::SuborMouse: device.reset(new SuborMouse(console, port)); break;
		case ControllerType::VsZapper: device.reset(new VsZapper(console, port)); break;
		case ControllerType::VbController: device.reset(new VbController(console, port, console->GetSettings()->GetControllerKeys(port))); break;*/
	}
	
	return device;
}

shared_ptr<BaseControlDevice> NesControlManager::CreateExpansionDevice(ExpansionPortDevice type)
{
	shared_ptr<BaseControlDevice> device;

	/*switch(type) {
		case ExpansionPortDevice::Zapper: device.reset(new Zapper(console, BaseControlDevice::ExpDevicePort)); break;
		case ExpansionPortDevice::ArkanoidController: device.reset(new ArkanoidController(console, BaseControlDevice::ExpDevicePort)); break;
		case ExpansionPortDevice::OekaKidsTablet: device.reset(new OekaKidsTablet(console)); break;
		case ExpansionPortDevice::FamilyTrainerMat: device.reset(new FamilyMatTrainer(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::KonamiHyperShot: device.reset(new KonamiHyperShot(console, console->GetSettings()->GetControllerKeys(0), console->GetSettings()->GetControllerKeys(1))); break;
		case ExpansionPortDevice::FamilyBasicKeyboard: device.reset(new FamilyBasicKeyboard(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::PartyTap: device.reset(new PartyTap(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::Pachinko: device.reset(new PachinkoController(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::ExcitingBoxing: device.reset(new ExcitingBoxingController(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::JissenMahjong: device.reset(new JissenMahjongController(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::SuborKeyboard: device.reset(new SuborKeyboard(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::BarcodeBattler: device.reset(new BarcodeBattlerReader(console)); break;
		case ExpansionPortDevice::HoriTrack: device.reset(new HoriTrack(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::BandaiHyperShot: device.reset(new BandaiHyperShot(console, console->GetSettings()->GetControllerKeys(0))); break;
		case ExpansionPortDevice::AsciiTurboFile: device.reset(new AsciiTurboFile(console)); break;
		case ExpansionPortDevice::BattleBox: device.reset(new BattleBox(console)); break;

		case ExpansionPortDevice::FourPlayerAdapter:
		default: break;
	}*/

	return device;
}

void NesControlManager::UpdateControlDevices()
{
	auto lock = _deviceLock.AcquireSafe();
	//EmuSettings* settings = _emu->GetSettings();
	//TODO
	//Reset update flag
	//settings->NeedControllerUpdate();

	bool hadKeyboard = HasKeyboard();

	_controlDevices.clear();

	RegisterControlDevice(_emu->GetSystemActionManager());

	bool fourScore = false; //settings->CheckFlag(EmulationFlags::HasFourScore);
	//TODO
	/*ConsoleType consoleType = settings->GetConsoleType();
	ExpansionPortDevice expansionDevice = settings->GetExpansionDevice();

	if(consoleType != ConsoleType::Famicom) {
		expansionDevice = ExpansionPortDevice::None;
	} else if(expansionDevice != ExpansionPortDevice::FourPlayerAdapter) {
		fourScore = false;
	}*/

	for(int i = 0; i < (fourScore ? 4 : 2); i++) {
		shared_ptr<BaseControlDevice> device = CreateControllerDevice(GetControllerType(i), i);
		if(device) {
			RegisterControlDevice(device);
		}
	}

	//TODO
	/*if(fourScore && consoleType == ConsoleType::Nes) {
		//FourScore is only used to provide the signature for reads past the first 16 reads
		RegisterControlDevice(shared_ptr<FourScore>(new FourScore(_console)));
	}*/

	/*<BaseControlDevice> expDevice = CreateExpansionDevice(expansionDevice, _console);
	if(expDevice) {
		RegisterControlDevice(expDevice);
	}*/

	bool hasKeyboard = HasKeyboard();
	//TODO
	/*if(!hasKeyboard) {
		settings->DisableKeyboardMode();
	} else if(!hadKeyboard && hasKeyboard) {
		settings->EnableKeyboardMode();
	}*/

	if(_mapperControlDevice) {
		RegisterControlDevice(_mapperControlDevice);
	}
	//TODO
	/*
	if(std::dynamic_pointer_cast<FamilyBasicKeyboard>(expDevice)) {
		//Automatically connect the data recorder if the keyboard is connected
		RegisterControlDevice(shared_ptr<FamilyBasicDataRecorder>(new FamilyBasicDataRecorder(_console)));
	}*/
}

bool NesControlManager::HasKeyboard()
{
	return false;
	//TODO
	/*shared_ptr<BaseControlDevice> expDevice = GetControlDevice(BaseControlDevice::ExpDevicePort);
	return expDevice && expDevice->IsKeyboard();*/
}

uint8_t NesControlManager::GetOpenBusMask(uint8_t port)
{
	//"In the NES and Famicom, the top three (or five) bits are not driven, and so retain the bits of the previous byte on the bus. 
	//Usually this is the most significant byte of the address of the controller port - 0x40.
	//Paperboy relies on this behavior and requires that reads from the controller ports return exactly $40 or $41 as appropriate."

	return 0xE0;
	//TODO
	/*switch(_console->GetSettings()->GetConsoleType()) {
		default:
		case ConsoleType::Nes:
			if(_console->GetSettings()->CheckFlag(EmulationFlags::UseNes101Hvc101Behavior)) {
				return port == 0 ? 0xE4 : 0xE0;
			} else {
				return 0xE0;
			}

		case ConsoleType::Famicom:
			if(_console->GetSettings()->CheckFlag(EmulationFlags::UseNes101Hvc101Behavior)) {
				return port == 0 ? 0xF8 : 0xE0;
			} else {
				return port == 0 ? 0xF8 : 0xE0;
			}
	}*/
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
	//RemapControllerButtons();
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
		value |= device->ReadRam(addr);
	}

	return value;
}

void NesControlManager::WriteRam(uint16_t addr, uint8_t value)
{
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		device->WriteRam(addr, value);
	}
}

void NesControlManager::Reset(bool softReset)
{
	ResetLagCounter();
}

void NesControlManager::Serialize(Serializer& s)
{
	//Restore controllers that were being used at the time the snapshot was made
	//This is particularely important to ensure proper sync during NetPlay
	EmuSettings* settings = _emu->GetSettings();
	ControllerType controllerTypes[4];
	NesModel nesModel;
	ExpansionPortDevice expansionDevice;
	ConsoleType consoleType;
	bool hasFourScore = false;
	bool useNes101Hvc101Behavior = false;
	uint32_t zapperDetectionRadius = 0;
	if(s.IsSaving()) {
		//TODO
		/*nesModel = _console->GetModel();
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
	s.Stream(nesModel, expansionDevice, consoleType, types, hasFourScore, useNes101Hvc101Behavior, zapperDetectionRadius, _lagCounter, _pollCounter);

	if(!s.IsSaving()) {
		//TODO
		/*settings->SetNesModel(nesModel);
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
