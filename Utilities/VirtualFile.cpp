#include "stdafx.h"
#include <algorithm>
#include <iterator>
#include "VirtualFile.h"
#include "Utilities/sha1.h"
#include "Utilities/ArchiveReader.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/BpsPatcher.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/Patches/UpsPatcher.h"

const std::initializer_list<string> VirtualFile::RomExtensions = {
	".nes", ".fds", ".unif", ".unf", ".nsf", ".nsfe", ".studybox",
	".sfc", ".swc", ".fig", ".smc", ".bs", ".spc",
	".gb", ".gbc", ".gbs",
	".pce", "cue"
};

VirtualFile::VirtualFile()
{
}

VirtualFile::VirtualFile(const string& archivePath, const string innerFile)
{
	_path = archivePath;
	_innerFile = innerFile;
}

VirtualFile::VirtualFile(const string& file)
{
	vector<string> tokens = StringUtilities::Split(file, '\x1');
	_path = tokens[0];
	if(tokens.size() > 1) {
		_innerFile = tokens[1];
		if(tokens.size() > 2) {
			try {
				_innerFileIndex = std::stoi(tokens[2]);
			} catch(std::exception&) {}
		}
	}
}

VirtualFile::VirtualFile(const void* buffer, size_t bufferSize, string fileName)
{
	_path = fileName;

	_data.resize(bufferSize);
	memcpy(_data.data(), buffer, bufferSize);
}

VirtualFile::VirtualFile(std::istream& input, string filePath)
{
	_path = filePath;
	FromStream(input, _data);
}

VirtualFile::operator std::string() const
{
	if(_innerFile.empty()) {
		return _path;
	} else if(_path.empty()) {
		throw std::runtime_error("Cannot convert to string");
	} else {
		if(_innerFileIndex >= 0) {
			return _path + "\x1" + _innerFile + "\x1" + std::to_string(_innerFileIndex);
		} else {
			return _path + "\x1" + _innerFile;
		}
	}
}

void VirtualFile::FromStream(std::istream& input, vector<uint8_t>& output)
{
	input.seekg(0, std::ios::end);
	uint32_t fileSize = (uint32_t)input.tellg();
	input.seekg(0, std::ios::beg);

	output.resize(fileSize, 0);
	input.read((char*)output.data(), fileSize);
}

void VirtualFile::LoadFile()
{
	if(_data.size() == 0) {
		if(!_innerFile.empty()) {
			unique_ptr<ArchiveReader> reader = ArchiveReader::GetReader(_path);
			if(reader) {
				if(_innerFileIndex >= 0) {
					vector<string> filelist = reader->GetFileList(VirtualFile::RomExtensions);
					if((int32_t)filelist.size() > _innerFileIndex) {
						reader->ExtractFile(filelist[_innerFileIndex], _data);
					}
				} else {
					reader->ExtractFile(_innerFile, _data);
				}
			}
		} else {
			ifstream input(_path, std::ios::in | std::ios::binary);
			if(input.good()) {
				FromStream(input, _data);
			}
		}
	}
}

bool VirtualFile::IsValid()
{
	if(_data.size() > 0) {
		return true;
	}

	if(!_innerFile.empty()) {
		unique_ptr<ArchiveReader> reader = ArchiveReader::GetReader(_path);
		if(reader) {
			vector<string> filelist = reader->GetFileList();
			if(_innerFileIndex >= 0) {
				if((int32_t)filelist.size() > _innerFileIndex) {
					return true;
				}
			} else {
				return std::find(filelist.begin(), filelist.end(), _innerFile) != filelist.end();
			}
		}
	} else {
		ifstream input(_path, std::ios::in | std::ios::binary);
		if(input) {
			return true;
		}
	}
	return false;
}

string VirtualFile::GetFilePath()
{
	return _path;
}

string VirtualFile::GetFolderPath()
{
	return FolderUtilities::GetFolderName(_path);
}

string VirtualFile::GetFileName()
{
	return _innerFile.empty() ? FolderUtilities::GetFilename(_path, true) : _innerFile;
}

string VirtualFile::GetFileExtension()
{
	return FolderUtilities::GetExtension(GetFileName());
}

string VirtualFile::GetSha1Hash()
{
	LoadFile();
	return SHA1::GetHash(_data);
}

size_t VirtualFile::GetSize()
{
	if(_data.size() > 0) {
		return _data.size();
	} else {
		ifstream input(_path, std::ios::in | std::ios::binary);
		if(input) {
			input.seekg(0, std::ios::end);
			return (uint32_t)input.tellg();
		}
		return 0;
	}
}

bool VirtualFile::ReadFile(vector<uint8_t>& out)
{
	LoadFile();
	if(_data.size() > 0) {
		out.resize(_data.size(), 0);
		std::copy(_data.begin(), _data.end(), out.begin());
		return true;
	}
	return false;
}

bool VirtualFile::ReadFile(std::stringstream& out)
{
	LoadFile();
	if(_data.size() > 0) {
		out.write((char*)_data.data(), _data.size());
		return true;
	}
	return false;
}

bool VirtualFile::ReadFile(uint8_t* out, uint32_t expectedSize)
{
	LoadFile();
	if(_data.size() == expectedSize) {
		memcpy(out, _data.data(), _data.size());
		return true;
	}
	return false;
}

uint8_t VirtualFile::ReadByte(uint32_t offset)
{
	LoadFile();
	return _data[offset];
}

bool VirtualFile::ApplyPatch(VirtualFile& patch)
{
	//Apply patch file
	bool result = false;
	if(IsValid() && patch.IsValid()) {
		patch.LoadFile();
		LoadFile();
		if(patch._data.size() >= 5) {
			vector<uint8_t> patchedData;
			std::stringstream ss;
			patch.ReadFile(ss);

			if(memcmp(patch._data.data(), "PATCH", 5) == 0) {
				result = IpsPatcher::PatchBuffer(ss, _data, patchedData);
			} else if(memcmp(patch._data.data(), "UPS1", 4) == 0) {
				result = UpsPatcher::PatchBuffer(ss, _data, patchedData);
			} else if(memcmp(patch._data.data(), "BPS1", 4) == 0) {
				result = BpsPatcher::PatchBuffer(ss, _data, patchedData);
			}
			if(result) {
				_data = patchedData;
			}
		}
	}
	return result;
}