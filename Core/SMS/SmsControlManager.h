#pragma once
#include "pch.h"
#include "SMS/SmsTypes.h"
#include "Shared/SettingTypes.h"
#include "Shared/BaseControlManager.h"

class SmsConsole;
class SmsVdp;

class SmsControlManager : public BaseControlManager
{
private:
	SmsConsole* _console = nullptr;
	SmsVdp* _vdp = nullptr;
	SmsControlManagerState _state = {};
	SmsConfig _prevConfig = {};
	CvConfig _prevCvConfig = {};

	bool GetTh(bool portB);
	bool GetTr(bool portB);
	uint8_t InternalReadPort(uint8_t port);

	uint8_t ReadColecoVisionPort(uint8_t port);
	void WriteColecoVisionPort(uint8_t value);

public:
	SmsControlManager(Emulator* emu, SmsConsole* console, SmsVdp* vdp);
	shared_ptr<BaseControlDevice> CreateControllerDevice(ControllerType type, uint8_t port) override;

	SmsControlManagerState& GetState() { return _state; }

	void UpdateControlDevices() override;

	bool IsPausePressed();

	uint8_t ReadPort(uint8_t port);
	void WriteControlPort(uint8_t value);

	void Serialize(Serializer& s) override;
};