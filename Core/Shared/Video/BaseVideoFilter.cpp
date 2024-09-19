#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/Video/RotateFilter.h"
#include "Shared/Video/ScaleFilter.h"
#include "Shared/Video/ScanlineFilter.h"
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
	FrameInfo frameInfo = _baseFrameInfo;
	OverscanDimensions overscan = GetOverscan();
	frameInfo.Width -= overscan.Left + overscan.Right;
	frameInfo.Height -= overscan.Top + overscan.Bottom;
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

void BaseVideoFilter::SetOverscan(OverscanDimensions overscan)
{
	_overscan = overscan;
}

void BaseVideoFilter::OnBeforeApplyFilter()
{
}

bool BaseVideoFilter::IsOddFrame()
{
	return _isOddFrame;
}

uint32_t BaseVideoFilter::GetVideoPhase()
{
	return _videoPhase;
}

uint32_t BaseVideoFilter::GetBufferSize()
{
	return _bufferSize * sizeof(uint32_t);
}

FrameInfo BaseVideoFilter::GetFrameInfo(uint16_t* ppuOutputBuffer, bool enableOverscan)
{
	_overscan = enableOverscan ? _emu->GetSettings()->GetOverscan() : OverscanDimensions {};
	_ppuOutputBuffer = ppuOutputBuffer;
	OnBeforeApplyFilter();
	FrameInfo frameInfo = GetFrameInfo();
	_frameInfo = frameInfo;
	_ppuOutputBuffer = nullptr;
	return frameInfo;
}

FrameInfo BaseVideoFilter::SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber, uint32_t videoPhase, void* frameData, bool enableOverscan)
{
	auto lock = _frameLock.AcquireSafe();
	_overscan = enableOverscan ? _emu->GetSettings()->GetOverscan() : OverscanDimensions{};
	_isOddFrame = frameNumber % 2;
	_videoPhase = videoPhase;
	_frameData = frameData;
	_ppuOutputBuffer = ppuOutputBuffer;
	OnBeforeApplyFilter();
	FrameInfo frameInfo = GetFrameInfo();
	_frameInfo = frameInfo;
	UpdateBufferSize();
	ApplyFilter(ppuOutputBuffer);
	_ppuOutputBuffer = nullptr;
	return frameInfo;
}

uint32_t* BaseVideoFilter::GetOutputBuffer()
{
	return _outputBuffer;
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

void BaseVideoFilter::ApplyColorOptions(uint8_t& r, uint8_t& g, uint8_t& b, double brightness, double contrast)
{
	double redChannel = r / 255.0;
	double greenChannel = g / 255.0;
	double blueChannel = b / 255.0;

	//Apply brightness, contrast, hue & saturation
	double y, i, q;
	RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
	y *= contrast * 0.5f + 1;
	y += brightness * 0.5f;
	YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

	r = (uint8_t)std::min(255, (int)std::round(redChannel * 255));
	g = (uint8_t)std::min(255, (int)std::round(greenChannel * 255));
	b = (uint8_t)std::min(255, (int)std::round(blueChannel * 255));
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
	
	uint8_t scale = 1;

	uint32_t screenRotation = _emu->GetSettings()->GetVideoConfig().ScreenRotation;
	_emu->GetScreenRotationOverride(screenRotation);

	unique_ptr<RotateFilter> rotateFilter(new RotateFilter(screenRotation));
	if(screenRotation != 0) {
		pngBuffer = rotateFilter->ApplyFilter(pngBuffer, frameInfo.Width, frameInfo.Height);
		frameInfo = rotateFilter->GetFrameInfo(frameInfo);
	}

	unique_ptr<ScaleFilter> scaleFilter = ScaleFilter::GetScaleFilter(_emu, filterType);
	if(scaleFilter) {
		pngBuffer = scaleFilter->ApplyFilter(pngBuffer, frameInfo.Width, frameInfo.Height);
		frameInfo = scaleFilter->GetFrameInfo(frameInfo);
		scale = scaleFilter->GetScale();
	}

	ScanlineFilter::ApplyFilter(pngBuffer, frameInfo.Width, frameInfo.Height, _emu->GetSettings()->GetVideoConfig().ScanlineIntensity, scale);
	
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
