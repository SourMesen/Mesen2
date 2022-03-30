#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "Core/Debugger/DebugTypes.h"
#include "Shared/Audio/AudioPlayerTypes.h"
#include "Shared/RomInfo.h"

class BaseControlManager;
class VirtualFile;
class BaseVideoFilter;
struct BaseState;
struct InternalCheatCode;
enum class ConsoleType;
enum class ConsoleRegion;
enum class CpuType : uint8_t;

enum class LoadRomResult
{
	Success,
	Failure,
	UnknownType
};

struct PpuFrameInfo
{
	uint8_t* FrameBuffer;
	uint32_t Width;
	uint32_t Height;
	uint32_t FrameCount;
	uint32_t ScanlineCount;
	int32_t FirstScanline;
	uint32_t CycleCount;
};

enum class ShortcutState
{
	Disabled = 0,
	Enabled = 1,
	Default = 2
};

class IConsole : public ISerializable
{
public:
	virtual ~IConsole() {}

	virtual void Stop() = 0;
	virtual void Reset() = 0;

	virtual void OnBeforeRun() = 0;
	
	virtual LoadRomResult LoadRom(VirtualFile& romFile) = 0;
	virtual void Init() = 0;

	//virtual void RunFrameWithRunAhead() = 0;
	virtual void RunFrame() = 0;

	virtual void SaveBattery() = 0;

	virtual ShortcutState IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam) { return ShortcutState::Default; }

	virtual BaseControlManager* GetControlManager() = 0;

	virtual ConsoleRegion GetRegion() = 0;
	virtual ConsoleType GetConsoleType() = 0;
	virtual vector<CpuType> GetCpuTypes() = 0;

	virtual uint64_t GetMasterClock() = 0;
	virtual uint32_t GetMasterClockRate() = 0;

	virtual double GetFps() = 0;

	virtual BaseVideoFilter* GetVideoFilter() = 0;

	virtual PpuFrameInfo GetPpuFrame() = 0;

	virtual RomFormat GetRomFormat() = 0;
	virtual AudioTrackInfo GetAudioTrackInfo() = 0;
	virtual void ProcessAudioPlayerAction(AudioPlayerActionParams p) = 0;

	virtual AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) = 0;
	virtual AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) = 0;
	virtual void GetConsoleState(BaseState& state, ConsoleType consoleType) = 0;

	virtual void ProcessCheatCode(InternalCheatCode& code, uint32_t addr, uint8_t& value) {}
};

