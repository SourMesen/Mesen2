#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Interfaces/IBarcodeReader.h"
#include "NES/NesConsole.h"
#include "Utilities/Serializer.h"

class DatachBarcodeReader : public BaseControlDevice, public IBarcodeReader
{
private:
	vector<uint8_t> _data;
	uint64_t _insertCycle = 0;
	uint64_t _newBarcode = 0;
	uint32_t _newBarcodeDigitCount = 0;
	NesConsole* _console = nullptr;

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);

		SVVector(_data);
		SV(_insertCycle);
		SV(_newBarcode);
		SV(_newBarcodeDigitCount);
	}

	bool IsRawString() override
	{
		return true;
	}

public:
	DatachBarcodeReader(NesConsole* console) : BaseControlDevice(console->GetEmulator(), ControllerType::DatachBarcodeReader, BaseControlDevice::MapperInputPort)
	{
		_console = console;
	}

	void InternalSetStateFromInput() override
	{
		if(_newBarcodeDigitCount > 0) {
			string barcodeText = std::to_string(_newBarcode);
			//Pad 8 or 13 character barcode with 0s at start
			barcodeText.insert(0, _newBarcodeDigitCount - barcodeText.size(), '0');
			SetTextState(barcodeText);
			_newBarcode = 0;
			_newBarcodeDigitCount = 0;
		}
	}

	void OnAfterSetState() override
	{
		if(GetRawState().State.size() > 0) {
			InitBarcodeData();
		}
	}

	uint8_t GetOutput()
	{
		uint64_t elapsedCycles = _console->GetMasterClock() - _insertCycle;
		uint32_t bitNumber = (uint32_t)(elapsedCycles / 1000);
		if(bitNumber < (uint32_t)_data.size()) {
			return _data[bitNumber];
		} else {
			return 0;
		}
	}

	void InputBarcode(uint64_t barcode, uint32_t digitCount) override
	{
		_newBarcode = barcode;
		_newBarcodeDigitCount = digitCount;
	}		

	void InitBarcodeData() 
	{
		_insertCycle = _console->GetMasterClock();

		static constexpr uint8_t prefixParityType[10][6] = {
			{ 8,8,8,8,8,8 },{ 8,8,0,8,0,0 },
			{ 8,8,0,0,8,0 },{ 8,8,0,0,0,8 },
			{ 8,0,8,8,0,0 },{ 8,0,0,8,8,0 },
			{ 8,0,0,0,8,8 },{ 8,0,8,0,8,0 },
			{ 8,0,8,0,0,8 },{ 8,0,0,8,0,8 }
		};

		static constexpr uint8_t dataLeftOdd[10][7] = {
			{ 8,8,8,0,0,8,0 },{ 8,8,0,0,8,8,0 },
			{ 8,8,0,8,8,0,0 },{ 8,0,0,0,0,8,0 },
			{ 8,0,8,8,8,0,0 },{ 8,0,0,8,8,8,0 },
			{ 8,0,8,0,0,0,0 },{ 8,0,0,0,8,0,0 },
			{ 8,0,0,8,0,0,0 },{ 8,8,8,0,8,0,0 }
		};

		static constexpr uint8_t dataLeftEven[10][7] =	{
			{ 8,0,8,8,0,0,0 },{ 8,0,0,8,8,0,0 },
			{ 8,8,0,0,8,0,0 },{ 8,0,8,8,8,8,0 },
			{ 8,8,0,0,0,8,0 },{ 8,0,0,0,8,8,0 },
			{ 8,8,8,8,0,8,0 },{ 8,8,0,8,8,8,0 },
			{ 8,8,8,0,8,8,0 },{ 8,8,0,8,0,0,0 }
		};

		static constexpr uint8_t dataRight[10][7] = {
			{ 0,0,0,8,8,0,8 },{ 0,0,8,8,0,0,8 },
			{ 0,0,8,0,0,8,8 },{ 0,8,8,8,8,0,8 },
			{ 0,8,0,0,0,8,8 },{ 0,8,8,0,0,0,8 },
			{ 0,8,0,8,8,8,8 },{ 0,8,8,8,0,8,8 },
			{ 0,8,8,0,8,8,8 },{ 0,0,0,8,0,8,8 }
		};

		string barcode = GetTextState();
		vector<uint8_t> code;
		for(uint8_t i = 0; i < barcode.size(); i++) {
			code.push_back(barcode[i] - '0');
		}
		
		_data.clear();

		for(uint32_t i = 0; i < 33; i++) {
			_data.push_back(8);
		}

		_data.push_back(0);
		_data.push_back(8);
		_data.push_back(0);

		uint32_t sum = 0;

		if(barcode.size() == 13) {
			for(uint32_t i = 0; i < 6; i++) {
				bool odd = prefixParityType[code[0]][i] != 0;
				for(uint32_t j = 0; j < 7; j++) {
					_data.push_back(odd ? dataLeftOdd[code[i + 1]][j] : dataLeftEven[code[i + 1]][j]);
				}
			}

			_data.push_back(8);
			_data.push_back(0);
			_data.push_back(8);
			_data.push_back(0);
			_data.push_back(8);
			
			for(uint32_t i = 7; i < 12; i++) {
				for(uint32_t j = 0; j < 7; j++) {
					_data.push_back(dataRight[code[i]][j]);
				}
			}

			for(uint32_t i = 0; i < 12; i++) {
				sum += (i & 1) ? (code[i] * 3) : (code[i] * 1);
			}
		} else {
			for(uint32_t i = 0; i < 4; i++) {
				for(uint32_t j = 0; j < 7; j++) {
					_data.push_back(dataLeftOdd[code[i]][j]);
				}
			}

			_data.push_back(8);
			_data.push_back(0);
			_data.push_back(8);
			_data.push_back(0);
			_data.push_back(8);

			for(uint32_t i = 4; i < 7; i++) {
				for(uint32_t j = 0; j < 7; j++) {
					_data.push_back(dataRight[code[i]][j]);
				}
			}

			for(uint32_t i = 0; i < 7; i++) {
				sum += (i & 1) ? (code[i] * 1) : (code[i] * 3);
			}
		}

		sum = (10 - (sum % 10)) % 10;

		for(uint32_t i = 0; i < 7; i++) {
			_data.push_back(dataRight[sum][i]);
		}

		_data.push_back(0);
		_data.push_back(8);
		_data.push_back(0);

		for(uint32_t i = 0; i < 32; i++) {
			_data.push_back(8);
		}
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}
};