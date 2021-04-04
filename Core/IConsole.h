#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "SNES/CartTypes.h"

class ControlManager;
class VirtualFile;

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
	
	virtual bool LoadRom(VirtualFile& romFile, VirtualFile& patchFile) = 0;
	virtual void Init() = 0;

	//virtual void RunFrameWithRunAhead() = 0;
	virtual void RunFrame() = 0;

	virtual shared_ptr<ControlManager> GetControlManager() = 0;

	virtual double GetFrameDelay() = 0;
	virtual double GetFps() = 0;

	virtual RomInfo GetRomInfo() = 0;

	virtual void RunSingleFrame() = 0;

	virtual PpuFrameInfo GetPpuFrame() = 0;
};

