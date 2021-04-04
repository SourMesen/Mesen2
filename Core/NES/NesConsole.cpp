#include "stdafx.h"
#include "NesConsole.h"
#include "Emulator.h"
#include "NesControlManager.h"
#include "IControlManager.h"

NesConsole::NesConsole(Emulator* emu)
{
	_emu = emu;
}

Emulator* NesConsole::GetEmulator()
{
	return _emu;
}

void NesConsole::Serialize(Serializer& s)
{
}

void NesConsole::Stop()
{
}

void NesConsole::Reset()
{
}

void NesConsole::OnBeforeRun()
{
}

bool NesConsole::LoadRom(VirtualFile& romFile, VirtualFile& patchFile)
{
    return false;
}

void NesConsole::Init()
{
}

void NesConsole::RunFrame()
{
}

shared_ptr<IControlManager> NesConsole::GetControlManager()
{
	return std::dynamic_pointer_cast<IControlManager>(_controlManager);
}

double NesConsole::GetFrameDelay()
{
    return 0.0;
}

double NesConsole::GetFps()
{
    return 0.0;
}

RomInfo NesConsole::GetRomInfo()
{
    return RomInfo();
}

void NesConsole::RunSingleFrame()
{
}

PpuFrameInfo NesConsole::GetPpuFrame()
{
    return PpuFrameInfo();
}
