#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"

class NesConsole;

class BaseExpansionAudio : public ISerializable
{
protected: 
	NesConsole* _console = nullptr;

	virtual void ClockAudio() = 0;
	void Serialize(Serializer& s) override;

public:
	BaseExpansionAudio(NesConsole* console);

	void Clock();
};