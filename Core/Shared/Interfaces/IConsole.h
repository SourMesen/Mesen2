#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Core/Debugger/DebugTypes.h"
#include "Shared/Audio/AudioPlayerTypes.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/RomInfo.h"
#include "Shared/TimingInfo.h"
#include "Shared/SaveStateCompatInfo.h"

class BaseControlManager;
class VirtualFile;
class BaseVideoFilter;
struct BaseState;
struct InternalCheatCode;
enum class ConsoleType;
enum class ConsoleRegion;
enum class CpuType : uint8_t;
enum class EmulatorShortcut;
enum class HashType;

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
	uint32_t FrameBufferSize;
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

class IConsole : public ISerializable, public INotificationListener
{
public:
	virtual ~IConsole() {}

	virtual void Reset() = 0;

	virtual LoadRomResult LoadRom(VirtualFile& romFile) = 0;

	virtual void RunFrame() = 0;

	virtual void SaveBattery() = 0;

	virtual ShortcutState IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam) { return ShortcutState::Default; }

	virtual BaseControlManager* GetControlManager() = 0;

	virtual DipSwitchInfo GetDipSwitchInfo() { return {}; }
	virtual ConsoleRegion GetRegion() = 0;
	virtual ConsoleType GetConsoleType() = 0;
	virtual vector<CpuType> GetCpuTypes() = 0;

	virtual uint64_t GetMasterClock() = 0;
	virtual uint32_t GetMasterClockRate() = 0;

	virtual double GetFps() = 0;

	virtual TimingInfo GetTimingInfo(CpuType cpuType)
	{
		TimingInfo info = {};
		info.MasterClock = GetMasterClock();
		info.MasterClockRate = GetMasterClockRate();
		info.Fps = GetFps();

		PpuFrameInfo frameInfo = GetPpuFrame();
		info.FrameCount = frameInfo.FrameCount;
		info.CycleCount = frameInfo.CycleCount;
		info.ScanlineCount = frameInfo.ScanlineCount;
		info.FirstScanline = frameInfo.FirstScanline;
		return info;
	}

	virtual BaseVideoFilter* GetVideoFilter(bool getDefaultFilter) = 0;
	virtual void GetScreenRotationOverride(uint32_t& rotation) {}

	virtual PpuFrameInfo GetPpuFrame() = 0;
	
	virtual string GetHash(HashType hashType) { return {}; }

	virtual RomFormat GetRomFormat() = 0;
	virtual AudioTrackInfo GetAudioTrackInfo() = 0;
	virtual void ProcessAudioPlayerAction(AudioPlayerActionParams p) = 0;

	virtual AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) = 0;
	virtual AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) = 0;
	virtual void GetConsoleState(BaseState& state, ConsoleType consoleType) = 0;
	
	virtual SaveStateCompatInfo ValidateSaveStateCompatibility(ConsoleType stateConsoleType) { return {}; }

	virtual void ProcessCheatCode(InternalCheatCode& code, uint32_t addr, uint8_t& value) {}

	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) {}
};

