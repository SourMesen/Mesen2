#include "stdafx.h"
#include "BaseVideoFilter.h"
#include "MessageManager.h"
#include "ScaleFilter.h"
#include "EmuSettings.h"
#include "../Utilities/PNGHelper.h"
#include "../Utilities/FolderUtilities.h"
#include "Console.h"

BaseVideoFilter::BaseVideoFilter(shared_ptr<Console> console)
{
	_console = console;
	_overscan = _console->GetSettings()->GetOverscan();
}

BaseVideoFilter::~BaseVideoFilter()
{
	auto lock = _frameLock.AcquireSafe();
	delete[] _outputBuffer;
}

void BaseVideoFilter::SetBaseFrameInfo(FrameInfo frameInfo)
{
	_baseFrameInfo = frameInfo;
}

FrameInfo BaseVideoFilter::GetFrameInfo()
{
	int overscanMultiplier = _baseFrameInfo.Width == 512 ? 2 : 1;
	FrameInfo frameInfo = _baseFrameInfo;
	OverscanDimensions overscan = GetOverscan();
	frameInfo.Width -= overscan.Left * overscanMultiplier + overscan.Right * overscanMultiplier;
	frameInfo.Height -= overscan.Top * overscanMultiplier + overscan.Bottom * overscanMultiplier;
	return frameInfo;
}

void BaseVideoFilter::UpdateBufferSize()
{
	uint32_t newBufferSize = GetFrameInfo().Width*GetFrameInfo().Height;
	if(_bufferSize != newBufferSize) {
		_frameLock.Acquire();
		delete[] _outputBuffer;
		_bufferSize = newBufferSize;
		_outputBuffer = new uint32_t[newBufferSize];
		_frameLock.Release();
	}
}

OverscanDimensions BaseVideoFilter::GetOverscan()
{
	return _overscan;
}

void BaseVideoFilter::OnBeforeApplyFilter()
{
}

bool BaseVideoFilter::IsOddFrame()
{
	return _isOddFrame;
}

uint32_t BaseVideoFilter::GetBufferSize()
{
	return _bufferSize * sizeof(uint32_t);
}

void BaseVideoFilter::SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber)
{
	_frameLock.Acquire();
	_overscan = _console->GetSettings()->GetOverscan();
	_isOddFrame = frameNumber % 2;
	UpdateBufferSize();
	OnBeforeApplyFilter();
	ApplyFilter(ppuOutputBuffer);

	_frameLock.Release();
}

uint32_t* BaseVideoFilter::GetOutputBuffer()
{
	return _outputBuffer;
}

uint32_t BaseVideoFilter::ApplyScanlineEffect(uint32_t argb, uint8_t scanlineIntensity)
{
	uint8_t r = ((argb & 0xFF0000) >> 16) * scanlineIntensity / 255;
	uint8_t g = ((argb & 0xFF00) >> 8) * scanlineIntensity / 255;
	uint8_t b = (argb & 0xFF) * scanlineIntensity / 255;

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void BaseVideoFilter::TakeScreenshot(VideoFilterType filterType, string filename, std::stringstream *stream)
{
	uint32_t* pngBuffer;
	FrameInfo frameInfo;
	uint32_t* frameBuffer = nullptr;
	{
		auto lock = _frameLock.AcquireSafe();
		if(_bufferSize == 0 || !GetOutputBuffer()) {
			return;
		}

		frameBuffer = new uint32_t[_bufferSize];
		memcpy(frameBuffer, GetOutputBuffer(), _bufferSize * sizeof(frameBuffer[0]));
		frameInfo = GetFrameInfo();
	}

	pngBuffer = frameBuffer;

	shared_ptr<ScaleFilter> scaleFilter = ScaleFilter::GetScaleFilter(filterType);
	if(scaleFilter) {
		pngBuffer = scaleFilter->ApplyFilter(pngBuffer, frameInfo.Width, frameInfo.Height, _console->GetSettings()->GetVideoConfig().ScanlineIntensity);
		frameInfo = scaleFilter->GetFrameInfo(frameInfo);
	}
	
	if(!filename.empty()) {
		PNGHelper::WritePNG(filename, pngBuffer, frameInfo.Width, frameInfo.Height);
	} else {
		PNGHelper::WritePNG(*stream, pngBuffer, frameInfo.Width, frameInfo.Height);
	}

	delete[] frameBuffer;
}

void BaseVideoFilter::TakeScreenshot(string romName, VideoFilterType filterType)
{
	string romFilename = FolderUtilities::GetFilename(romName, false);

	int counter = 0;
	string baseFilename = FolderUtilities::CombinePath(FolderUtilities::GetScreenshotFolder(), romFilename);
	string ssFilename;
	while(true) {
		string counterStr = std::to_string(counter);
		while(counterStr.length() < 3) {
			counterStr = "0" + counterStr;
		}
		ssFilename = baseFilename + "_" + counterStr + ".png";
		ifstream file(ssFilename, ios::in);
		if(file) {
			file.close();
		} else {
			break;
		}
		counter++;
	}

	TakeScreenshot(filterType, ssFilename);

	MessageManager::DisplayMessage("ScreenshotSaved", FolderUtilities::GetFilename(ssFilename, true));
}

