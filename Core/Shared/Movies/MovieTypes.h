#pragma once
#include "pch.h"

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

namespace MovieKeys
{
	constexpr const char* MesenVersion = "MesenVersion";
	constexpr const char* MovieFormatVersion = "MovieFormatVersion";
	constexpr const char* GameFile = "GameFile";
	constexpr const char* Sha1 = "SHA1";
	constexpr const char* PatchFile = "PatchFile";
	constexpr const char* PatchFileSha1 = "PatchFileSHA1";
	constexpr const char* PatchedRomSha1 = "PatchedRomSHA1";
};