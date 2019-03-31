//--------------------------------------------------------------------------------------
// This a heavily modified version of SpriteFont.cpp from DirectX Toolkit (MIT license)
// It strips down a lot of options not needed for Mesen and implements the minimum
// required to use .spritefont files in SDL. 
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// File: SpriteFont.cpp
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

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <climits>

#include "SpriteFont.h"

// Internal SpriteFont implementation class.
class SpriteFont::Impl
{
public:
    Impl(SDL_Renderer* renderer, BinaryReader* reader);
	 virtual ~Impl();

    Glyph const* FindGlyph(wchar_t character) const;

    void SetDefaultCharacter(wchar_t character);

    template<typename TAction>
    void ForEachGlyph(wchar_t const* text, TAction action) const;


    // Fields.
	 std::vector<uint8_t> textureData; 
    SDL_Texture* texture;
    std::vector<Glyph> glyphs;
    Glyph const* defaultGlyph;
    float lineSpacing;
};


// Constants.
const XMFLOAT2 SpriteFont::Float2Zero(0, 0);

static const char spriteFontMagic[] = "DXTKfont";


// Comparison operators make our sorted glyph vector work with std::binary_search and lower_bound.
static inline bool operator< (wchar_t left, SpriteFont::Glyph const& right)
{
	return (uint32_t)left < right.Character;
}

static inline bool operator< (SpriteFont::Glyph const& left, wchar_t right)
{
	return left.Character < (uint32_t)right;
}

// Reads a SpriteFont from the binary format created by the MakeSpriteFont utility.
SpriteFont::Impl::Impl(SDL_Renderer* renderer, BinaryReader* reader) :
    defaultGlyph(nullptr)
{
	// Validate the header.
	for (char const* magic = spriteFontMagic; *magic; magic++)
	{
		if (reader->Read<uint8_t>() != *magic)
		{
			throw std::runtime_error("Not a MakeSpriteFont output binary");
		}
	}

	// Read the glyph data.
	auto glyphCount = reader->Read<uint32_t>();
	auto glyphData = reader->ReadArray<Glyph>(glyphCount);

	glyphs.assign(glyphData, glyphData + glyphCount);

	// Read font properties.
	lineSpacing = reader->Read<float>();

	SetDefaultCharacter((wchar_t)reader->Read<uint32_t>());

	// Read the texture data.
	auto textureWidth = reader->Read<uint32_t>();
	auto textureHeight = reader->Read<uint32_t>();
	reader->Read<uint32_t>(); //DXGI_FORMAT, ignored - assume 32-bit RBGA
	auto textureStride = reader->Read<uint32_t>();
	auto textureRows = reader->Read<uint32_t>();
	auto pixelData = reader->ReadArray<uint8_t>(textureStride * textureRows);
	
	textureData.insert(textureData.end(), pixelData, pixelData+textureStride*textureHeight);

	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)textureData.data(), textureWidth, textureHeight, 32, textureStride, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
}

SpriteFont::Impl::~Impl()
{
	SDL_DestroyTexture(texture);
}

// Looks up the requested glyph, falling back to the default character if it is not in the font.
SpriteFont::Glyph const* SpriteFont::Impl::FindGlyph(wchar_t character) const
{
	auto glyph = std::lower_bound(glyphs.begin(), glyphs.end(), character);

	if (glyph != glyphs.end() && glyph->Character == (uint32_t)character)
	{
		return &*glyph;
	}

	if (defaultGlyph)
	{
		return defaultGlyph;
	}

	throw std::runtime_error("Character not in font");
}


// Sets the missing-character fallback glyph.
void SpriteFont::Impl::SetDefaultCharacter(wchar_t character)
{
	defaultGlyph = nullptr;

	if (character)
	{
		defaultGlyph = FindGlyph(character);
	}
}


// The core glyph layout algorithm, shared between DrawString and MeasureString.
template<typename TAction>
void SpriteFont::Impl::ForEachGlyph(wchar_t const* text, TAction action) const
{
	float x = 0;
	float y = 0;

	for (; *text; text++)
	{
		wchar_t character = *text;

		switch (character)
		{
			case '\r':
				// Skip carriage returns.
				continue;

			case '\n':
				// New line.
				x = 0;
				y += lineSpacing;
				break;

			default:
				// Output this character.
				auto glyph = FindGlyph(character);

				x += glyph->XOffset;

				if (x < 0)
					x = 0;

				float advance = glyph->Subrect.right - glyph->Subrect.left + glyph->XAdvance;

				if(!std::iswspace(character) || (glyph->Subrect.right - glyph->Subrect.left) > 1 || (glyph->Subrect.bottom - glyph->Subrect.top ) > 1) {
					action(glyph, x, y, advance);
				}

				x += advance;
				break;
		}
	}
}


// Construct from a binary file created by the MakeSpriteFont utility.
SpriteFont::SpriteFont(SDL_Renderer* renderer, string fileName)
{
	BinaryReader reader(fileName);

	pImpl = std::make_unique<Impl>(renderer, &reader);
}

// Move constructor.
SpriteFont::SpriteFont(SpriteFont&& moveFrom)
  : pImpl(std::move(moveFrom.pImpl))
{
}


// Move assignment.
SpriteFont& SpriteFont::operator= (SpriteFont&& moveFrom)
{
	pImpl = std::move(moveFrom.pImpl);
	return *this;
}


// Public destructor.
SpriteFont::~SpriteFont()
{
}

void SpriteFont::DrawString(SDL_Renderer *renderer, wchar_t const* text, int x, int y, uint8_t r, uint8_t g, uint8_t b) const
{
	SDL_SetTextureColorMod(pImpl->texture, r, g, b);	
	pImpl->ForEachGlyph(text, [&](Glyph const* glyph, float offsetX, float offsetY, float advance)
	{
		int width = (int)(glyph->Subrect.right - glyph->Subrect.left);
		int height = (int)(glyph->Subrect.bottom - glyph->Subrect.top);
		
		SDL_Rect source = {(int)glyph->Subrect.left, (int)glyph->Subrect.top, width, height};
		SDL_Rect dest = {x + (int)offsetX, y + (int)(offsetY + glyph->YOffset), width, height};
		SDL_RenderCopy(renderer, pImpl->texture, &source, &dest);
	});
}

XMFLOAT2 SpriteFont::MeasureString(wchar_t const* text) const
{
	XMFLOAT2 result;

	pImpl->ForEachGlyph(text, [&](Glyph const* glyph, float x, float y, float advance)
	{
		float w = (float)(glyph->Subrect.right - glyph->Subrect.left);
		float h = (float)(glyph->Subrect.bottom - glyph->Subrect.top) + glyph->YOffset;

		h = std::max(h, pImpl->lineSpacing);

		result.x = std::max(result.x, x + w);
		result.y = std::max(result.y, y + h);
	});

	return result;
}

// Spacing properties
float SpriteFont::GetLineSpacing() const
{
	return pImpl->lineSpacing;
}

void SpriteFont::SetLineSpacing(float spacing)
{
	pImpl->lineSpacing = spacing;
}

// Font properties
wchar_t SpriteFont::GetDefaultCharacter() const
{
	return pImpl->defaultGlyph ? (wchar_t)pImpl->defaultGlyph->Character : 0;
}

void SpriteFont::SetDefaultCharacter(wchar_t character)
{
	pImpl->SetDefaultCharacter(character);
}

bool SpriteFont::ContainsCharacter(wchar_t character) const
{
	return std::binary_search(pImpl->glyphs.begin(), pImpl->glyphs.end(), character);
}

// Custom layout/rendering
SpriteFont::Glyph const* SpriteFont::FindGlyph(wchar_t character) const
{
	return pImpl->FindGlyph(character);
}
