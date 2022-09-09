#pragma once
#include "pch.h"
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
	virtual ~BaseExpansionAudio() = default;

	void Clock();
};