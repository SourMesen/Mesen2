#pragma once
#include "stdafx.h"
#include <deque>
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/RewindData.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"

class Emulator;
class EmuSettings;

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
	uint32_t Width;
	uint32_t Height;
};

class RewindManager : public INotificationListener, public IInputProvider, public IInputRecorder
{
private:
	static constexpr int32_t BufferSize = 60; //Number of frames between each save state

	shared_ptr<Emulator> _emu;
	shared_ptr<EmuSettings> _settings;
	
	bool _hasHistory;

	std::deque<RewindData> _history;
	std::deque<RewindData> _historyBackup;
	RewindData _currentHistory;

	RewindState _rewindState;
	int32_t _framesToFastForward;

	std::deque<VideoFrame> _videoHistory;
	vector<VideoFrame> _videoHistoryBuilder;
	std::deque<int16_t> _audioHistory;
	vector<int16_t> _audioHistoryBuilder;

	void AddHistoryBlock();
	void PopHistory();

	void Start(bool forDebugger);
	void Stop();
	void ForceStop();

	void ProcessFrame(void *frameBuffer, uint32_t width, uint32_t height, bool forRewind);
	bool ProcessAudio(int16_t *soundBuffer, uint32_t sampleCount);
	
	void ClearBuffer();

public:
	RewindManager(shared_ptr<Emulator> emu);
	virtual ~RewindManager();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
	void ProcessEndOfFrame();

	void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) override;
	bool SetInput(BaseControlDevice *device) override;

	void StartRewinding(bool forDebugger = false);
	void StopRewinding(bool forDebugger = false);
	bool IsRewinding();
	bool IsStepBack();
	void RewindSeconds(uint32_t seconds);

	bool HasHistory();

	void SendFrame(void *frameBuffer, uint32_t width, uint32_t height, bool forRewind);
	bool SendAudio(int16_t *soundBuffer, uint32_t sampleCount);
};