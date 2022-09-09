#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Interfaces/IBarcodeReader.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

class BarcodeBattlerReader : public BaseControlDevice, public IBarcodeReader
{
private:
	static constexpr int StreamSize = 200;
	uint64_t _newBarcode = 0;
	uint32_t _newBarcodeDigitCount = 0;

	uint8_t _barcodeStream[BarcodeBattlerReader::StreamSize] = {};
	uint64_t _insertCycle = 0;

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);

		SVArray(_barcodeStream, BarcodeBattlerReader::StreamSize);
		SV(_newBarcode); SV(_newBarcodeDigitCount); SV(_insertCycle);
	}

	bool IsRawString() override
	{
		return true;
	}

	void InitBarcodeStream()
	{
		vector<uint8_t> state = GetRawState().State;
		string barcodeText(state.begin(), state.end());

		//Signature at the end, needed for code to be recognized
		barcodeText += "EPOCH\xD\xA";
		//Pad to 20 characters with spaces
		barcodeText.insert(0, 20 - barcodeText.size(), ' ');

		int pos = 0;
		vector<uint8_t> bits;
		for(int i = 0; i < 20; i++) {
			_barcodeStream[pos++] = 1;
			for(int j = 0; j < 8; j++) {
				_barcodeStream[pos++] = ~((barcodeText[i] >> j) & 0x01);
			}
			_barcodeStream[pos++] = 0;
		}
	}

public:
	BarcodeBattlerReader(Emulator* emu) : BaseControlDevice(emu, ControllerType::BarcodeBattler, BaseControlDevice::ExpDevicePort)
	{
	}

	void InternalSetStateFromInput() override
	{
		ClearState();

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
			InitBarcodeStream();
			if(_emu) {
				_insertCycle = _emu->GetMasterClock();
			}
		}
	}

	void InputBarcode(uint64_t barcode, uint32_t digitCount) override
	{
		_newBarcode = barcode;
		_newBarcodeDigitCount = digitCount;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			uint64_t elapsedCycles = _emu->GetMasterClock() - _insertCycle;
			uint32_t cyclesPerBit = _emu->GetMasterClockRate() / 1200;

			uint32_t streamPosition = (uint32_t)(elapsedCycles / cyclesPerBit);
			if(streamPosition < BarcodeBattlerReader::StreamSize) {
				return _barcodeStream[streamPosition] << 2;
			}
		}
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}
};