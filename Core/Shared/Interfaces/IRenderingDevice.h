#pragma once

#include "stdafx.h"

class IRenderingDevice
{
	public:
		virtual ~IRenderingDevice() {}
		virtual void UpdateFrame(uint32_t* frameBuffer, uint32_t width, uint32_t height) = 0;
		virtual void Render(uint32_t* hudBuffer, uint32_t width, uint32_t height) = 0;
		virtual void Reset() = 0;
		virtual void SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight) = 0;
};