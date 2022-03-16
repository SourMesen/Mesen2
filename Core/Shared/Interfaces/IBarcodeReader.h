#pragma once
#include "stdafx.h"

class IBarcodeReader
{
public:
	virtual void InputBarcode(uint64_t barcode, uint32_t digitCount) = 0;
};
