#include "pch.h"
#include <string>
#include <cstring>
#include <sstream>
#include "ZipWriter.h"
#include "FolderUtilities.h"

ZipWriter::ZipWriter()
{
}

ZipWriter::~ZipWriter()
{
}

bool ZipWriter::Initialize(string filename)
{
	_zipFilename = filename;
	memset(&_zipArchive, 0, sizeof(mz_zip_archive));
	return mz_zip_writer_init_file(&_zipArchive, _zipFilename.c_str(), 0) != 0;
}

bool ZipWriter::Save()
{
	bool result = mz_zip_writer_finalize_archive(&_zipArchive) != 0;
	result &= mz_zip_writer_end(&_zipArchive) != 0;
	return result;
}

void ZipWriter::AddFile(string filepath, string zipFilename)
{
	if(!mz_zip_writer_add_file(&_zipArchive, zipFilename.c_str(), filepath.c_str(), "", 0, MZ_BEST_COMPRESSION)) {
		std::cout << "mz_zip_writer_add_file() failed!" << std::endl;
	}
}

void ZipWriter::AddFile(vector<uint8_t> &fileData, string zipFilename)
{
	if(!mz_zip_writer_add_mem(&_zipArchive, zipFilename.c_str(), fileData.data(), fileData.size(), MZ_BEST_COMPRESSION)) {
		std::cout << "mz_zip_writer_add_file() failed!" << std::endl;
	}
}

void ZipWriter::AddFile(std::stringstream &filestream, string zipFilename)
{
	filestream.seekg(0, std::ios::end);
	size_t bufferSize = (size_t)filestream.tellg();
	filestream.seekg(0, std::ios::beg);

	vector<uint8_t> buffer(bufferSize);
	filestream.read((char*)buffer.data(), bufferSize);

	AddFile(buffer, zipFilename);
}
