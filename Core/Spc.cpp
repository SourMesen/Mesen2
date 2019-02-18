#include "stdafx.h"
#include "Spc.h"
#include "SNES_SPC.h"
#include "Console.h"
#include "MemoryManager.h"
#include "SoundMixer.h"
#include "../Utilities/VirtualFile.h"

uint64_t _startFrameMasterClock = 0;

Spc::Spc(shared_ptr<Console> console)
{
	_console = console;

	vector<uint8_t> fileData;
	VirtualFile spcBios("spc700.rom");
	spcBios.ReadFile(fileData);
	memcpy(_spcBios, fileData.data(), 64);

	_soundBuffer = new int16_t[0x50000];

	_spc = new SNES_SPC();
	_spc->init();
	_spc->init_rom(_spcBios);
	_spc->reset();
	_spc->set_output(_soundBuffer, 0x50000);

	_startFrameMasterClock = 0;
}

Spc::~Spc()
{
	delete _spc;
}

uint8_t Spc::Read(uint16_t addr)
{
	uint64_t currentClock = _console->GetMemoryManager()->GetMasterClock();
	uint64_t elapsedClocks = currentClock - _startFrameMasterClock;
	uint64_t frameTime = elapsedClocks * 1024000 / 2 / 21477000;
	return _spc->read_port(frameTime, addr & 0x03);
}

void Spc::Write(uint32_t addr, uint8_t value)
{
	uint64_t currentClock = _console->GetMemoryManager()->GetMasterClock();
	uint64_t elapsedClocks = currentClock - _startFrameMasterClock;
	uint64_t frameTime = elapsedClocks * 1024000 / 2 / 21477000;
	_spc->write_port(frameTime, addr & 0x03, value);
}

void Spc::ProcessEndFrame()
{
	_spc->end_frame(1024000 / 2);
	int sampleCount = _spc->sample_count();
	_console->GetSoundMixer()->PlayAudioBuffer(_soundBuffer, sampleCount);
	_spc->set_output(_soundBuffer, 0x50000);

	_startFrameMasterClock = _console->GetMemoryManager()->GetMasterClock();
}
