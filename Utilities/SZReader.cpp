#include "pch.h"
#include <algorithm>
#include <cstring>
#include "SZReader.h"
#include "Utilities/UTF8Util.h"
#include "SevenZip/7zMemBuffer.h"

SZReader::SZReader()
{
}

SZReader::~SZReader()
{
	SzArEx_Free(&_archive, &_allocImp);
}

bool SZReader::InternalLoadArchive(void* buffer, size_t size)
{
	if(_initialized) {
		SzArEx_Free(&_archive, &_allocImp);
		_initialized = false;
	}

	ISzAlloc allocImp{ SzAlloc, SzFree };
	ISzAlloc allocTempImp{ SzAllocTemp, SzFreeTemp };

	MemBufferInit(&_memBufferStream, &_lookStream, buffer, size);
	CrcGenerateTable();
	SzArEx_Init(&_archive);

	return !SzArEx_Open(&_archive, &_lookStream.s, &allocImp, &allocTempImp);
}

bool SZReader::ExtractFile(string filename, vector<uint8_t> &output)
{
	bool result = false;
	if(_initialized) {
		char16_t *utf16Filename = (char16_t*)SzAlloc(nullptr, 2000);

		uint32_t blockIndex = 0xFFFFFFFF;
		uint8_t *outBuffer = 0;
		size_t outBufferSize = 0;

		for(uint32_t i = 0; i < _archive.NumFiles; i++) {
			size_t offset = 0;
			size_t outSizeProcessed = 0;
			unsigned isDir = SzArEx_IsDir(&_archive, i);
			if(isDir) {
				continue;
			}

			SzArEx_GetFileNameUtf16(&_archive, i, (uint16_t*)utf16Filename);
			string entryName = utf8::utf8::encode(std::u16string(utf16Filename));
			if(filename == entryName) {
				WRes res = SzArEx_Extract(&_archive, &_lookStream.s, i, &blockIndex, &outBuffer, &outBufferSize, &offset, &outSizeProcessed, &_allocImp, &_allocTempImp);
				if(res == SZ_OK) {
					output = vector<uint8_t>(outBuffer+offset, outBuffer+offset+outSizeProcessed);
					result = true;
				}
				IAlloc_Free(&_allocImp, outBuffer);
				break;
			}
		}
		SzFree(nullptr, utf16Filename);
	}

	return result;
}

vector<string> SZReader::InternalGetFileList()
{
	vector<string> filenames;
	char16_t *utf16Filename = (char16_t*)SzAlloc(nullptr, 2000);

	if(_initialized) {
		for(uint32_t i = 0; i < _archive.NumFiles; i++) {
			unsigned isDir = SzArEx_IsDir(&_archive, i);
			if(isDir) {
				continue;
			}

			SzArEx_GetFileNameUtf16(&_archive, i, (uint16_t*)utf16Filename);
			string filename = utf8::utf8::encode(std::u16string(utf16Filename));
			filenames.push_back(filename);
		}
	}
	SzFree(nullptr, utf16Filename);

	return filenames;
}