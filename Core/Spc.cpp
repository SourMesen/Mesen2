#include "stdafx.h"
#include "Spc.h"
#include "SNES_SPC.h"
#include "Console.h"
#include "MemoryManager.h"
#include "SoundMixer.h"

Spc::Spc(shared_ptr<Console> console, vector<uint8_t> &spcRomData)
{
	_console = console;
	
	memcpy(_spcBios, spcRomData.data(), 64);

	_soundBuffer = new int16_t[Spc::SampleBufferSize];

	_spc = new SNES_SPC();
	_spc->init();
	_spc->init_rom(_spcBios);
	_spc->reset();
	_spc->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);

	_startFrameMasterClock = 0;
}

Spc::~Spc()
{
	delete _spc;
}

int Spc::GetSpcTime()
{
	uint64_t currentClock = _console->GetMemoryManager()->GetMasterClock();
	uint64_t elapsedClocks = currentClock - _startFrameMasterClock;
	return (int)(elapsedClocks * 1024000 / 21477000);
}

uint8_t Spc::Read(uint16_t addr)
{
	return _spc->read_port(GetSpcTime(), addr & 0x03);
}

void Spc::Write(uint32_t addr, uint8_t value)
{
	_spc->write_port(GetSpcTime(), addr & 0x03, value);
}

void Spc::ProcessEndFrame()
{
	_spc->end_frame(GetSpcTime());

	int sampleCount = _spc->sample_count();
	_console->GetSoundMixer()->PlayAudioBuffer(_soundBuffer, sampleCount / 2);
	_spc->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);

	uint64_t remainder = (_console->GetMemoryManager()->GetMasterClock() - _startFrameMasterClock) * 1024000 % 21477000 / 1024000;
	_startFrameMasterClock = _console->GetMemoryManager()->GetMasterClock() - remainder;
}
