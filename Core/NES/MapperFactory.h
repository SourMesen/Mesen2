#pragma once
#include "pch.h"

class NesConsole;
class BaseMapper;
class VirtualFile;
struct RomData;
enum class LoadRomResult;

class MapperFactory
{
	private:
		static BaseMapper* GetMapperFromID(RomData &romData);

	public:
		static constexpr uint16_t FdsMapperID = 65535;
		static constexpr uint16_t NsfMapperID = 65534;
		static constexpr uint16_t StudyBoxMapperID = 65533;
		static constexpr uint16_t FamicomNetworkSystemMapperID = 65532;

		static unique_ptr<BaseMapper> InitializeFromFile(NesConsole* console, VirtualFile &romFile, RomData &outRomData, LoadRomResult& result);
};
