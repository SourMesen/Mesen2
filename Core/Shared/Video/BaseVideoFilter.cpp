#include "stdafx.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/Video/ScaleFilter.h"
#include "Utilities/PNGHelper.h"
#include "Utilities/FolderUtilities.h"

const static double PI = 3.14159265358979323846;

BaseVideoFilter::BaseVideoFilter(Emulator* emu)
{
	_emu = emu;
	_overscan = _emu->GetSettings()->GetOverscan();
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
	uint32_t newBufferSize = _frameInfo.Width*_frameInfo.Height;
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

FrameInfo BaseVideoFilter::SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber, void* frameData)
{
	auto lock = _frameLock.AcquireSafe();
	_overscan = _emu->GetSettings()->GetOverscan();
	_isOddFrame = frameNumber % 2;
	_frameData = frameData;
	FrameInfo frameInfo = GetFrameInfo();
	_frameInfo = frameInfo;
	UpdateBufferSize();
	OnBeforeApplyFilter();
	ApplyFilter(ppuOutputBuffer);
	return frameInfo;
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

void BaseVideoFilter::InitConversionMatrix(double hueShift, double saturationShift)
{
	double hue = hueShift * PI;
	double sat = saturationShift + 1;

	double baseValues[6] = { 0.956f, 0.621f, -0.272f, -0.647f, -1.105f, 1.702f };

	double s = sin(hue) * sat;
	double c = cos(hue) * sat;

	double* output = _yiqToRgbMatrix;
	double* input = baseValues;
	for(int n = 0; n < 3; n++) {
		double i = *input++;
		double q = *input++;
		*output++ = i * c - q * s;
		*output++ = i * s + q * c;
	}
}

void BaseVideoFilter::RgbToYiq(double r, double g, double b, double& y, double& i, double& q)
{
	y = r * 0.299f + g * 0.587f + b * 0.114f;
	i = r * 0.596f - g * 0.275f - b * 0.321f;
	q = r * 0.212f - g * 0.523f + b * 0.311f;
}

void BaseVideoFilter::YiqToRgb(double y, double i, double q, double& r, double& g, double& b)
{
	r = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[0] * i + _yiqToRgbMatrix[1] * q)));
	g = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[2] * i + _yiqToRgbMatrix[3] * q)));
	b = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[4] * i + _yiqToRgbMatrix[5] * q)));
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
		frameInfo = _frameInfo;
	}

	pngBuffer = frameBuffer;

	unique_ptr<ScaleFilter> scaleFilter = ScaleFilter::GetScaleFilter(filterType);
	if(scaleFilter) {
		pngBuffer = scaleFilter->ApplyFilter(pngBuffer, frameInfo.Width, frameInfo.Height, _emu->GetSettings()->GetVideoConfig().ScanlineIntensity);
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

