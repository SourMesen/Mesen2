#pragma once
#include "pch.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/ISerializable.h"

class GbCart : public ISerializable
{
protected:
	Gameboy* _gameboy = nullptr;
	GbMemoryManager* _memoryManager = nullptr;
	uint8_t* _cartRam = nullptr;
	
	void Map(uint16_t start, uint16_t end, GbMemoryType type, uint32_t offset, bool readonly)
	{
		_memoryManager->Map(start, end, type, offset, readonly);
	}

	void Unmap(uint16_t start, uint16_t end)
	{
		_memoryManager->Unmap(start, end);
	}

public:
	virtual ~GbCart()
	{
	}

	void Init(Gameboy* gameboy, GbMemoryManager* memoryManager)
	{
		_gameboy = gameboy;
		_memoryManager = memoryManager;
		_cartRam = gameboy->DebugGetMemory(MemoryType::GbCartRam);
	}

	virtual void InitCart()
	{
	}

	virtual void RefreshMappings()
	{
		Map(0x0000, 0x7FFF, GbMemoryType::PrgRom, 0, true);
	}

	virtual uint8_t ReadRegister(uint16_t addr)
	{
		LogDebug("[Debug] GB - Missing read handler: $" + HexUtilities::ToHex(addr));
		return 0;
	}

	virtual void WriteRegister(uint16_t addr, uint8_t value)
	{
	}

	void Serialize(Serializer& s) override
	{
	}
};
