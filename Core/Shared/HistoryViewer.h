#pragma once
#include "pch.h"
#include <deque>
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/RewindData.h"

class Emulator;
class BaseControlDevice;

struct HistoryViewerState
{
	uint32_t Position = 0;
	uint32_t Length = 0;
	uint32_t Volume = 0;
	double Fps = 60.0;
	bool IsPaused = false;
	
	uint32_t SegmentCount = 0;
	uint32_t Segments[1000] = {};
};

struct HistoryViewerOptions
{
	bool IsPaused = false;
	uint32_t Volume = 100;
	uint32_t Width = 256;
	uint32_t Height = 240;
};

class HistoryViewer : public IInputProvider
{
private:
	Emulator* _emu = nullptr;
	Emulator* _mainEmu = nullptr;
	deque<RewindData> _history;
	uint32_t _position = 0;
	uint32_t _pollCounter = 0;

public:
	HistoryViewer(Emulator* emu);
	virtual ~HistoryViewer();

	bool Initialize(Emulator* mainEmu);

	void SetOptions(HistoryViewerOptions options);
	HistoryViewerState GetState();
	
	void SeekTo(uint32_t seekPosition);
	bool CreateSaveState(string outputFile, uint32_t position);
	bool SaveMovie(string movieFile, uint32_t startPosition, uint32_t endPosition);

	void ResumeGameplay(uint32_t resumePosition);

	void ProcessEndOfFrame();

	// Inherited via IInputProvider
	bool SetInput(BaseControlDevice* device) override;
};