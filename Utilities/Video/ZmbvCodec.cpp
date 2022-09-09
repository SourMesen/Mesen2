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

#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "miniz.h"
#include "ZmbvCodec.h"

#define DBZV_VERSION_HIGH 0
#define DBZV_VERSION_LOW 1

#define COMPRESSION_NONE 0
#define COMPRESSION_ZLIB 1

#define MAX_VECTOR	16

#define Mask_KeyFrame			0x01
#define	Mask_DeltaPalette		0x02

int ZmbvCodec::NeededSize( int _width, int _height, zmbv_format_t _format) {
	int f;
	switch (_format) {
	case ZMBV_FORMAT_8BPP:f = 1;break;
	case ZMBV_FORMAT_15BPP:f = 2;break;
	case ZMBV_FORMAT_16BPP:f = 2;break;
	case ZMBV_FORMAT_32BPP:f = 4;break;
	default:
		return -1;
	}
	f = f*_width*_height + 2*(1+(_width/8)) * (1+(_height/8))+1024;
	return f + f/1000;
}

bool ZmbvCodec::SetupBuffers(zmbv_format_t _format, int blockwidth, int blockheight) {
	FreeBuffers();
	palsize = 0;
	switch (_format) {
	case ZMBV_FORMAT_8BPP:
		pixelsize = 1;
		palsize = 256;
		break;
	case ZMBV_FORMAT_15BPP:
		pixelsize = 2;
		break;
	case ZMBV_FORMAT_16BPP:
		pixelsize = 2;
		break;
	case ZMBV_FORMAT_32BPP:
		pixelsize = 4;
		break;
	default:
		return false;
	};
	bufsize = (height+2*MAX_VECTOR)*pitch*pixelsize+2048;

	buf1 = new unsigned char[bufsize];
	buf2 = new unsigned char[bufsize];
	work = new unsigned char[bufsize];

	int xblocks = (width/blockwidth);
	int xleft = width % blockwidth;
	if (xleft) xblocks++;
	int yblocks = (height/blockheight);
	int yleft = height % blockheight;
	if (yleft) yblocks++;
	blockcount=yblocks*xblocks;
	blocks=new FrameBlock[blockcount];

	if (!buf1 || !buf2 || !work || !blocks) {
		FreeBuffers();
		return false;
	}
	int y,x,i;
	i=0;
	for (y=0;y<yblocks;y++) {
		for (x=0;x<xblocks;x++) {
			blocks[i].start=((y*blockheight)+MAX_VECTOR)*pitch+
				(x*blockwidth)+MAX_VECTOR;
			if (xleft && x==(xblocks-1)) {
                blocks[i].dx=xleft;
			} else {
				blocks[i].dx=blockwidth;
			}
			if (yleft && y==(yblocks-1)) {
                blocks[i].dy=yleft;
			} else {
				blocks[i].dy=blockheight;
			}
			i++;
		}
	}

	memset(buf1,0,bufsize);
	memset(buf2,0,bufsize);
	memset(work,0,bufsize);
	oldframe=buf1;
	newframe=buf2;
	format = _format;

	_bufSize = NeededSize(width, height, format);
	_buf = new uint8_t[_bufSize];

	return true;
}

void ZmbvCodec::CreateVectorTable(void) {
	int x,y,s;
	VectorCount=1;

	VectorTable[0].x=VectorTable[0].y=0;
	for (s=1;s<=10;s++) {
		for (y=0-s;y<=0+s;y++) for (x=0-s;x<=0+s;x++) {
			if (abs(x)==s || abs(y)==s) {
				VectorTable[VectorCount].x=x;
				VectorTable[VectorCount].y=y;
				VectorCount++;
			}
		}
	}
}

template<class P>
INLINE int ZmbvCodec::PossibleBlock(int vx,int vy,FrameBlock * block) {
	int ret=0;
	P * pold=((P*)oldframe)+block->start+(vy*pitch)+vx;
	P * pnew=((P*)newframe)+block->start;;	
	for (int y=0;y<block->dy;y+=4) {
		for (int x=0;x<block->dx;x+=4) {
			int test=0-((pold[x]-pnew[x])&0x00ffffff);
			ret-=(test>>31);
		}
		pold+=pitch*4;
		pnew+=pitch*4;
	}
	return ret;
}

template<class P>
INLINE int ZmbvCodec::CompareBlock(int vx,int vy,FrameBlock * block) {
	int ret=0;
	P * pold=((P*)oldframe)+block->start+(vy*pitch)+vx;
	P * pnew=((P*)newframe)+block->start;;	
	for (int y=0;y<block->dy;y++) {
		for (int x=0;x<block->dx;x++) {
			int test=0-((pold[x]-pnew[x])&0x00ffffff);
			ret-=(test>>31);
		}
		pold+=pitch;
		pnew+=pitch;
	}
	return ret;
}

template<class P>
INLINE void ZmbvCodec::AddXorBlock(int vx,int vy,FrameBlock * block) {
	P * pold=((P*)oldframe)+block->start+(vy*pitch)+vx;
	P * pnew=((P*)newframe)+block->start;
	for (int y=0;y<block->dy;y++) {
		for (int x=0;x<block->dx;x++) {
			*((P*)&work[workUsed])=pnew[x] ^ pold[x];
			workUsed+=sizeof(P);
		}
		pold+=pitch;
		pnew+=pitch;
	}
}

template<class P>
void ZmbvCodec::AddXorFrame(void) {
	signed char * vectors=(signed char*)&work[workUsed];
	/* Align the following xor data on 4 byte boundary*/
	workUsed=(workUsed + blockcount*2 +3) & ~3;
	for (int b=0;b<blockcount;b++) {
		FrameBlock * block=&blocks[b];
		int bestvx = 0;
		int bestvy = 0;
		int bestchange=CompareBlock<P>(0,0, block);
		int possibles=64;
		for (int v=0;v<VectorCount && possibles;v++) {
			if (bestchange<4) break;
			int vx = VectorTable[v].x;
			int vy = VectorTable[v].y;
			if (PossibleBlock<P>(vx, vy, block) < 4) {
				possibles--;
				int testchange=CompareBlock<P>(vx,vy, block);
				if (testchange<bestchange) {
					bestchange=testchange;
					bestvx = vx;
					bestvy = vy;
				}
			}
		}
		vectors[b*2+0]=(bestvx << 1);
		vectors[b*2+1]=(bestvy << 1);
		if (bestchange) {
			vectors[b*2+0]|=1;
			AddXorBlock<P>(bestvx, bestvy, block);
		}
	}
}

bool ZmbvCodec::SetupCompress( int _width, int _height, uint32_t compressionLevel ) {
	width = _width;
	height = _height;
	pitch = _width + 2*MAX_VECTOR;
	format = ZMBV_FORMAT_NONE;
	if (deflateInit (&zstream, compressionLevel) != Z_OK)
		return false;

	return true;
}

bool ZmbvCodec::PrepareCompressFrame(int flags, zmbv_format_t _format, char * pal)
{
	int i;
	unsigned char *firstByte;

	if (_format != format) {
		if (!SetupBuffers( _format, 16, 16))
			return false;
		flags|=1;	//Force a keyframe
	}
	/* replace oldframe with new frame */
	unsigned char *copyFrame = newframe;
	newframe = oldframe;
	oldframe = copyFrame;

	compressInfo.linesDone = 0;
	compressInfo.writeSize = _bufSize;
	compressInfo.writeDone = 1;
	compressInfo.writeBuf = (unsigned char *)_buf;
	/* Set a pointer to the first byte which will contain info about this frame */
	firstByte = compressInfo.writeBuf;
	*firstByte = 0;
	//Reset the work buffer
	workUsed = 0;workPos = 0;
	if (flags & 1) {
		/* Make a keyframe */
		*firstByte |= Mask_KeyFrame;
		KeyframeHeader * header = (KeyframeHeader *)(compressInfo.writeBuf + compressInfo.writeDone);
		header->high_version = DBZV_VERSION_HIGH;
		header->low_version = DBZV_VERSION_LOW;
		header->compression = COMPRESSION_ZLIB;
		header->format = format;
		header->blockwidth = 16;
		header->blockheight = 16;
		compressInfo.writeDone += sizeof(KeyframeHeader);
		/* Copy the new frame directly over */
		if (palsize) {
			if (pal)
				memcpy(&palette, pal, sizeof(palette));
			else 
				memset(&palette,0, sizeof(palette));
			/* keyframes get the full palette */
			for (i=0;i<palsize;i++) {
				work[workUsed++] = palette[i*4+0];
				work[workUsed++] = palette[i*4+1];
				work[workUsed++] = palette[i*4+2];
			}
		}
		/* Restart deflate */
		deflateReset(&zstream);
	} else {
		if (palsize && pal && memcmp(pal, palette, palsize * 4)) {
			*firstByte |= Mask_DeltaPalette;
			for(i=0;i<palsize;i++) {
				work[workUsed++]=palette[i*4+0] ^ pal[i*4+0];
				work[workUsed++]=palette[i*4+1] ^ pal[i*4+1];
				work[workUsed++]=palette[i*4+2] ^ pal[i*4+2];
			}
			memcpy(&palette,pal, palsize * 4);
		}
	}
	return true;
}

void ZmbvCodec::CompressLines(int lineCount, void *lineData[])
{
	int linePitch = pitch * pixelsize;
	int lineWidth = width * pixelsize;
	int i = 0;
	unsigned char *destStart = newframe + pixelsize*(MAX_VECTOR+(compressInfo.linesDone+MAX_VECTOR)*pitch);
	while ( i < lineCount && (compressInfo.linesDone < height)) {
		memcpy(destStart, lineData[i],  lineWidth );
		destStart += linePitch;
		i++; compressInfo.linesDone++;
	}
}

int ZmbvCodec::FinishCompressFrame(uint8_t** compressedData)
{
	unsigned char firstByte = *compressInfo.writeBuf;
	if (firstByte & Mask_KeyFrame) {
		int i;
		/* Add the full frame data */
		unsigned char * readFrame = newframe + pixelsize*(MAX_VECTOR+MAX_VECTOR*pitch);	
		for (i=0;i<height;i++) {
			memcpy(&work[workUsed], readFrame, width*pixelsize);
			readFrame += pitch*pixelsize;
			workUsed += width*pixelsize;
		}
	} else {
		/* Add the delta frame data */
		switch (format) {
			case ZMBV_FORMAT_8BPP:
				AddXorFrame<int8_t>();
				break;
			case ZMBV_FORMAT_15BPP:
			case ZMBV_FORMAT_16BPP:
				AddXorFrame<int16_t>();
				break;

			default:
			case ZMBV_FORMAT_32BPP:
				AddXorFrame<int32_t>();
				break;
		}
	}
	/* Create the actual frame with compression */
	zstream.next_in = (Bytef *)work;
	zstream.avail_in = workUsed;
	zstream.total_in = 0;

	zstream.next_out = (Bytef *)(compressInfo.writeBuf + compressInfo.writeDone);
	zstream.avail_out = compressInfo.writeSize - compressInfo.writeDone;
	zstream.total_out = 0;
	
	deflate(&zstream, Z_SYNC_FLUSH);

	*compressedData = _buf;

	return compressInfo.writeDone + zstream.total_out;
}

void ZmbvCodec::FreeBuffers()
{
	delete[] blocks;
	blocks = nullptr;
	delete[] buf1;
	buf1 = nullptr;
	delete[] buf2;
	buf2 = nullptr;
	delete[] work;
	work = nullptr;
	delete[] _buf;
	_buf = nullptr;
}

ZmbvCodec::ZmbvCodec() 
{
	CreateVectorTable();
	blocks = nullptr;
	buf1 = nullptr;
	buf2 = nullptr;
	work = nullptr;
	memset( &zstream, 0, sizeof(zstream));
}

int ZmbvCodec::CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData)
{
	if(!PrepareCompressFrame(isKeyFrame ? 1 : 0, ZMBV_FORMAT_32BPP, nullptr)) {
		return -1;
	}

	for(int i = 0; i < height; i++) {
		void * rowPointer = frameData + i*width*4;
		CompressLines(1, &rowPointer);
	}

	return FinishCompressFrame(compressedData);
}

const char* ZmbvCodec::GetFourCC()
{
	return "ZMBV";
}