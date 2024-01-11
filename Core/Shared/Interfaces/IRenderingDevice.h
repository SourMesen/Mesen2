#pragma once

#include "pch.h"
#include "Shared/SettingTypes.h"

struct RenderedFrame;

struct RenderSurfaceInfo
{
	uint32_t* Buffer = nullptr;
	uint32_t Width = 0;
	uint32_t Height = 0;
	bool IsDirty = true;

	bool UpdateSize(uint32_t width, uint32_t height)
	{
		if(Width != width || Height != height) {
			delete[] Buffer;
			Buffer = new uint32_t[height * width];
			Width = width;
			Height = height;
			Clear();
			return true;
		}
		return false;
	}

	void Clear()
	{
		memset(Buffer, 0, Width * Height * sizeof(uint32_t));
		IsDirty = true;
	}

	~RenderSurfaceInfo()
	{
		delete[] Buffer;
	}
};

class IRenderingDevice
{
	public:
		virtual ~IRenderingDevice() {}
		virtual void UpdateFrame(RenderedFrame& frame) = 0;
		virtual void ClearFrame() = 0;
		virtual void Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud) = 0;
		virtual void Reset() = 0;
		virtual void OnRendererThreadStarted() {}
		virtual void SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle) = 0;
};