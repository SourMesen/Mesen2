#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
class GbaCpu;
class GbaPpu;
class GbaMemoryManager;
class GbaDmaController;
class Debugger;

struct GbaEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg PaletteReads;
	EventViewerCategoryCfg PaletteWrites;
	EventViewerCategoryCfg VramReads;
	EventViewerCategoryCfg VramWrites;
	EventViewerCategoryCfg OamReads;
	EventViewerCategoryCfg OamWrites;

	EventViewerCategoryCfg PpuRegisterBgScrollReads;
	EventViewerCategoryCfg PpuRegisterBgScrollWrites;
	EventViewerCategoryCfg PpuRegisterWindowReads;
	EventViewerCategoryCfg PpuRegisterWindowWrites;
	EventViewerCategoryCfg PpuRegisterOtherReads;
	EventViewerCategoryCfg PpuRegisterOtherWrites;

	EventViewerCategoryCfg DmaRegisterReads;
	EventViewerCategoryCfg DmaRegisterWrites;

	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;

	EventViewerCategoryCfg SerialReads;
	EventViewerCategoryCfg SerialWrites;

	EventViewerCategoryCfg TimerReads;
	EventViewerCategoryCfg TimerWrites;

	EventViewerCategoryCfg InputReads;
	EventViewerCategoryCfg InputWrites;

	EventViewerCategoryCfg OtherRegisterReads;
	EventViewerCategoryCfg OtherRegisterWrites;

	bool ShowPreviousFrameEvents;
};

class GbaEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = 308*4;
	static constexpr int ScreenHeight = 228;
	static constexpr int VBlankScanline = 160;

	GbaEventViewerConfig _config = {};

	GbaPpu* _ppu = nullptr;
	GbaCpu* _cpu = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;
	GbaDmaController* _dmaController = nullptr;
	Debugger* _debugger = nullptr;

	uint32_t _scanlineCount = GbaEventManager::ScreenHeight;
	uint16_t* _ppuBuffer = nullptr;

protected:
	bool ShowPreviousFrameEvents() override;
	void ConvertScanlineCycleToRowColumn(int32_t& x, int32_t& y) override;
	void DrawScreen(uint32_t* buffer) override;

public:
	GbaEventManager(Debugger* debugger, GbaCpu* cpu, GbaPpu* ppu, GbaMemoryManager* memoryManager, GbaDmaController* dmaController);
	~GbaEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) override;
	void AddEvent(DebugEventType type) override;

	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt) override;

	uint32_t TakeEventSnapshot(bool forAutoRefresh) override;
	DebugEventInfo GetEvent(uint16_t y, uint16_t x) override;

	FrameInfo GetDisplayBufferSize() override;
	void SetConfiguration(BaseEventViewerConfig& config) override;
};
