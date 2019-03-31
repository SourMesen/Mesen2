//--------------------------------------------------------------------------------------
// This a heavily modified version of SpriteFont.h from DirectX Toolkit (MIT)
// It strips down a lot of options not needed for Mesen and implements the minimum
// required to use .spritefont files in SDL. 
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// File: SpriteFont.h
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <string>
using std::string;

struct RECT
{
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
};

struct XMFLOAT2
{
    float x = 0.0f;
    float y = 0.0f;

    XMFLOAT2() {}
    XMFLOAT2(float _x, float _y) : x(_x), y(_y) {}
    explicit XMFLOAT2(const float *pArray) : x(pArray[0]), y(pArray[1]) {}

    XMFLOAT2& operator= (const XMFLOAT2& Float2) { x = Float2.x; y = Float2.y; return *this; }
};

class SpriteFont
{
public:
	struct Glyph;

	SpriteFont(SDL_Renderer* renderer, string fileName);

	SpriteFont(SpriteFont&& moveFrom);
	SpriteFont& operator= (SpriteFont&& moveFrom);

	SpriteFont(SpriteFont const&) = delete;
	SpriteFont& operator= (SpriteFont const&) = delete;

	virtual ~SpriteFont();

	void DrawString(SDL_Renderer *renderer, wchar_t const* text, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255) const;

	XMFLOAT2 MeasureString(wchar_t const* text) const;

	// Spacing properties
	float GetLineSpacing() const;
	void SetLineSpacing(float spacing);

	// Font properties
	wchar_t GetDefaultCharacter() const;
	void SetDefaultCharacter(wchar_t character);

	bool ContainsCharacter(wchar_t character) const;

	// Custom layout/rendering
	Glyph const* FindGlyph(wchar_t character) const;

	// Describes a single character glyph.
	struct Glyph
	{
		uint32_t Character;
		RECT Subrect;
		float XOffset;
		float YOffset;
		float XAdvance;
	};


private:
	// Private implementation.
	class Impl;

	std::unique_ptr<Impl> pImpl;

	static const XMFLOAT2 Float2Zero;
};

class BinaryReader
{
public:
	BinaryReader(string fileName) : mPos(nullptr), mEnd(nullptr)
	{
		size_t dataSize;

		bool result = ReadEntireFile(fileName, mOwnedData, &dataSize);
		if(!result) {
			throw std::runtime_error( "BinaryReader" );
		}

		mPos = mOwnedData.get();
		mEnd = mOwnedData.get() + dataSize;
	}
	
	// Reads a single value.
	template<typename T> T const& Read()
	{
		return *ReadArray<T>(1);
	}


	// Reads an array of values.
	template<typename T> T const* ReadArray(size_t elementCount)
	{
		static_assert(std::is_pod<T>::value, "Can only read plain-old-data types");

		uint8_t const* newPos = mPos + sizeof(T) * elementCount;

		if (newPos < mPos)
				throw std::overflow_error("ReadArray");

		if (newPos > mEnd)
				throw std::runtime_error("End of file");

		auto result = reinterpret_cast<T const*>(mPos);

		mPos = newPos;

		return result;
	}

	// Lower level helper reads directly from the filesystem into memory.
	static bool ReadEntireFile(string fileName, std::unique_ptr<uint8_t[]>& data, size_t* dataSize)
	{
		std::ifstream file(fileName, std::ios::binary | std::ios::in);
		file.seekg(0, std::ios::end);
		size_t filesize = file.tellg();
		file.seekg(0, std::ios::beg);

		// Create enough space for the file data.
		data.reset(new uint8_t[filesize]);

		// Read the data in.
		file.read((char*)data.get(), filesize);

		*dataSize = filesize;

		return true;
	}

private:
	// The data currently being read.
	uint8_t const* mPos;
	uint8_t const* mEnd;

	std::unique_ptr<uint8_t[]> mOwnedData;
};