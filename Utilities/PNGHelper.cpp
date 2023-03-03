#include "pch.h"
#include <sstream>
#include "PNGHelper.h"
#include "miniz.h"

#define SPNG_USE_MINIZ
#include "spng.h"

bool PNGHelper::WritePNG(std::stringstream &stream, uint32_t* buffer, uint32_t xSize, uint32_t ySize, uint32_t bitsPerPixel)
{
	size_t pngSize = 0;

	uint32_t size = xSize * ySize * bitsPerPixel / 8;
	vector<uint8_t> convertedData(size, 0);
	if(bitsPerPixel == 32) {
		//ARGB -> ABGR
		for(uint32_t i = 0; i < size / 4; i++) {
			convertedData[i * 4] = (buffer[i] & 0xFF0000) >> 16;
			convertedData[i * 4 + 1] = (buffer[i] & 0xFF00) >> 8;
			convertedData[i * 4 + 2] = (buffer[i] & 0xFF);
			convertedData[i * 4 + 3] = (buffer[i] & 0xFF000000) >> 24;
		}
	} else if(bitsPerPixel == 24) {
		//ARGB -> BGR
		for(uint32_t i = 0; i < size / 3; i++) {
			convertedData[i * 3] = (buffer[i] & 0xFF0000) >> 16;
			convertedData[i * 3 + 1] = (buffer[i] & 0xFF00) >> 8;
			convertedData[i * 3 + 2] = (buffer[i] & 0xFF);
		}
	} else {
		return false;
	}

	void* pngData = tdefl_write_image_to_png_file_in_memory_ex(convertedData.data(), xSize, ySize, bitsPerPixel / 8, &pngSize, MZ_DEFAULT_LEVEL, MZ_FALSE);
	if(!pngData) {
		std::cout << "tdefl_write_image_to_png_file_in_memory_ex() failed!" << std::endl;
		return false;
	} else {
		stream.write((char*)pngData, pngSize);
		mz_free(pngData);
		return true;
	}
}

bool PNGHelper::WritePNG(string filename, uint32_t* buffer, uint32_t xSize, uint32_t ySize, uint32_t bitsPerPixel)
{
	std::stringstream stream;
	if(WritePNG(stream, buffer, xSize, ySize, bitsPerPixel)) {
		ofstream file(filename, std::ios::out | std::ios::binary);
		if(file.good()) {
			file << stream.rdbuf();
		}
		file.close();
		return true;
	}
	return false;
}

template<typename T>
bool PNGHelper::ReadPNG(vector<uint8_t> input, vector<T> &output, uint32_t &pngWidth, uint32_t &pngHeight)
{
	unsigned long width = 0;
	unsigned long height = 0;

	pngWidth = 0;
	pngHeight = 0;

	if(DecodePNG(output, width, height, input.data(), input.size()) == 0) {
		uint32_t *pngDataPtr = (uint32_t*)output.data();
		for(size_t i = 0, len = output.size() * sizeof(T) / 4; i < len; i++) {
			//ABGR to ARGB
			pngDataPtr[i] = (pngDataPtr[i] & 0xFF00FF00) | ((pngDataPtr[i] & 0xFF0000) >> 16) | ((pngDataPtr[i] & 0xFF) << 16);
		}
		pngWidth = width;
		pngHeight = height;

		return true;
	} else {
		return false;
	}
} 

bool PNGHelper::ReadPNG(string filename, vector<uint8_t> &pngData, uint32_t &pngWidth, uint32_t &pngHeight)
{
	pngWidth = 0;
	pngHeight = 0;

	ifstream pngFile(filename, std::ios::in | std::ios::binary);
	if(pngFile) {
		pngFile.seekg(0, std::ios::end);
		size_t fileSize = (size_t)pngFile.tellg();
		pngFile.seekg(0, std::ios::beg);

		vector<uint8_t> fileData(fileSize, 0);
		pngFile.read((char*)fileData.data(), fileData.size());
		return ReadPNG(fileData, pngData, pngWidth, pngHeight);
	}

	return false;
}

template<typename T>
int PNGHelper::DecodePNG(vector<T>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32)
{
	int r = 0;
	spng_ctx* ctx = spng_ctx_new(0);
	if((r = spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE))) {
		spng_ctx_free(ctx);
		return r;
	}

	size_t limit = 1024 * 1024 * 64;
	if((r = spng_set_chunk_limits(ctx, limit, limit))) {
		spng_ctx_free(ctx);
		return r;
	}

	if((r = spng_set_png_buffer(ctx, in_png, in_size))) {
		spng_ctx_free(ctx);
		return r;
	}
	
	struct spng_ihdr ihdr;
	if((r = spng_get_ihdr(ctx, &ihdr))) {
		spng_ctx_free(ctx);
		return r;
	}

	image_width = ihdr.width;
	image_height = ihdr.height;

	int fmt = SPNG_FMT_RGBA8;
	size_t out_size = 0;
	if((r = spng_decoded_image_size(ctx, fmt, &out_size))) {
		spng_ctx_free(ctx);
		return r;
	}

	out_image.resize(out_size / sizeof(T));
	
	if((r = spng_decode_image(ctx, (unsigned char*)out_image.data(), out_size, fmt, SPNG_DECODE_TRNS))) {
		spng_ctx_free(ctx);
		return r;
	}

	spng_ctx_free(ctx);
	return 0;
}

template int PNGHelper::DecodePNG<uint8_t>(vector<uint8_t>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32);
template int PNGHelper::DecodePNG<uint32_t>(vector<uint32_t>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32);

template bool PNGHelper::ReadPNG<uint8_t>(vector<uint8_t> input, vector<uint8_t>& output, uint32_t& pngWidth, uint32_t& pngHeight);
template bool PNGHelper::ReadPNG<uint32_t>(vector<uint8_t> input, vector<uint32_t>& output, uint32_t& pngWidth, uint32_t& pngHeight);
