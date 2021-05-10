#include "stdafx.h"
#include <sstream>
#include "PNGHelper.h"
#include "miniz.h"

#define SPNG_USE_MINIZ
#include "spng.h"

bool PNGHelper::WritePNG(std::stringstream &stream, uint32_t* buffer, uint32_t xSize, uint32_t ySize, uint32_t bitsPerPixel)
{
	size_t pngSize = 0;

	//ARGB -> BGR
	uint32_t size = xSize * ySize * bitsPerPixel / 8 / 4;
	vector<uint8_t> convertedData(size * 3, 0);
	for(uint32_t i = 0; i < size; i++) {
		convertedData[i * 3] = (buffer[i] & 0xFF0000) >> 16;
		convertedData[i * 3 + 1] = (buffer[i] & 0xFF00) >> 8;
		convertedData[i * 3 + 2] = (buffer[i] & 0xFF);
	}

	void* pngData = tdefl_write_image_to_png_file_in_memory_ex(convertedData.data(), xSize, ySize, 3, &pngSize, MZ_DEFAULT_LEVEL, MZ_FALSE);
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

bool PNGHelper::ReadPNG(vector<uint8_t> input, vector<uint8_t> &output, uint32_t &pngWidth, uint32_t &pngHeight)
{
	unsigned long width = 0;
	unsigned long height = 0;

	pngWidth = 0;
	pngHeight = 0;

	if(DecodePNG(output, width, height, input.data(), input.size()) == 0) {
		uint32_t *pngDataPtr = (uint32_t*)output.data();
		for(size_t i = 0, len = output.size() / 4; i < len; i++) {
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

int PNGHelper::DecodePNG(vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32)
{
	int r = 0;
	unique_ptr<spng_ctx> ctx = unique_ptr<spng_ctx>(spng_ctx_new(0));
	if(r = spng_set_crc_action(ctx.get(), SPNG_CRC_USE, SPNG_CRC_USE)) {
		return r;
	}

	size_t limit = 1024 * 1024 * 64;
	if(r = spng_set_chunk_limits(ctx.get(), limit, limit)) {
		return r;
	}

	if(r = spng_set_png_buffer(ctx.get(), in_png, in_size)) {
		return r;
	}
	
	struct spng_ihdr ihdr;
	if(r = spng_get_ihdr(ctx.get(), &ihdr)) {
		return r;
	}

	image_width = ihdr.width;
	image_height = ihdr.height;

	int fmt = SPNG_FMT_RGBA8;
	size_t out_size = 0;
	if(r = spng_decoded_image_size(ctx.get(), fmt, &out_size)) {
		return r;
	}

	out_image.resize(out_size);
	
	if(r = spng_decode_image(ctx.get(), out_image.data(), out_image.size(), fmt, 0)) {
		return r;
	}

	return 0;
}