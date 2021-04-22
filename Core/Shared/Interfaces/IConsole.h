#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "Core/Debugger/DebugTypes.h"
#include "Shared/RomInfo.h"

class IControlManager;
class VirtualFile;
enum class ConsoleType;
enum class CpuType : uint8_t;

struct PpuFrameInfo
{
	uint32_t Width;
	uint32_t Height;
	uint32_t FrameCount;
	uint8_t* FrameBuffer;
};

class IConsole : public ISerializable
{
public:
	virtual void Stop() = 0;
	virtual void Reset() = 0;

	virtual void OnBeforeRun() = 0;
	
	virtual bool LoadRom(VirtualFile& romFile) = 0;
	virtual void Init() = 0;

	//virtual void RunFrameWithRunAhead() = 0;
	virtual void RunFrame() = 0;

	virtual void SaveBattery() = 0;

	virtual shared_ptr<IControlManager> GetControlManager() = 0;

	virtual ConsoleType GetConsoleType() = 0;
	virtual vector<CpuType> GetCpuTypes() = 0;

	virtual uint64_t GetMasterClock() = 0;
	virtual uint32_t GetMasterClockRate() = 0;

	virtual double GetFrameDelay() = 0;
	virtual double GetFps() = 0;

	virtual void RunSingleFrame() = 0;

	virtual PpuFrameInfo GetPpuFrame() = 0;

	virtual AddressInfo GetAbsoluteAddress(AddressInfo relAddress) = 0;
	virtual AddressInfo GetRelativeAddress(AddressInfo absAddress, CpuType cpuType) = 0;
};

