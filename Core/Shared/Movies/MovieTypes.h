#pragma once
#include "stdafx.h"

enum class RecordMovieFrom
{
	StartWithoutSaveData = 0,
	StartWithSaveData,
	CurrentState
};

struct RecordMovieOptions
{
	char Filename[2000] = {};
	char Author[250] = {};
	char Description[10000] = {};

	RecordMovieFrom RecordFrom = RecordMovieFrom::StartWithoutSaveData;
};

const vector<string> ConsoleRegionNames = {
	"Auto",
	"NTSC",
	"PAL"
};

//TODO
const vector<string> ControllerTypeNames = {
	"None",
	"SnesController",
	"SnesMouse",
	"SuperScope",
	"Multitap",
	"NesController"
};

const vector<string> RamStateNames = {
	"AllZeros",
	"AllOnes",
	"Random"
};

namespace MovieKeys
{
	constexpr const char* MesenVersion = "MesenVersion";
	constexpr const char* MovieFormatVersion = "MovieFormatVersion";
	constexpr const char* GameFile = "GameFile";
	constexpr const char* Sha1 = "SHA1";
	constexpr const char* PatchFile = "PatchFile";
	constexpr const char* PatchFileSha1 = "PatchFileSHA1";
	constexpr const char* PatchedRomSha1 = "PatchedRomSHA1";
	constexpr const char* Region = "Region";
	constexpr const char* ConsoleType = "ConsoleType";
	constexpr const char* Controller1 = "Controller1";
	constexpr const char* Controller2 = "Controller2";
	constexpr const char* Controller3 = "Controller3";
	constexpr const char* Controller4 = "Controller4";
	constexpr const char* Controller5 = "Controller5";
	constexpr const char* ExtraScanlinesBeforeNmi = "ExtraScanlinesBeforeNmi";
	constexpr const char* ExtraScanlinesAfterNmi = "ExtraScanlinesAfterNmi";
	constexpr const char* RamPowerOnState = "RamPowerOnState";
	constexpr const char* InputPollScanline = "InputPollScanline";
	constexpr const char* GsuClockSpeed = "GsuClockSpeed";
};