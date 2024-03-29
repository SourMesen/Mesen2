#pragma once
#include "pch.h"
#include "NES/NesControlManager.h"
#include "NES/NesConsole.h"
#include "Shared/Interfaces/IInputProvider.h"

class BaseControlDevice;
class VsInputButtons;

class VsControlManager : public NesControlManager, public IInputProvider
{
private:
	shared_ptr<VsInputButtons> _input;
	uint8_t _prgChrSelectBit = 0;
	uint8_t _mainSubBit = 0;

	bool _refreshState = false;

	VsSystemType _vsSystemType = VsSystemType::Default;

	uint32_t _protectionCounter = 0;
	const uint32_t _protectionData[3][32] = { 
		{
			0xFF, 0xBF, 0xB7, 0x97, 0x97, 0x17, 0x57, 0x4F,
			0x6F, 0x6B, 0xEB, 0xA9, 0xB1, 0x90, 0x94, 0x14,
			0x56, 0x4E, 0x6F, 0x6B, 0xEB, 0xA9, 0xB1, 0x90,
			0xD4, 0x5C, 0x3E, 0x26, 0x87, 0x83, 0x13, 0x00
		},
		{
			0x00, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00,
			0x00, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x94, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		{
			0x05, 0x01, 0x89, 0x37, 0x05, 0x00, 0xD1, 0x3E,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		}
	};

protected:
	void RemapControllerButtons() override;
	uint8_t GetOpenBusMask(uint8_t port) override;

	void UpdateMainSubBit(uint8_t mainSubBit);
	void UpdateMemoryAccess();

public:
	VsControlManager(NesConsole* console);
	~VsControlManager();

	void Serialize(Serializer& s) override;
	void Reset(bool softReset) override;

	void GetMemoryRanges(MemoryRanges &ranges) override;

	uint8_t GetPrgChrSelectBit();

	void UpdateControlDevices() override;

	uint8_t ReadRam(uint16_t addr) override;	
	void WriteRam(uint16_t addr, uint8_t value) override;

	// Inherited via IInputProvider
	virtual bool SetInput(BaseControlDevice* device) override;
};