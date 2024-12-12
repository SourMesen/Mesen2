#pragma once
#include "pch.h"
#include "WS/WsCpu.h"
#include "WS/APU/WsApu.h"
#include "WS/WsPpu.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"

class WsConsole;
class WsTimer;
class WsControlManager;
class WsCart;
class WsSerial;
class WsDmaController;
class WsEeprom;
class Emulator;

class WsMemoryManager final : public ISerializable
{
private:
	WsConsole* _console = nullptr;
	WsCpu* _cpu = nullptr;
	WsPpu* _ppu = nullptr;
	WsApu* _apu = nullptr;
	WsControlManager* _controlManager = nullptr;
	WsCart* _cart = nullptr;
	WsTimer* _timer = nullptr;
	WsSerial* _serial = nullptr;
	WsDmaController* _dmaController = nullptr;
	WsEeprom* _eeprom = nullptr;
	Emulator* _emu = nullptr;

	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;
	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;
	uint8_t* _bootRom = nullptr;
	uint32_t _bootRomSize = 0;
	uint32_t _workRamSize = 0;

	WsMemoryManagerState _state = {};

	uint8_t* _reads[256] = {};
	uint8_t* _writes[256] = {};

	bool IsWordPort(uint16_t port);
	uint8_t GetPortWaitStates(uint16_t port);
	bool IsUnmappedPort(uint16_t port);

public:
	WsMemoryManager() {}
		
	void Init(Emulator* emu, WsConsole* console, WsCpu* cpu, WsPpu* ppu, WsControlManager* controlManager, WsCart* cart, WsTimer* timer, WsDmaController* dmaController, WsEeprom* eeprom, WsApu* apu, WsSerial* serial);
	WsMemoryManagerState& GetState() { return _state; }

	void RefreshMappings();

	void Map(uint32_t start, uint32_t end, MemoryType type, uint32_t offset, bool readonly);
	void Unmap(uint32_t start, uint32_t end);

	__forceinline void Exec()
	{
		_cpu->IncCycleCount();
		_ppu->Exec();
		_apu->Run();
	}

	__forceinline uint8_t InternalRead(uint32_t addr)
	{
		uint8_t* handler = _reads[addr >> 12];
		uint8_t value = 0x90;
		if(handler) {
			value = handler[addr & 0xFFF];
		}

		//TODOWS open bus
		return value;
	}

	__forceinline void InternalWrite(uint32_t addr, uint8_t value)
	{
		//TODOWS open bus
		uint8_t* handler = _writes[addr >> 12];
		if(handler) {
			handler[addr & 0xFFF] = value;
		}
	}

	uint8_t DebugRead(uint32_t addr);
	void DebugWrite(uint32_t addr, uint8_t value);

	template<typename T>
	T DebugCpuRead(uint16_t seg, uint16_t offset);

	template<typename T>
	__forceinline T Read(uint16_t seg, uint16_t offset, MemoryOperationType opType = MemoryOperationType::Read)
	{
		uint32_t addr = ((seg << 4) + offset) & 0xFFFFF;
		if constexpr(std::is_same<T, uint16_t>::value) {
			bool splitReads = !IsWordBus(addr) || (addr & 0x01);
			if(splitReads) {
				uint8_t lo = Read<uint8_t>(seg, offset);
				uint8_t hi = Read<uint8_t>(seg, offset + 1);
				return lo | (hi << 8);
			} else {
				Exec();
				uint8_t lo = InternalRead(addr);
				uint8_t hi = InternalRead(((seg << 4) + (uint16_t)(offset + 1)) & 0xFFFFF);
				uint16_t value = lo | (hi << 8);
				_emu->ProcessMemoryRead<CpuType::Ws, 2>(addr, value, opType);
				return value;
			}
		} else {
			Exec();
			uint8_t value = InternalRead(addr);
			_emu->ProcessMemoryRead<CpuType::Ws, 1>(addr, value, opType);
			return value;
		}
	}

	template<typename T>
	__forceinline void Write(uint16_t seg, uint16_t offset, T value, MemoryOperationType opType = MemoryOperationType::Write)
	{
		uint32_t addr = ((seg << 4) + offset) & 0xFFFFF;
		if constexpr(std::is_same<T, uint16_t>::value) {
			bool splitWrites = !IsWordBus(addr) || (addr & 0x01);
			if(splitWrites) {
				Write<uint8_t>(seg, offset, (uint8_t)value);
				Write<uint8_t>(seg, offset + 1, (uint8_t)(value >> 8));
			} else {
				Exec();
				if(_emu->ProcessMemoryWrite<CpuType::Ws, 2>(addr, value, opType)) {
					InternalWrite(addr, (uint8_t)value);
					InternalWrite(((seg << 4) + (uint16_t)(offset + 1)) & 0xFFFFF, value >> 8);
				}
			}
		} else {
			Exec();
			if(_emu->ProcessMemoryWrite<CpuType::Ws>(addr, value, opType)) {
				InternalWrite(addr, value);
			}
		}
	}

	template<typename T> T ReadPort(uint16_t port);
	template<typename T> void WritePort(uint16_t port, T value);
	uint8_t InternalReadPort(uint16_t port, bool isWordAccess);
	void InternalWritePort(uint16_t port, uint8_t value, bool isWordAccess);

	template<typename T> T DebugReadPort(uint16_t port);

	bool IsWordBus(uint32_t addr);
	uint8_t GetWaitStates(uint32_t addr);

	void SetIrqSource(WsIrqSource src);
	void ClearIrqSource(WsIrqSource src);
	uint8_t GetActiveIrqs();
	uint8_t GetIrqVector();

	__forceinline bool HasPendingIrq()
	{
		return (GetActiveIrqs() & _state.EnabledIrqs) != 0;
	}

	AddressInfo GetAbsoluteAddress(uint32_t relAddr);
	int GetRelativeAddress(AddressInfo& absAddress);

	void Serialize(Serializer& s) override;
};
