// This file is a part of Mesen
// It is a heavily modified version of the hardware.h/cpp file found in DOSBox's code.

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
#include <fstream>
#include <cstring>
#include "AviWriter.h"
#include "BaseCodec.h"
#include "RawCodec.h"
#include "ZmbvCodec.h"
#include "CamstudioCodec.h"

void AviWriter::WriteAviChunk(const char *tag, uint32_t size, void *data, uint32_t flags)
{
	uint8_t chunk[8] = { (uint8_t)tag[0], (uint8_t)tag[1], (uint8_t)tag[2], (uint8_t)tag[3] };
	host_writed(&chunk[4], size);
	_file.write((char*)chunk, 8);
	
	uint32_t writesize = (size + 1)&~1;
	_file.write((char*)data, writesize);
	
	uint32_t pos = _written + 4;
	_written += writesize + 8;

	_aviIndex.push_back(tag[0]);
	_aviIndex.push_back(tag[1]);
	_aviIndex.push_back(tag[2]);
	_aviIndex.push_back(tag[3]);
	_aviIndex.insert(_aviIndex.end(), 12, 0);
	host_writed(_aviIndex.data() + _aviIndex.size() - 12, flags);
	host_writed(_aviIndex.data() + _aviIndex.size() - 8, pos);
	host_writed(_aviIndex.data() + _aviIndex.size() - 4, size);
}

void AviWriter::host_writew(uint8_t* buffer, uint16_t value)
{
	buffer[0] = value & 0xFF;
	buffer[1] = value >> 8;
}

void AviWriter::host_writed(uint8_t* buffer, uint32_t value)
{
	buffer[0] = value;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
}

bool AviWriter::StartWrite(string filename, VideoCodec codec, uint32_t width, uint32_t height, uint32_t bpp, uint32_t fps, uint32_t audioSampleRate, uint32_t compressionLevel)
{
	_codecType = codec;
	_file.open(filename, std::ios::out | std::ios::binary);
	if(!_file) {
		return false;
	}
	
	switch(_codecType) {
		default:
		case VideoCodec::None: _codec.reset(new RawCodec()); break;
		case VideoCodec::ZMBV: _codec.reset(new ZmbvCodec()); break;
		case VideoCodec::CSCD: _codec.reset(new CamstudioCodec()); break;
	}

	if(!_codec->SetupCompress(width, height, compressionLevel)) {
		return false;
	}

	_frameBuffer = new uint8_t[width*height*bpp];

	_aviIndex.clear();
	_aviIndex.insert(_aviIndex.end(), 8, 0);

	_width = width;
	_height = height;
	_bpp = bpp;
	_fps = fps;

	_audiorate = audioSampleRate;

	for(int i = 0; i < AviWriter::AviHeaderSize; i++) {
		_file.put(0);
	}
	_frames = 0;
	_written = 0;
	_audioPos = 0;
	_audiowritten = 0;

	return true;
}

void AviWriter::EndWrite()
{
	/* Close the video */
	uint8_t avi_header[AviWriter::AviHeaderSize];
	uint32_t main_list;
	uint32_t header_pos = 0;
#define AVIOUT4(_S_) memcpy(&avi_header[header_pos],_S_,4);header_pos+=4;
#define AVIOUTw(_S_) host_writew(&avi_header[header_pos], _S_);header_pos+=2;
#define AVIOUTd(_S_) host_writed(&avi_header[header_pos], _S_);header_pos+=4;
	/* Try and write an avi header */
	AVIOUT4("RIFF");                    // Riff header 
	AVIOUTd(AviWriter::AviHeaderSize + _written - 8 + (uint32_t)_aviIndex.size());
	AVIOUT4("AVI ");
	AVIOUT4("LIST");                    // List header
	main_list = header_pos;
	AVIOUTd(0);				            // TODO size of list
	AVIOUT4("hdrl");

	AVIOUT4("avih");
	AVIOUTd(56);                         /* # of bytes to follow */
	AVIOUTd((uint32_t)(1000000 / _fps));       /* Microseconds per frame */
	AVIOUTd(0);
	AVIOUTd(0);                         /* PaddingGranularity (whatever that might be) */
	AVIOUTd(0x110);                     /* Flags,0x10 has index, 0x100 interleaved */
	AVIOUTd(_frames);      /* TotalFrames */
	AVIOUTd(0);                         /* InitialFrames */
	AVIOUTd(2);                         /* Stream count */
	AVIOUTd(0);                         /* SuggestedBufferSize */
	AVIOUTd(_width);       /* Width */
	AVIOUTd(_height);      /* Height */
	AVIOUTd(0);                         /* TimeScale:  Unit used to measure time */
	AVIOUTd(0);                         /* DataRate:   Data rate of playback     */
	AVIOUTd(0);                         /* StartTime:  Starting time of AVI data */
	AVIOUTd(0);                         /* DataLength: Size of AVI data chunk    */

													/* Video stream list */
	AVIOUT4("LIST");
	AVIOUTd(4 + 8 + 56 + 8 + 40);       /* Size of the list */
	AVIOUT4("strl");
	/* video stream header */
	AVIOUT4("strh");
	AVIOUTd(56);                        /* # of bytes to follow */
	AVIOUT4("vids");                    /* Type */
	AVIOUT4(_codec->GetFourCC());		            /* Handler */
	AVIOUTd(0);                         /* Flags */
	AVIOUTd(0);                         /* Reserved, MS says: wPriority, wLanguage */
	AVIOUTd(0);                         /* InitialFrames */
	AVIOUTd(1000000);                   /* Scale */
	AVIOUTd(_fps);              /* Rate: Rate/Scale == samples/second */
	AVIOUTd(0);                         /* Start */
	AVIOUTd(_frames);      /* Length */
	AVIOUTd(0);                  /* SuggestedBufferSize */
	AVIOUTd(~0);                 /* Quality */
	AVIOUTd(0);                  /* SampleSize */
	AVIOUTd(0);                  /* Frame */
	AVIOUTd(0);                  /* Frame */
											/* The video stream format */
	AVIOUT4("strf");
	AVIOUTd(40);                 /* # of bytes to follow */
	AVIOUTd(40);                 /* Size */
	AVIOUTd(_width);         /* Width */
	AVIOUTd(_height);        /* Height */
														//		OUTSHRT(1); OUTSHRT(24);     /* Planes, Count */
	AVIOUTw(1);  //number of planes
	AVIOUTw(24); //bits for colors
	AVIOUT4(_codec->GetFourCC());          /* Compression */
	AVIOUTd(_width * _height * 4);  /* SizeImage (in bytes?) */
	AVIOUTd(0);                  /* XPelsPerMeter */
	AVIOUTd(0);                  /* YPelsPerMeter */
	AVIOUTd(0);                  /* ClrUsed: Number of colors used */
	AVIOUTd(0);                  /* ClrImportant: Number of colors important */

											/* Audio stream list */
	AVIOUT4("LIST");
	AVIOUTd(4 + 8 + 56 + 8 + 16);  /* Length of list in bytes */
	AVIOUT4("strl");
	/* The audio stream header */
	AVIOUT4("strh");
	AVIOUTd(56);            /* # of bytes to follow */
	AVIOUT4("auds");
	AVIOUTd(0);             /* Format (Optionally) */
	AVIOUTd(0);             /* Flags */
	AVIOUTd(0);             /* Reserved, MS says: wPriority, wLanguage */
	AVIOUTd(0);             /* InitialFrames */
	AVIOUTd(4);    /* Scale */
	AVIOUTd(_audiorate * 4);             /* Rate, actual rate is scale/rate */
	AVIOUTd(0);             /* Start */
	if(!_audiorate)
		_audiorate = 1;
	AVIOUTd(_audiowritten / 4);   /* Length */
	AVIOUTd(0);             /* SuggestedBufferSize */
	AVIOUTd(~0);            /* Quality */
	AVIOUTd(4);				/* SampleSize */
	AVIOUTd(0);             /* Frame */
	AVIOUTd(0);             /* Frame */
									/* The audio stream format */
	AVIOUT4("strf");
	AVIOUTd(16);            /* # of bytes to follow */
	AVIOUTw(1);             /* Format, WAVE_ZMBV_FORMAT_PCM */
	AVIOUTw(2);             /* Number of channels */
	AVIOUTd(_audiorate);          /* SamplesPerSec */
	AVIOUTd(_audiorate * 4);        /* AvgBytesPerSec*/
	AVIOUTw(4);             /* BlockAlign */
	AVIOUTw(16);            /* BitsPerSample */
	int nmain = header_pos - main_list - 4;
	/* Finish stream list, i.e. put number of bytes in the list to proper pos */

	int njunk = AviWriter::AviHeaderSize - 8 - 12 - header_pos;
	AVIOUT4("JUNK");
	AVIOUTd(njunk);
	/* Fix the size of the main list */
	header_pos = main_list;
	AVIOUTd(nmain);
	header_pos = AviWriter::AviHeaderSize - 12;
	AVIOUT4("LIST");
	AVIOUTd(_written + 4); /* Length of list in bytes */
	AVIOUT4("movi");
	/* First add the index table to the end */
	memcpy(_aviIndex.data(), "idx1", 4);
	host_writed(_aviIndex.data() + 4, (uint32_t)_aviIndex.size() - 8);
	
	_file.write((char*)_aviIndex.data(), _aviIndex.size());
	_file.seekp(std::ios::beg);
	_file.write((char*)avi_header, AviWriter::AviHeaderSize);
	_file.close();
}

void AviWriter::AddFrame(uint8_t *frameData)
{
	if(!_file) {
		return;
	}

	bool isKeyFrame = (_frames % 120 == 0) ? 1 : 0;

	uint8_t* compressedData = nullptr;
	int written = _codec->CompressFrame(isKeyFrame, frameData, &compressedData);
	if(written < 0) {
		return;
	}

	if(_codecType == VideoCodec::None) {
		isKeyFrame = true;
	}
	WriteAviChunk(_codecType == VideoCodec::None ? "00db" : "00dc", written, compressedData, isKeyFrame ? 0x10 : 0);
	_frames++;

	if(_audioPos) {
		auto lock = _audioLock.AcquireSafe();
		WriteAviChunk("01wb", _audioPos, _audiobuf, 0);
		_audiowritten += _audioPos;
		_audioPos = 0;
	}
}

void AviWriter::AddSound(int16_t *data, uint32_t sampleCount)
{
	if(!_file) {
		return;
	}

	auto lock = _audioLock.AcquireSafe();
	memcpy(_audiobuf+_audioPos/2, data, sampleCount * 4);
	_audioPos += sampleCount * 4;
}