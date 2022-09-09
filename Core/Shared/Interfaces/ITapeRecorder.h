#pragma once
#include "pch.h"

enum class TapeRecorderAction
{
	Play,
	StartRecord,
	StopRecord
};

class ITapeRecorder
{
public:
	virtual void ProcessTapeRecorderAction(TapeRecorderAction action, string filename) = 0;
	virtual bool IsRecording() = 0;
};
