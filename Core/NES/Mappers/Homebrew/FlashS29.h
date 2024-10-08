#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"
#include "Shared/MessageManager.h"

class FlashS29 final : public ISerializable
{
private:
	enum class ChipMode
	{
		WaitingForCommand,
		Write,
		Erase
	};

	enum class ChipModel
	{
		S29AL008,
		S29AL016,
		S29JL032,
		S29GL064S
	};

	ChipModel _model = {};

	ChipMode _mode = ChipMode::WaitingForCommand;
	uint8_t _cycle = 0;
	bool _softwareId = false;
	bool _unlockBypass = false;

	//ROM data and size
	uint8_t* _data = nullptr;
	uint32_t _size = 0;

	void ProcessUnlockBypassMode(uint8_t value)
	{
		if(_cycle == 0) {
			if(value == 0xA0) {
				//1st write of unlock bypass write
				_mode = ChipMode::Write;
			} else if(value == 0x90) {
				//1st write of unlock bypass reset
				_cycle++;
			} else {
				ResetState();
			}
		} else if(_cycle == 1) {
			if(value == 0x00) {
				//2nd write of unlock bypass reset
				_unlockBypass = false;
			}
			ResetState();
		}
	}

protected:
	void Serialize(Serializer& s)
	{
		SV(_mode);
		SV(_cycle);
		SV(_softwareId);
		SV(_unlockBypass);
	}

public:
	FlashS29(uint8_t* data, uint32_t size)
	{
		_data = data;
		_size = size;

		switch(_size) {
			case 0x100000: _model = ChipModel::S29AL008; break;
			case 0x200000: _model = ChipModel::S29AL016; break;
			case 0x400000: _model = ChipModel::S29JL032; break;
			case 0x800000: _model = ChipModel::S29GL064S; break;
		}
	}

	bool IsSoftwareIdMode()
	{
		return _softwareId;
	}

	int16_t Read(uint32_t addr)
	{
		if(_softwareId) {
			switch(addr & 0x1FF) {
				case 0x00: return 0x01;

				case 0x02: {
					switch(_model) {
						case ChipModel::S29AL008: return 0x5B;
						case ChipModel::S29AL016: return 0x49;
						case ChipModel::S29JL032: return 0x7E;
						case ChipModel::S29GL064S: return 0x7E;
						default: return 0xFF;
					}
				}
				case 0x1C: {
					switch(_model) {
						case ChipModel::S29JL032: return 0x0A;
						case ChipModel::S29GL064S: return 0x10;
						default: return 0xFF;
					}
				}

				case 0x1E: {
					switch(_model) {
						case ChipModel::S29JL032: return 0x00;
						case ChipModel::S29GL064S: return 0x00;
						default: return 0xFF;
					}
				}

				default: return 0xFF;
			}
		}
		return -1;
	}

	void ResetState()
	{
		_mode = ChipMode::WaitingForCommand;
		_cycle = 0;
	}

	void Write(uint32_t addr, uint8_t value)
	{
		uint16_t cmd = addr & 0xFFF;
		if(_mode == ChipMode::WaitingForCommand) {
			if(_unlockBypass) {
				ProcessUnlockBypassMode(value);
				return;
			}

			if(_cycle == 0) {
				if(cmd == 0xAAA && value == 0xAA) {
					//1st write, $AAA = $AA
					_cycle++;
				} else if(value == 0xF0) {
					//Software ID exit
					ResetState();
					_softwareId = false;
				}
			} else if(_cycle == 1 && cmd == 0x555 && value == 0x55) {
				//2nd write, $555 = $55
				_cycle++;
			} else if(_cycle == 2 && cmd == 0xAAA) {
				//3rd write, determines command type
				_cycle++;
				switch(value) {
					case 0x20: ResetState(); _unlockBypass = true; break;
					case 0x80: _mode = ChipMode::Erase; break;
					case 0x90: ResetState();  _softwareId = true; break;
					case 0xA0: _mode = ChipMode::Write; break;
					case 0xF0: ResetState(); _softwareId = false; break;
				}
			} else {
				_cycle = 0;
			}
		} else if(_mode == ChipMode::Write) {
			//Write a single byte
			if(addr < _size) {
				_data[addr] &= value;
			}
			ResetState();
		} else if(_mode == ChipMode::Erase) {
			if(_cycle == 3) {
				//4th write for erase command, $AAA = $AA
				if(cmd == 0xAAA && value == 0xAA) {
					_cycle++;
				} else {
					ResetState();
				}
			} else if(_cycle == 4) {
				//5th write for erase command, $555 = $55
				if(cmd == 0x555 && value == 0x55) {
					_cycle++;
				} else {
					ResetState();
				}
			} else if(_cycle == 5) {
				if(cmd == 0xAAA && value == 0x10) {
					//Chip erase
					memset(_data, 0xFF, _size);
				} else if(value == 0x30) {
					//Sector erase
					uint32_t pageCount = _size / 0x10000;
					uint32_t page = addr / 0x10000;
					if(page == pageCount - 1) {
						//Last sector is split into multiple smaller sectors
						vector<int> sectorSizes;
						switch(_model) {
							case ChipModel::S29AL008: sectorSizes = vector<int> { 32, 8, 8, 16 }; break;
							case ChipModel::S29AL016: sectorSizes = vector<int> { 32, 8, 8, 16 }; break;
							case ChipModel::S29JL032: sectorSizes = vector<int> { 8, 8, 8, 8, 8, 8, 8, 8 }; break;
							case ChipModel::S29GL064S: sectorSizes = vector<int> { 8, 8, 8, 8, 8, 8, 8, 8 }; break;
						}

						uint32_t offsetKb = (addr & 0xFFFF) / 1024;
						uint32_t segOffset = 0;
						uint32_t segSize = 0;
						for(int i = 0; i < sectorSizes.size(); i++) {
							if(segOffset + sectorSizes[i] > offsetKb) {
								break;
							}
							segOffset += sectorSizes[i];
							segSize = sectorSizes[i];
						}
						memset(_data + page * 0x10000 + segOffset * 1024, 0xFF, segSize * 1024);
					} else {
						memset(_data + page * 0x10000, 0xFF, 0x10000);
					}
				}
				ResetState();
			}
		}
	}
};