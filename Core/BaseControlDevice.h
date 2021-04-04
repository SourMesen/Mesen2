#pragma once
#include "stdafx.h"
#include "ControlDeviceState.h"
#include "SettingTypes.h"
#include "IKeyManager.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"

class Console;

class BaseControlDevice : public ISerializable
{
private:
	ControlDeviceState _state;

protected:
	Console* _console;
	vector<KeyMapping> _keyMappings;
	bool _strobe;
	uint8_t _port;
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

	void SetPressedState(uint8_t bit, uint32_t keyCode);
	void SetPressedState(uint8_t bit, bool enabled);

	void SetCoordinates(MousePosition pos);

	void SetMovement(MouseMovement mov);
	MouseMovement GetMovement();
	
	virtual void InternalSetStateFromInput();
	
public:
	static constexpr uint8_t ExpDevicePort = 4;
	static constexpr uint8_t ConsoleInputPort = 5;
	static constexpr uint8_t MapperInputPort = 6;
	static constexpr uint8_t ExpDevicePort2 = 7;
	static constexpr uint8_t PortCount = ExpDevicePort2 + 1;

	BaseControlDevice(Console* console, uint8_t port, KeyMappingSet keyMappingSet = KeyMappingSet());
	virtual ~BaseControlDevice();

	uint8_t GetPort();

	bool IsPressed(uint8_t bit);
	MousePosition GetCoordinates();
	
	void ClearState();
	void SetBit(uint8_t bit);
	void ClearBit(uint8_t bit);
	void InvertBit(uint8_t bit);
	void SetBitValue(uint8_t bit, bool set);
	
	void SetTextState(string state);
	string GetTextState();

	void SetStateFromInput();
	virtual void OnAfterSetState() { }
	
	void SetRawState(ControlDeviceState state);
	ControlDeviceState GetRawState();

	virtual ControllerType GetControllerType() = 0;
	virtual uint8_t ReadRam(uint16_t addr) = 0;
	virtual void WriteRam(uint16_t addr, uint8_t value) = 0;
	
	void static SwapButtons(shared_ptr<BaseControlDevice> state1, uint8_t button1, shared_ptr<BaseControlDevice> state2, uint8_t button2);

	void Serialize(Serializer &s) override;
};
