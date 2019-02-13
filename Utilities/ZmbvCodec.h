// This file is a part of Mesen
// It is a heavily modified version of the zmbv.h/cpp file found in DOSBox's code.

/*
 *  Copyright (C) 2002-2011  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#pragma once

#include "BaseCodec.h"
#include "miniz.h"

#ifdef _MSC_VER
#define INLINE __forceinline
#else
#define INLINE inline
#endif

typedef enum {
	ZMBV_FORMAT_NONE		= 0x00,
	ZMBV_FORMAT_1BPP		= 0x01,
	ZMBV_FORMAT_2BPP		= 0x02,
	ZMBV_FORMAT_4BPP		= 0x03,
	ZMBV_FORMAT_8BPP		= 0x04,
	ZMBV_FORMAT_15BPP	= 0x05,
	ZMBV_FORMAT_16BPP	= 0x06,
	ZMBV_FORMAT_24BPP	= 0x07,
	ZMBV_FORMAT_32BPP	= 0x08
} zmbv_format_t;

class ZmbvCodec : public BaseCodec
{
private:
	struct FrameBlock {
		int start = 0;
		int dx = 0,dy = 0;
	};
	struct CodecVector {
		int x = 0,y = 0;
		int slot = 0;
	};
	struct KeyframeHeader {
		unsigned char high_version = 0;
		unsigned char low_version = 0;
		unsigned char compression = 0;
		unsigned char format = 0;
		unsigned char blockwidth = 0,blockheight = 0;
	};

	struct {
		int		linesDone = 0;
		int		writeSize = 0;
		int		writeDone = 0;
		unsigned char	*writeBuf = nullptr;
	} compressInfo;

	CodecVector VectorTable[512] = {};
	int VectorCount = 0;

	unsigned char *oldframe=nullptr, *newframe=nullptr;
	unsigned char *buf1=nullptr, *buf2=nullptr, *work=nullptr;
	int bufsize = 0;

	int blockcount = 0; 
	FrameBlock * blocks = nullptr;

	int workUsed = 0, workPos = 0;

	int palsize = 0;
	char palette[256*4] = {};
	int height = 0, width = 0, pitch = 0;
	zmbv_format_t format = zmbv_format_t::ZMBV_FORMAT_NONE;
	int pixelsize = 0;

	uint8_t* _buf = nullptr;
	uint32_t _bufSize = 0;

	z_stream zstream = {};

	// methods
	void FreeBuffers(void);
	void CreateVectorTable(void);
	bool SetupBuffers(zmbv_format_t format, int blockwidth, int blockheight);

	template<class P> void AddXorFrame(void);
	template<class P> INLINE int PossibleBlock(int vx,int vy,FrameBlock * block);
	template<class P> INLINE int CompareBlock(int vx,int vy,FrameBlock * block);
	template<class P> INLINE void AddXorBlock(int vx,int vy,FrameBlock * block);

	int NeededSize(int _width, int _height, zmbv_format_t _format);

	void CompressLines(int lineCount, void *lineData[]);
	bool PrepareCompressFrame(int flags, zmbv_format_t _format, char * pal);
	int FinishCompressFrame(uint8_t** compressedData);

public:
	ZmbvCodec();
	bool SetupCompress(int _width, int _height, uint32_t compressionLevel) override;
	int CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData) override;
	const char* GetFourCC() override;
};
