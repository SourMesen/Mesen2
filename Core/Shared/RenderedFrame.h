#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"
#include "Shared/ControlDeviceState.h"

struct RenderedFrame
{
	void* FrameBuffer = nullptr;
	void* Data = nullptr; //Used by HD packs
	uint32_t Width = 256;
	uint32_t Height = 240;
	double Scale = 1.0;
	uint32_t FrameNumber = 0;
	uint32_t VideoPhase = 0;
	vector<ControllerData> InputData;

	RenderedFrame()
	{}

	RenderedFrame(void* buffer, uint32_t width, uint32_t height, double scale = 1.0, uint32_t frameNumber = 0) :
		FrameBuffer(buffer),
		Data(nullptr),
		Width(width),
		Height(height),
		Scale(scale),
		FrameNumber(frameNumber),
		InputData({})
	{}

	RenderedFrame(void* buffer, uint32_t width, uint32_t height, double scale, uint32_t frameNumber, vector<ControllerData> inputData, uint32_t videoPhase = 0) :
		FrameBuffer(buffer),
		Data(nullptr),
		Width(width),
		Height(height),
		Scale(scale),
		FrameNumber(frameNumber),
		VideoPhase(videoPhase),
		InputData(inputData)
	{}
};
