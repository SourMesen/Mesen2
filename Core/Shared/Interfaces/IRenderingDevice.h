#pragma once

#include "stdafx.h"
#include "Shared/SettingTypes.h"

struct RenderedFrame;

class IRenderingDevice
{
	public:
		virtual ~IRenderingDevice() {}
		virtual void UpdateFrame(RenderedFrame& frame) = 0;
		virtual void ClearFrame() = 0;
		virtual void Render(uint32_t* hudBuffer, uint32_t width, uint32_t height) = 0;
		virtual void Reset() = 0;
		virtual void SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle) = 0;
};