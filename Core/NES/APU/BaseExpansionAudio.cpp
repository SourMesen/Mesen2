#include "pch.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"

BaseExpansionAudio::BaseExpansionAudio(NesConsole* console)
{
	_console = console;
}

void BaseExpansionAudio::Serialize(Serializer& s)
{
}

void BaseExpansionAudio::Clock()
{
	if(_console->GetApu()->IsApuEnabled()) {
		ClockAudio();
	}
}