#include "pch.h"
#include "SNES/Coprocessors/BSX/BsxMemoryPack.h"
#include "SNES/SnesConsole.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/Serializer.h"

BsxMemoryPack::BsxMemoryPack(SnesConsole* console, vector<uint8_t>& data, bool persistFlash)
{
	_console = console;
	_orgData = data;
	_dataSize = (uint32_t)data.size();
	_data = new uint8_t[_dataSize];
	console->GetEmulator()->RegisterMemory(MemoryType::BsxMemoryPack, _data, _dataSize);
	_persistFlash = persistFlash;
	memcpy(_data, data.data(), _dataSize);

	_calculatedSize = std::min<uint8_t>(0x0C, (uint8_t)log2(_dataSize >> 10));

	for(uint32_t i = 0; i < _dataSize / 0x1000; i++) {
		_handlers.push_back(unique_ptr<BsxMemoryPackHandler>(new BsxMemoryPackHandler(this, i * 0x1000)));
	}
}

BsxMemoryPack::~BsxMemoryPack()
{
	delete[] _data;
}

void BsxMemoryPack::SaveBattery()
{
	if(_persistFlash) {
		_console->GetEmulator()->GetBatteryManager()->SaveBattery(".bs", _data, _dataSize);
	}
}

void BsxMemoryPack::Serialize(Serializer& s)
{
	SV(_enableCsr);
	SV(_enableEsr);
	SV(_enableVendorInfo);
	SV(_writeByte);
	SV(_command);
	
	if(s.IsSaving()) {
		//Save content of memory pack as an IPS patch
		vector<uint8_t> newData(_data, _data + _dataSize);
		vector<uint8_t> ipsData = IpsPatcher::CreatePatch(_orgData, newData);
		SVVector(ipsData);
	} else {
		//Apply IPS patch to original data and overwrite the current data
		vector<uint8_t> ipsData;
		SVVector(ipsData);

		if(ipsData.size() > 8) {
			vector<uint8_t> output;
			IpsPatcher::PatchBuffer(ipsData, _orgData, output);
			memcpy(_data, output.data(), _dataSize);
		}
	}
}

void BsxMemoryPack::ProcessCommand(uint8_t value, uint32_t page)
{
	_command = (_command << 8) | value;

	switch(value) {
		case 0x00:
		case 0xFF:
			_enableCsr = false;
			_enableEsr = false;
			_enableVendorInfo = false;
			break;

		case 0x10:
		case 0x40:
			_writeByte = true;
			break;

		case 0x70: _enableCsr = true; break;
		case 0x71: _enableEsr = true; break;
		case 0x75: _enableVendorInfo = true; break;
	}

	switch(_command) {
		case 0x20D0: memset(_data + page * 0x10000, 0xFF, 0x10000); break; //Page erase
		case 0xA7D0: memset(_data, 0xFF, _dataSize); break; //Chip erase
	}
}

void BsxMemoryPack::Reset()
{
	_enableCsr = false;
	_enableEsr = false;
	_writeByte = false;
	_enableVendorInfo = false;
	_command = 0;
}

vector<unique_ptr<IMemoryHandler>>& BsxMemoryPack::GetMemoryHandlers()
{
	return _handlers;
}

uint8_t* BsxMemoryPack::DebugGetMemoryPack()
{
	return _data;
}

uint32_t BsxMemoryPack::DebugGetMemoryPackSize()
{
	return _dataSize;
}

BsxMemoryPack::BsxMemoryPackHandler::BsxMemoryPackHandler(BsxMemoryPack* memPack, uint32_t offset) : RamHandler(memPack->_data, offset, memPack->_dataSize, MemoryType::BsxMemoryPack)
{
	_memPack = memPack;
	_page = offset / 0x10000;
}

uint8_t BsxMemoryPack::BsxMemoryPackHandler::Read(uint32_t addr)
{
	if(_offset == 0 && _memPack->_enableEsr) {
		switch(addr & 0xFFF) {
			case 0x0002: return 0xC0;
			case 0x0004: return 0x82;
		}
	}

	if(_memPack->_enableCsr) {
		_memPack->_enableCsr = false;
		return 0x80;
	}

	if(_memPack->_enableVendorInfo && ((addr & 0x7FFF) >= 0x7F00) && ((addr & 0x7FFF) <= 0x7F13)) {
		//Flash cartridge vendor information
		switch(addr & 0xFF) {
			case 0x00: return 0x4d;
			case 0x01: return 0x00;
			case 0x02: return 0x50;
			case 0x03: return 0x00;
			case 0x04: return 0x00;
			case 0x05: return 0x00;
			case 0x06: return 0x10 | _memPack->_calculatedSize; //Memory Pack Type 1
			case 0x07: return 0x00;
			default:   return 0x00;
		}
	}

	return RamHandler::Read(addr);
}

void BsxMemoryPack::BsxMemoryPackHandler::Write(uint32_t addr, uint8_t value)
{
	if(_memPack->_writeByte) {
		uint8_t currentByte = RamHandler::Read(addr);
		RamHandler::Write(addr, value & currentByte);
		_memPack->_writeByte = false;
	} else if(_offset == 0 && (addr & 0xFFF) == 0) {
		_memPack->ProcessCommand(value, _page);
	}
}
