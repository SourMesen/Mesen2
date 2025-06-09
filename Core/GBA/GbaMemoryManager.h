#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaTimer.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaWaitStates.h"
#include "GBA/GbaRomPrefetch.h"
#include "Debugger/AddressInfo.h"
#include "Utilities/ISerializable.h"

class Emulator;
class GbaConsole;
class GbaPpu;
class GbaDmaController;
class GbaControlManager;
class GbaTimer;
class GbaApu;
class GbaCart;
class GbaSerial;
class MgbaLogHandler;

struct GbaPendingIrq
{
	GbaIrqSource Source;
	uint8_t Delay;
};

class GbaMemoryManager final : public ISerializable
{
private:
	Emulator* _emu = nullptr;
	GbaConsole* _console = nullptr;
	GbaPpu* _ppu = nullptr;
	GbaDmaController* _dmaController = nullptr;
	GbaControlManager* _controlManager = nullptr;
	GbaTimer* _timer;
	GbaApu* _apu;
	GbaCart* _cart;
	GbaSerial* _serial;
	GbaRomPrefetch* _prefetch;
	GbaWaitStates _waitStates;

	unique_ptr<MgbaLogHandler> _mgbaLog;

	uint64_t _masterClock = 0;
	bool _hasPendingUpdates = false;
	bool _hasPendingLateUpdates = false;
	GbaMemoryManagerState _state = {};

	uint32_t _prgRomSize = 0;

	uint8_t* _prgRom = nullptr;
	uint8_t* _bootRom = nullptr;
	uint8_t* _intWorkRam = nullptr;
	uint8_t* _extWorkRam = nullptr;
	uint8_t* _vram = nullptr;
	uint8_t* _oam = nullptr;
	uint8_t* _palette = nullptr;

	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0;
	
	vector<GbaPendingIrq> _pendingIrqs;
	
	bool _haltModeUsed = false;
	bool _biosLocked = false;
	uint8_t _haltDelay = 0;
	uint8_t _irqFirstAccessCycle = 0;

	uint8_t _dmaIrqCounter = 0;
	uint16_t _dmaIrqPending = 0;
	uint16_t _dmaIrqLine = 0;

	uint8_t _objEnableDelay = 0;

	__forceinline void ProcessWaitStates(GbaAccessModeVal mode, uint32_t addr);

	__noinline void ProcessVramAccess(GbaAccessModeVal mode, uint32_t addr);
	__noinline void ProcessVramStalling(uint8_t memType);

	template<uint8_t width>
	void UpdateOpenBus(uint32_t addr, uint32_t value);

	template<bool debug = false>
	uint32_t RotateValue(GbaAccessModeVal mode, uint32_t addr, uint32_t value, bool isSigned);

	__forceinline uint8_t InternalRead(GbaAccessModeVal mode, uint32_t addr, uint32_t readAddr);
	__forceinline void InternalWrite(GbaAccessModeVal mode, uint32_t addr, uint8_t value, uint32_t writeAddr, uint32_t fullValue);

	uint32_t ReadRegister(uint32_t addr);
	void WriteRegister(GbaAccessModeVal mode, uint32_t addr, uint8_t value);

	void TriggerIrqUpdate();
	__noinline void ProcessPendingUpdates(bool allowStartDma);
	__noinline void ProcessPendingLateUpdates();

	void ProcessParallelIdleCycle();
	__forceinline void RunPrefetch();

public:
	GbaMemoryManager(Emulator* emu, GbaConsole* console, GbaPpu* ppu, GbaDmaController* dmaController, GbaControlManager* controlManager, GbaTimer* timer, GbaApu* apu, GbaCart* cart, GbaSerial* serial, GbaRomPrefetch* prefetch);
	~GbaMemoryManager();

	GbaMemoryManagerState& GetState() { return _state; }
	uint64_t GetMasterClock() { return _masterClock; }

	GbaWaitStates* GetWaitStates() { return &_waitStates; }

	__forceinline void ProcessIdleCycle()
	{
		if(_dmaController->HasPendingDma()) {
			_dmaController->RunPendingDma(true);
		}

		if(_dmaController->CanRunInParallelWithDma()) {
			//When DMA is running, CPU idle cycles (e.g from MUL or other instructions) can run in parallel
			//with the DMA. The CPU only stops once it tries to read or write to the bus.
			//This allows this idle cycle to run in "parallel" with the DMA
			ProcessParallelIdleCycle();
			return;
		}

		if(_prefetch->NeedExec(_state.PrefetchEnabled)) {
			_prefetch->Exec(1, _state.PrefetchEnabled);
		}

		ProcessInternalCycle<true>();
	}

	template<bool firstAccessCycle = false>
	__forceinline void ProcessInternalCycle()
	{
		if(_hasPendingUpdates) {
			ProcessPendingUpdates(firstAccessCycle);
		} else {
			_masterClock++;
			_ppu->Exec();
			_timer->Exec(_masterClock);
		}

		if constexpr(firstAccessCycle) {
			//The CPU appears to check the IRQ line on the first cycle in each read/write access
			//So a 4-cycle read to ROM will check the IRQ line's state after the first of these
			//4 cycles and this will determine whether or not the CPU runs an extra instruction
			//before processing the IRQ or not.
			//This is needed to pass the Internal_Cycle_DMA_IRQ test
			_irqFirstAccessCycle = _state.IrqLine;
		}

		if(_hasPendingLateUpdates) {
			ProcessPendingLateUpdates();
		}
	}
	
	void ProcessDmaStart();

	__forceinline void ProcessDma()
	{
		if(_dmaController->HasPendingDma()) {
			_dmaController->RunPendingDma(true);
		}
	}

	void TriggerObjEnableUpdate();

	void ProcessStoppedCycle();

	void LockBus() { _state.BusLocked = true; }
	void UnlockBus() { _state.BusLocked = false; }
	bool IsBusLocked() { return _state.BusLocked; }

	bool IsSystemStopped() { return _state.StopMode; }
	bool UseInlineHalt();

	void SetPendingUpdateFlag() { _hasPendingUpdates = true; }
	void SetPendingLateUpdateFlag() { _hasPendingLateUpdates = true; }

	uint32_t Read(GbaAccessModeVal mode, uint32_t addr);
	void Write(GbaAccessModeVal mode, uint32_t addr, uint32_t value);

	void SetDelayedIrqSource(GbaIrqSource source, uint8_t delay);
	void SetIrqSource(GbaIrqSource source);
	bool HasPendingIrq();
	bool IsHaltOver();

	uint8_t GetOpenBus(uint32_t addr);

	uint32_t DebugCpuRead(GbaAccessModeVal mode, uint32_t addr);
	uint8_t DebugRead(uint32_t addr);
	void DebugWrite(uint32_t addr, uint8_t value);

	AddressInfo GetAbsoluteAddress(uint32_t addr);
	int64_t GetRelativeAddress(AddressInfo& absAddress);

	void Serialize(Serializer& s) override;
};