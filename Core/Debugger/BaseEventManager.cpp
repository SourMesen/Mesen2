#include "stdafx.h"
#include "BaseEventManager.h"

void BaseEventManager::FilterEvents()
{
	auto lock = _lock.AcquireSafe();
	_sentEvents.clear();

	if(ShowPreviousFrameEvents() && _snapshotScanline != 0 && _snapshotCycle != 0) {
		int offset = GetScanlineOffset();
		uint32_t key = (_snapshotScanline << 16) + _snapshotCycle;
		for(DebugEventInfo& evt : _snapshotPrevFrame) {
			uint32_t evtKey = ((evt.Scanline + offset) << 16) + evt.Cycle;
			if(evtKey > key) {
				_sentEvents.push_back(evt);
				_sentEvents.back().Flags |= (uint32_t)EventFlags::PreviousFrame;
			}
		}
	}

	for(DebugEventInfo& evt : _snapshotCurrentFrame) {
		EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
		if(eventCfg.Visible) {
			_sentEvents.push_back(evt);
		}
	}
}

void BaseEventManager::DrawDot(uint32_t x, uint32_t y, uint32_t color, bool drawBackground, uint32_t* buffer)
{
	if(drawBackground) {
		color = 0xFF000000 | ((color >> 1) & 0x7F7F7F);
	} else {
		color |= 0xFF000000;
	}

	int iMin = drawBackground ? -2 : 0;
	int iMax = drawBackground ? 3 : 1;
	int jMin = drawBackground ? -2 : 0;
	int jMax = drawBackground ? 3 : 1;

	FrameInfo size = GetDisplayBufferSize();

	for(int i = iMin; i <= iMax; i++) {
		for(int j = jMin; j <= jMax; j++) {
			if(j + x >= size.Width) {
				continue;
			}

			int32_t pos = (y + i) * size.Width + x + j;
			if(pos < 0 || pos >= (int)(size.Width * size.Height)) {
				continue;
			}
			buffer[pos] = color;
		}
	}
}

void BaseEventManager::GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount)
{
	auto lock = _lock.AcquireSafe();
	uint32_t eventCount = std::min(maxEventCount, (uint32_t)_sentEvents.size());
	memcpy(eventArray, _sentEvents.data(), eventCount * sizeof(DebugEventInfo));
	maxEventCount = eventCount;
}

uint32_t BaseEventManager::GetEventCount()
{
	auto lock = _lock.AcquireSafe();
	FilterEvents();
	return (uint32_t)_sentEvents.size();
}

void BaseEventManager::ClearFrameEvents()
{
	_prevDebugEvents = _debugEvents;
	_debugEvents.clear();
}
