#pragma once
#include "pch.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsVdp.h"
#include "SMS/Carts/SmsCart.h"
#include "Shared/Emulator.h"
#include "Shared/CheatManager.h"
#include "Debugger/AddressInfo.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class Emulator;
class SmsConsole;
class SmsControlManager;
class SmsCart;
class SmsPsg;
class SmsFmAudio;
class SmsBiosMapper;

class SmsMemoryManager final : public ISerializable
{
private:
	static constexpr uint32_t SmsWorkRamSize = 0x2000;
	static constexpr uint32_t CartRamMaxSize = 0x8000;
	static constexpr uint32_t CvWorkRamSize = 0x400;

	Emulator* _emu = nullptr;
	SmsConsole* _console = nullptr;
	SmsVdp* _vdp = nullptr;
	SmsControlManager* _controlManager = nullptr;
	SmsCart* _cart = nullptr;
	SmsPsg* _psg = nullptr;
	SmsFmAudio* _fmAudio = nullptr;
	unique_ptr<SmsBiosMapper> _biosMapper;

	SmsMemoryManagerState _state = {};

	uint8_t* _workRam = nullptr;
	uint32_t _workRamSize = 0;

	uint8_t* _cartRam = nullptr;
	uint32_t _cartRamSize = 0;
	uint8_t* _originalCartRam = nullptr;

	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;

	uint8_t* _biosRom = nullptr;
	uint32_t _biosRomSize = 0;

	uint64_t _masterClock = 0;

	uint8_t* _reads[0x100] = {};
	uint8_t* _writes[0x100] = {};

	int32_t _sgRamMapAddress = -1;
	SmsModel _model = {};

	void LoadBattery();

	template<bool isPeek = false> __forceinline uint8_t InternalReadPort(uint8_t port);
	template<bool isPeek> uint8_t ReadSmsPort(uint8_t port);
	template<bool isPeek> uint8_t ReadColecoVisionPort(uint8_t port);
	template<bool isPeek> uint8_t ReadGameGearPort(uint8_t port);
	void WriteGameGearPort(uint8_t port, uint8_t value);
	void WriteSmsPort(uint8_t port, uint8_t value);
	void WriteColecoVisionPort(uint8_t port, uint8_t value);
	
	uint32_t DetectSgCartRam(vector<uint8_t>& romData);

public:
	void Init(Emulator* emu, SmsConsole* console, vector<uint8_t>& romData, vector<uint8_t>& biosRom, SmsVdp* vdp, SmsControlManager* controlManager, SmsCart* cart, SmsPsg* psg, SmsFmAudio* fmAudio);
	SmsMemoryManager();
	~SmsMemoryManager();

	SmsMemoryManagerState& GetState()
	{
		return _state;
	}

	__forceinline void Exec(uint8_t clocks)
	{
		_masterClock += clocks;
		_vdp->Run(_masterClock);
	}

	void RefreshMappings();
	bool HasBios();
	void SaveBattery();

	AddressInfo GetAbsoluteAddress(uint16_t addr);
	int32_t GetRelativeAddress(AddressInfo& absAddress);

	void Map(uint16_t start, uint16_t end, MemoryType type, uint32_t offset, bool readonly);
	void Unmap(uint16_t start, uint16_t end);
	void MapRegisters(uint16_t start, uint16_t end, SmsRegisterAccess access);

	uint8_t GetOpenBus();

	__forceinline uint8_t Read(uint16_t addr, MemoryOperationType opType)
	{
		uint8_t value;
		if(_state.IsReadRegister[addr >> 8]) {
			value = _cart->ReadRegister(addr);
		} else if(_reads[addr >> 8]) {
			value = _reads[addr >> 8][(uint8_t)addr];
		} else {
			value = GetOpenBus();
		}
		if(_emu->GetCheatManager()->HasCheats<CpuType::Sms>()) {
			_emu->GetCheatManager()->ApplyCheat<CpuType::Sms>(addr, value);
		}
		_state.OpenBus = value;
		_emu->ProcessMemoryRead<CpuType::Sms>(addr, value, opType);
		return value;
	}

	uint8_t DebugRead(uint16_t addr);

	void Write(uint16_t addr, uint8_t value);
	void DebugWrite(uint16_t addr, uint8_t value);
	
	uint8_t DebugReadPort(uint8_t port);
	uint8_t ReadPort(uint8_t port);
	void WritePort(uint8_t port, uint8_t value);

	void Serialize(Serializer& s) override;
};
