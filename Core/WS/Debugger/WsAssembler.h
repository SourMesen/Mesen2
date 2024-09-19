#pragma once
#include "pch.h"
#include "Debugger/IAssembler.h"

class LabelManager;

class WsAssembler final : public IAssembler
{
private:
	LabelManager* _labelManager;

public:
	WsAssembler(LabelManager* labelManager)
	{
		_labelManager = labelManager;
	}

	virtual ~WsAssembler() {}

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode)
	{
		//TODOWS
		return 0;
	}
};