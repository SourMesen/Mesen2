#include "pch.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "NES/Mappers/VsSystem/VsSystem.h"
#include "NES/Mappers/VsSystem/VsInputButtons.h"
#include "NES/Input/VsZapper.h"
#include "NES/Input/NesController.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "Shared/NotificationManager.h"

VsControlManager::VsControlManager(NesConsole* console) : NesControlManager(console)
{
	bool isVsDualSystem = _console->GetVsMainConsole() || _console->GetVsSubConsole();

	_input.reset(new VsInputButtons(_emu, isVsDualSystem));

	if(_console->IsVsMainConsole()) {
		_emu->GetNotificationManager()->RegisterNotificationListener(_input);
	} else 	{
		//Remove SystemActionManager (reset/power buttons) from sub console
		_systemDevices.clear();
	}

	AddSystemControlDevice(_input);
}

VsControlManager::~VsControlManager()
{
	UnregisterInputProvider(this);
}

void VsControlManager::Reset(bool softReset)
{
	NesControlManager::Reset(softReset);
	_protectionCounter = 0;

	//Unsure about this, needed for VS Wrecking Crew
	UpdateMainSubBit(_console->IsVsMainConsole() ? 0x00 : 0x02);

	_vsSystemType = _console->GetMapper()->GetRomInfo().VsType;

	if(!softReset && !_console->IsVsMainConsole()) {
		UnregisterInputProvider(this);
		RegisterInputProvider(this);
	}
}

void VsControlManager::Serialize(Serializer& s)
{
	NesControlManager::Serialize(s);
	SV(_prgChrSelectBit); SV(_protectionCounter); SV(_refreshState);
}

void VsControlManager::GetMemoryRanges(MemoryRanges &ranges)
{
	NesControlManager::GetMemoryRanges(ranges);
	ranges.AddHandler(MemoryOperation::Read, 0x4020, 0x5FFF);
	ranges.AddHandler(MemoryOperation::Write, 0x4020, 0x5FFF);
}

uint8_t VsControlManager::GetPrgChrSelectBit()
{
	return _prgChrSelectBit;
}

void VsControlManager::RemapControllerButtons()
{
	shared_ptr<NesController> controllers[2];
	controllers[0] = std::dynamic_pointer_cast<NesController>(GetControlDevice(0));
	controllers[1] = std::dynamic_pointer_cast<NesController>(GetControlDevice(1));

	if(!controllers[0] || !controllers[1]) {
		return;
	}

	GameInputType inputType = _console->GetMapper()->GetRomInfo().InputType;
	if(inputType == GameInputType::VsSystemSwapped) {
		//Swap controllers 1 & 2
		ControlDeviceState port1State = controllers[0]->GetRawState();
		ControlDeviceState port2State = controllers[1]->GetRawState();
		controllers[0]->SetRawState(port2State);
		controllers[1]->SetRawState(port1State);

		//But don't swap the start/select buttons
		BaseControlDevice::SwapButtons(controllers[0], NesController::Buttons::Start, controllers[1], NesController::Buttons::Start);
		BaseControlDevice::SwapButtons(controllers[0], NesController::Buttons::Select, controllers[1], NesController::Buttons::Select);
	} else if(inputType == GameInputType::VsSystemSwapAB) {
		//Swap buttons P1 A & P2 B (Pinball (Japan))
		BaseControlDevice::SwapButtons(controllers[0], NesController::Buttons::B, controllers[1], NesController::Buttons::A);
	}

	//Swap Start/Select for all configurations (makes it more intuitive)
	BaseControlDevice::SwapButtons(controllers[0], NesController::Buttons::Start, controllers[0], NesController::Buttons::Select);
	BaseControlDevice::SwapButtons(controllers[1], NesController::Buttons::Start, controllers[1], NesController::Buttons::Select);

	if(_vsSystemType == VsSystemType::RaidOnBungelingBayProtection || _vsSystemType == VsSystemType::IceClimberProtection) {
		//Bit 3 of the input status must always be on
		controllers[0]->SetBit(NesController::Buttons::Start);
		controllers[1]->SetBit(NesController::Buttons::Start);
	}
}

uint8_t VsControlManager::GetOpenBusMask(uint8_t port)
{
	return 0x00;
}

uint8_t VsControlManager::ReadRam(uint16_t addr)
{
	uint8_t value = 0;

	if(!_console->IsVsMainConsole()) {
		//Copy the insert coin 3/4 + service button "2" bits from the main console to this one
		NesConsole* mainConsole = _console->GetVsMainConsole();
		VsInputButtons* mainButtons = ((VsControlManager*)mainConsole->GetControlManager())->_input.get();
		_input->SetBitValue(VsInputButtons::VsButtons::InsertCoin1, mainButtons->IsPressed(VsInputButtons::VsButtons::InsertCoin3));
		_input->SetBitValue(VsInputButtons::VsButtons::InsertCoin2, mainButtons->IsPressed(VsInputButtons::VsButtons::InsertCoin4));
		_input->SetBitValue(VsInputButtons::VsButtons::ServiceButton, mainButtons->IsPressed(VsInputButtons::VsButtons::ServiceButton2));
	}

	switch(addr) {
		case 0x4016: {
			uint32_t dipSwitches = _emu->GetSettings()->GetGameConfig().DipSwitches;
			if(!_console->IsVsMainConsole()) {
				dipSwitches >>= 8;
			}

			value = NesControlManager::ReadRam(addr) & 0x65;
			value |= ((dipSwitches & 0x01) ? 0x08 : 0x00);
			value |= ((dipSwitches & 0x02) ? 0x10 : 0x00);
			value |= (_console->IsVsMainConsole() ? 0x00 : 0x80);
			break;
		}

		case 0x4017: {
			value = NesControlManager::ReadRam(addr) & 0x01;

			uint32_t dipSwitches = _emu->GetSettings()->GetGameConfig().DipSwitches;
			if(!_console->IsVsMainConsole()) {
				dipSwitches >>= 8;
			}
			value |= ((dipSwitches & 0x04) ? 0x04 : 0x00);
			value |= ((dipSwitches & 0x08) ? 0x08 : 0x00);
			value |= ((dipSwitches & 0x10) ? 0x10 : 0x00);
			value |= ((dipSwitches & 0x20) ? 0x20 : 0x00);
			value |= ((dipSwitches & 0x40) ? 0x40 : 0x00);
			value |= ((dipSwitches & 0x80) ? 0x80 : 0x00);
			break;
		}

		case 0x5E00:
			_protectionCounter = 0;
			break;

		case 0x5E01:
			if(_vsSystemType == VsSystemType::TkoBoxingProtection) {
				value = _protectionData[0][_protectionCounter++ & 0x1F];
			} else if(_vsSystemType == VsSystemType::RbiBaseballProtection) {
				value = _protectionData[1][_protectionCounter++ & 0x1F];
			}
			break;

		default:
			if(_vsSystemType == VsSystemType::SuperXeviousProtection) {
				return _protectionData[2][_protectionCounter++ & 0x1F];
			}
			break;
	}

	return value;
}

void VsControlManager::WriteRam(uint16_t addr, uint8_t value)
{
	NesControlManager::WriteRam(addr, value);

	_refreshState = (value & 0x01) == 0x01;

	if(addr == 0x4016) {
		_prgChrSelectBit = (value >> 2) & 0x01;
		
		//Bit 2: DualSystem-only
		uint8_t mainSubBit = (value & 0x02);
		if(mainSubBit != _mainSubBit) {
			UpdateMainSubBit(mainSubBit);
		}
	}
}

void VsControlManager::UpdateMainSubBit(uint8_t mainSubBit)
{
	_mainSubBit = mainSubBit;

	NesConsole* otherConsole = _console->GetVsMainConsole() ? _console->GetVsMainConsole() : _console->GetVsSubConsole();
	if(otherConsole) {
		if(_console->IsVsMainConsole()) {
			UpdateMemoryAccess();
		}

		NesCpu* mainCpu = otherConsole->GetCpu(); //Will be null when running this after loading the ROM
		if(mainCpu) {
			if(mainSubBit) {
				mainCpu->ClearIrqSource(IRQSource::External);
			} else {
				//When low, asserts /IRQ on the other CPU
				otherConsole->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}
}

void VsControlManager::UpdateMemoryAccess()
{
	NesConsole* subConsole = _console->GetVsSubConsole();
	if(subConsole) {
		BaseMapper* mainMapper = _console->GetMapper();
		BaseMapper* subMapper = subConsole->GetMapper();
		mainMapper->SwapMemoryAccess(subMapper, _mainSubBit ? true : false);
	}
}

void VsControlManager::UpdateControlDevices()
{
	if(_console->GetVsMainConsole() || _console->GetVsSubConsole()) {
		auto lock = _deviceLock.AcquireSafe();
		ClearDevices();

		if(_console->IsVsMainConsole()) {
			//Force 4x standard controllers for VS Dualsystem main system
			for(int i = 0; i < 4; i++) {
				shared_ptr<BaseControlDevice> device = CreateControllerDevice(ControllerType::NesController, i);
				if(device) {
					if(i >= 2) {
						//Don't send input to main console for P3 & P4
						//P3 & P4 will be sent to the sub console - see SetInput() below.
						device->Disconnect();
					}
					RegisterControlDevice(device);
				}
			}
		} else {
			//2x standard controllers for sub system
			for(int i = 0; i < 2; i++) {
				shared_ptr<BaseControlDevice> device = CreateControllerDevice(ControllerType::NesController, i);
				if(device) {
					RegisterControlDevice(device);
				}
			}
		}
	} else {
		NesControlManager::UpdateControlDevices();
	}
}

bool VsControlManager::SetInput(BaseControlDevice* device)
{
	uint8_t port = device->GetPort();
	NesControlManager* mainControlManager = (NesControlManager*)_console->GetVsMainConsole()->GetControlManager();
	if(mainControlManager && port <= 1) {
		shared_ptr<BaseControlDevice> controlDevice = mainControlManager->GetControlDevice(port + 2);
		if(controlDevice) {
			ControlDeviceState state = controlDevice->GetRawState();
			device->SetRawState(state);
		}
	}
	return true;
}
