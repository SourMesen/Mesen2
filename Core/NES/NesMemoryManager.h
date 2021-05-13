#pragma once

#include "../stdafx.h"
#include "../../Utilities/ISerializable.h"

#include "INesMemoryHandler.h"
#include "OpenBusHandler.h"
#include "InternalRamHandler.h"
#include "MemoryOperationType.h"

class BaseMapper;
class CheatManager;
class Emulator;
class NesConsole;

class NesMemoryManager : public ISerializable
{
	private:
		static constexpr int RAMSize = 0x10000;
		static constexpr int VRAMSize = 0x4000;
		
		Emulator* _emu = nullptr;
		shared_ptr<CheatManager> _cheatManager;
		NesConsole* _console;
		BaseMapper* _mapper;

		uint8_t *_internalRAM;

		OpenBusHandler _openBusHandler;
		InternalRamHandler<0x7FF> _internalRamHandler;
		INesMemoryHandler** _ramReadHandlers;
		INesMemoryHandler** _ramWriteHandlers;

		void InitializeMemoryHandlers(INesMemoryHandler** memoryHandlers, INesMemoryHandler* handler, vector<uint16_t> *addresses, bool allowOverride);

	protected:
		void Serialize(Serializer &s) override;

	public:
		static const int InternalRAMSize = 0x800;

		NesMemoryManager(NesConsole* console);
		virtual ~NesMemoryManager();

		void SetMapper(BaseMapper* mapper);
		
		void Reset(bool softReset);
		void RegisterIODevice(INesMemoryHandler* handler);
		void RegisterWriteHandler(INesMemoryHandler* handler, uint32_t start, uint32_t end);
		void UnregisterIODevice(INesMemoryHandler* handler);

		uint8_t DebugRead(uint16_t addr, bool disableSideEffects = true);
		uint16_t DebugReadWord(uint16_t addr);
		void DebugWrite(uint16_t addr, uint8_t value, bool disableSideEffects = true);

		uint8_t* GetInternalRAM();

		uint8_t Read(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read);
		void Write(uint16_t addr, uint8_t value, MemoryOperationType operationType);

		uint8_t GetOpenBus(uint8_t mask = 0xFF);
};

