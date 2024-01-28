#pragma once
#include "pch.h"
#include <deque>
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/RewindData.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"

class Emulator;
class EmuSettings;
struct RenderedFrame;

enum class RewindState
{
	Stopped = 0,
	Stopping = 1,
	Starting = 2,
	Started = 3,
	Debugging = 4
};

struct VideoFrame
{
	vector<uint32_t> Data;
	uint32_t Width = 0;
	uint32_t Height = 0;
	double Scale = 0;
	uint32_t FrameNumber = 0;
	vector<ControllerData> InputData;
};

struct RewindStats
{
	uint32_t MemoryUsage;
	uint32_t HistorySize;
	uint32_t HistoryDuration;
};

class RewindManager : public INotificationListener, public IInputProvider, public IInputRecorder
{
public:
	static constexpr int32_t BufferSize = 30; //Number of frames between each save state

private:
	Emulator* _emu = nullptr;
	EmuSettings* _settings = nullptr;
	
	bool _hasHistory = false;

	deque<RewindData> _history;
	deque<RewindData> _historyBackup;
	RewindData _currentHistory = {};

	RewindState _rewindState = RewindState::Stopped;
	int32_t _framesToFastForward = 0;

	deque<VideoFrame> _videoHistory;
	vector<VideoFrame> _videoHistoryBuilder;
	deque<int16_t> _audioHistory;
	vector<int16_t> _audioHistoryBuilder;

	void AddHistoryBlock();
	void PopHistory();

	void Start(bool forDebugger);
	void InternalStart(bool forDebugger);
	void Stop();
	void ForceStop(bool deleteFutureData);

	void ProcessFrame(RenderedFrame& frame, bool forRewind);
	bool ProcessAudio(int16_t* soundBuffer, uint32_t sampleCount);
	
	void ClearBuffer();

public:
	RewindManager(Emulator* emu);
	virtual ~RewindManager();

	void InitHistory();
	void Reset();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
	void ProcessEndOfFrame();

	void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) override;
	bool SetInput(BaseControlDevice *device) override;

	void StartRewinding(bool forDebugger = false);
	void StopRewinding(bool forDebugger = false, bool deleteFutureData = false);
	bool IsRewinding();
	bool IsStepBack();
	void RewindSeconds(uint32_t seconds);

	bool HasHistory();
	deque<RewindData> GetHistory();
	RewindStats GetStats();

	void SendFrame(RenderedFrame& frame, bool forRewind);
	bool SendAudio(int16_t *soundBuffer, uint32_t sampleCount);
};