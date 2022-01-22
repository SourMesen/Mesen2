#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/SimpleLock.h"

enum class DebugEventType;
struct DebugEventInfo;
struct EventViewerDisplayOptions;
class SnesCpu;
class SnesPpu;
class Debugger;
class SnesDmaController;
class SnesMemoryManager;

struct SnesEventViewerConfig : public BaseEventViewerConfig
{
	EventViewerCategoryCfg Irq;
	EventViewerCategoryCfg Nmi;
	EventViewerCategoryCfg MarkedBreakpoints;

	EventViewerCategoryCfg PpuRegisterReads;
	EventViewerCategoryCfg PpuRegisterCgramWrites;
	EventViewerCategoryCfg PpuRegisterVramWrites;
	EventViewerCategoryCfg PpuRegisterOamWrites;
	EventViewerCategoryCfg PpuRegisterMode7Writes;
	EventViewerCategoryCfg PpuRegisterBgOptionWrites;
	EventViewerCategoryCfg PpuRegisterBgScrollWrites;
	EventViewerCategoryCfg PpuRegisterWindowWrites;
	EventViewerCategoryCfg PpuRegisterOtherWrites;

	EventViewerCategoryCfg ApuRegisterReads;
	EventViewerCategoryCfg ApuRegisterWrites;
	EventViewerCategoryCfg CpuRegisterReads;
	EventViewerCategoryCfg CpuRegisterWrites;
	EventViewerCategoryCfg WorkRamRegisterReads;
	EventViewerCategoryCfg WorkRamRegisterWrites;

	bool ShowPreviousFrameEvents;
	uint8_t ShowDmaChannels[8];
};

class SnesEventManager final : public BaseEventManager
{
private:
	static constexpr int ScanlineWidth = 1364 / 2;

	SnesEventViewerConfig _config;

	SnesCpu * _cpu;
	SnesPpu *_ppu;
	SnesMemoryManager* _memoryManager;
	SnesDmaController *_dmaController;
	Debugger *_debugger;

	bool _overscanMode = false;
	bool _useHighResOutput = false;
	uint32_t _scanlineCount = 262;
	uint16_t *_ppuBuffer = nullptr;

	void DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer);

protected:
	bool ShowPreviousFrameEvents() override;

public:
	SnesEventManager(Debugger *debugger, SnesCpu *cpu, SnesPpu *ppu, SnesMemoryManager *memoryManager, SnesDmaController *dmaController);
	~SnesEventManager();

	void AddEvent(DebugEventType type, MemoryOperationInfo &operation, int32_t breakpointId = -1);
	void AddEvent(DebugEventType type);
	
	EventViewerCategoryCfg GetEventConfig(DebugEventInfo& evt);

	uint32_t TakeEventSnapshot();
	FrameInfo GetDisplayBufferSize();
	void GetDisplayBuffer(uint32_t *buffer, uint32_t bufferSize);
	DebugEventInfo GetEvent(uint16_t y, uint16_t x);

	void SetConfiguration(BaseEventViewerConfig& config) override;
};
