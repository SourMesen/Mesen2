#include "stdafx.h"
#include "DebugStats.h"
#include "Console.h"
#include "SoundMixer.h"
#include "Ppu.h"
#include "EmuSettings.h"
#include "DebugHud.h"
#include "IAudioDevice.h"

DebugStats::DebugStats(Console * console)
{
	_console = console;
}

void DebugStats::DisplayStats(double lastFrameTime)
{
	AudioStatistics stats = _console->GetSoundMixer()->GetStatistics();
	AudioConfig audioCfg = _console->GetSettings()->GetAudioConfig();
	shared_ptr<DebugHud> hud = _console->GetDebugHud();

	_frameDurations[_frameDurationIndex] = lastFrameTime;
	_frameDurationIndex = (_frameDurationIndex + 1) % 60;

	int startFrame = _console->GetPpu()->GetFrameCount();

	hud->DrawRectangle(8, 8, 115, 49, 0x40000000, true, 1, startFrame);
	hud->DrawRectangle(8, 8, 115, 49, 0xFFFFFF, false, 1, startFrame);

	hud->DrawString(10, 10, "Audio Stats", 0xFFFFFF, 0xFF000000, 1, startFrame);
	hud->DrawString(10, 21, "Latency: ", 0xFFFFFF, 0xFF000000, 1, startFrame);

	int color = (stats.AverageLatency > 0 && std::abs(stats.AverageLatency - audioCfg.AudioLatency) > 3) ? 0xFF0000 : 0xFFFFFF;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << stats.AverageLatency << " ms";
	hud->DrawString(54, 21, ss.str(), color, 0xFF000000, 1, startFrame);

	hud->DrawString(10, 30, "Underruns: " + std::to_string(stats.BufferUnderrunEventCount), 0xFFFFFF, 0xFF000000, 1, startFrame);
	hud->DrawString(10, 39, "Buffer Size: " + std::to_string(stats.BufferSize / 1024) + "kb", 0xFFFFFF, 0xFF000000, 1, startFrame);
	hud->DrawString(10, 48, "Rate: " + std::to_string((uint32_t)(audioCfg.SampleRate *  _console->GetSoundMixer()->GetRateAdjustment())) + "Hz", 0xFFFFFF, 0xFF000000, 1, startFrame);

	hud->DrawRectangle(132, 8, 115, 49, 0x40000000, true, 1, startFrame);
	hud->DrawRectangle(132, 8, 115, 49, 0xFFFFFF, false, 1, startFrame);
	hud->DrawString(134, 10, "Video Stats", 0xFFFFFF, 0xFF000000, 1, startFrame);

	double totalDuration = 0;
	if(_frameDurations) {
		for(int i = 0; i < 60; i++) {
			totalDuration += _frameDurations[i];
		}
	}

	ss = std::stringstream();
	ss << "FPS: " << std::fixed << std::setprecision(4) << (1000 / (totalDuration / 60));
	hud->DrawString(134, 21, ss.str(), 0xFFFFFF, 0xFF000000, 1, startFrame);

	ss = std::stringstream();
	ss << "Last Frame: " << std::fixed << std::setprecision(2) << lastFrameTime << " ms";
	hud->DrawString(134, 30, ss.str(), 0xFFFFFF, 0xFF000000, 1, startFrame);

	if(_console->GetPpu()->GetFrameCount() > 60) {
		_lastFrameMin = std::min(lastFrameTime, _lastFrameMin);
		_lastFrameMax = std::max(lastFrameTime, _lastFrameMax);
	} else {
		_lastFrameMin = 9999;
		_lastFrameMax = 0;
	}

	ss = std::stringstream();
	ss << "Min Delay: " << std::fixed << std::setprecision(2) << ((_lastFrameMin < 9999) ? _lastFrameMin : 0.0) << " ms";
	hud->DrawString(134, 39, ss.str(), 0xFFFFFF, 0xFF000000, 1, startFrame);

	ss = std::stringstream();
	ss << "Max Delay: " << std::fixed << std::setprecision(2) << _lastFrameMax << " ms";
	hud->DrawString(134, 48, ss.str(), 0xFFFFFF, 0xFF000000, 1, startFrame);
}
