#pragma once
#include "pch.h"
#include "Shared/ControlDeviceState.h"
#include "Shared/SettingTypes.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class Emulator;
class InputHud;

struct DeviceButtonName
{
	string Name;
	int ButtonId = 0;
	bool IsNumeric = false;
};

class BaseControlDevice : public ISerializable
{
protected:
	ControlDeviceState _state = {};

	Emulator* _emu = nullptr;
	vector<KeyMapping> _keyMappings;
	bool _strobe = false;
	ControllerType _type = ControllerType::None;
	uint8_t _port = 0;
	bool _connected = true;
	SimpleLock _stateLock;

	virtual void RefreshStateBuffer() { }

	void EnsureCapacity(int32_t minBitCount);
	uint32_t GetByteIndex(uint8_t bit);
	virtual bool HasCoordinates();
	virtual bool IsRawString();

	bool IsCurrentPort(uint16_t addr);
	bool IsExpansionDevice();
	void StrobeProcessRead();
	void StrobeProcessWrite(uint8_t value);

	virtual string GetKeyNames() { return ""; }

	void SetPressedState(uint8_t bit, uint16_t keyCode);
	void SetPressedState(uint8_t bit, bool enabled);

	void SetMovement(MouseMovement mov);
	MouseMovement GetMovement();

	virtual void InternalSetStateFromInput();

public:
	static constexpr int DeviceXCoordButtonId = 0xFFFE;
	static constexpr int DeviceYCoordButtonId = 0xFFFF;

	static constexpr uint8_t ExpDevicePort = 4;
	static constexpr uint8_t ConsoleInputPort = 5;
	static constexpr uint8_t MapperInputPort = 6;
	static constexpr uint8_t ExpDevicePort2 = 7;
	static constexpr uint8_t PortCount = ExpDevicePort2 + 1;

	BaseControlDevice(Emulator* emu, ControllerType type, uint8_t port, KeyMappingSet keyMappingSet = KeyMappingSet());
	virtual ~BaseControlDevice();
	
	virtual void Init() {}

	uint8_t GetPort();
	ControllerType GetControllerType();

	bool IsPressed(uint8_t bit);

	MousePosition GetCoordinates();
	void SetCoordinates(MousePosition pos);

	void Connect();
	void Disconnect();
	bool IsConnected();

	void ClearState();
	void SetBit(uint8_t bit);
	void ClearBit(uint8_t bit);
	void InvertBit(uint8_t bit);
	void SetBitValue(uint8_t bit, bool set);
	
	virtual void SetTextState(string state);
	virtual string GetTextState();

	void SetStateFromInput();
	virtual void OnAfterSetState() { }
	
	virtual void SetRawState(ControlDeviceState state);
	virtual ControlDeviceState GetRawState();

	virtual void InternalDrawController(InputHud& hud) {}
	virtual void DrawController(InputHud& hud);

	virtual uint8_t ReadRam(uint16_t addr) = 0;
	virtual void WriteRam(uint16_t addr, uint8_t value) = 0;

	//Used by Lua API
	virtual vector<DeviceButtonName> GetKeyNameAssociations() { return {}; }

	virtual bool HasControllerType(ControllerType type);
	
	void static SwapButtons(shared_ptr<BaseControlDevice> state1, uint8_t button1, shared_ptr<BaseControlDevice> state2, uint8_t button2);

	void Serialize(Serializer &s) override;
};
