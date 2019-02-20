#include "stdafx.h"
#include "DmaController.h"
#include "MemoryManager.h"
#include "MessageManager.h"

DmaController::DmaController(MemoryManager *memoryManager)
{
	_memoryManager = memoryManager;
}

void DmaController::RunSingleTransfer(DmaChannelConfig &channel)
{
	const uint8_t *transferOffsets = _transferOffset[channel.TransferMode];
	uint8_t transferByteCount = _transferByteCount[channel.TransferMode];

	uint8_t i = 0;
	do {
		if(channel.InvertDirection) {
			uint8_t valToWrite = _memoryManager->Read(0x2100 | channel.DestAddress + transferOffsets[i], MemoryOperationType::DmaRead);
			_memoryManager->Write((channel.SrcBank << 16) | channel.SrcAddress, valToWrite, MemoryOperationType::DmaWrite);
		} else {
			uint8_t valToWrite = _memoryManager->Read((channel.SrcBank << 16) | channel.SrcAddress, MemoryOperationType::DmaRead);
			_memoryManager->Write(0x2100 | channel.DestAddress + transferOffsets[i], valToWrite, MemoryOperationType::DmaWrite);
		}

		if(!channel.FixedTransfer) {
			channel.SrcAddress += channel.Decrement ? -1 : 1;
		}

		channel.TransferSize--;
		transferByteCount--;
		i++;
	} while(channel.TransferSize > 0 && transferByteCount > 0);
}

void DmaController::RunDma(DmaChannelConfig &channel)
{
	do {
		//Manual DMA transfers run to the end of the transfer when started
		RunSingleTransfer(channel);

		//TODO : Run HDMA when needed, between 2 DMA transfers
	} while(channel.TransferSize > 0);
}

void DmaController::InitHdmaChannels()
{
	for(int i = 0; i < 8; i++) {
		DmaChannelConfig &ch = _channel[i];
		ch.HdmaFinished = false;
		if(_hdmaChannels & (1 << i)) {
			//"1. Copy AAddress into Address."
			ch.HdmaTableAddress = ch.SrcAddress;

			//"2. Load $43xA (Line Counter and Repeat) from the table. I believe $00 will terminate this channel immediately."
			ch.HdmaLineCounterAndRepeat = _memoryManager->Read((ch.SrcBank << 16) | ch.HdmaTableAddress, MemoryOperationType::DmaRead);
			ch.HdmaTableAddress++;
			if(ch.HdmaLineCounterAndRepeat == 0) {
				ch.HdmaFinished = true;
			}

			//3. Load Indirect Address, if necessary.
			if(ch.HdmaIndirectAddressing) {
				uint8_t lsb = _memoryManager->Read((ch.SrcBank << 16) | ch.HdmaTableAddress++, MemoryOperationType::DmaRead);
				uint8_t msb = _memoryManager->Read((ch.SrcBank << 16) | ch.HdmaTableAddress++, MemoryOperationType::DmaRead);
				ch.TransferSize = (msb << 8) | lsb;
			}
			
			//4. Set DoTransfer to true.
			ch.DoTransfer = true;
		}
	}
}

void DmaController::RunHdmaTransfer(DmaChannelConfig &channel)
{
	const uint8_t *transferOffsets = _transferOffset[channel.TransferMode];
	uint8_t transferByteCount = _transferByteCount[channel.TransferMode];

	uint32_t srcAddress;
	if(channel.HdmaIndirectAddressing) {
		srcAddress = (channel.HdmaBank << 16) | channel.TransferSize;
	} else {
		srcAddress = (channel.SrcBank << 16) | channel.HdmaTableAddress;
	}

	uint8_t i = 0;
	do {
		if(channel.InvertDirection) {
			uint8_t valToWrite = _memoryManager->Read(0x2100 | channel.DestAddress + transferOffsets[i], MemoryOperationType::DmaRead);
			_memoryManager->Write(srcAddress, valToWrite, MemoryOperationType::DmaWrite);
		} else {
			uint8_t valToWrite = _memoryManager->Read(srcAddress, MemoryOperationType::DmaRead);
			_memoryManager->Write(0x2100 | channel.DestAddress + transferOffsets[i], valToWrite, MemoryOperationType::DmaWrite);
		}

		if(!channel.FixedTransfer) {
			srcAddress = (srcAddress + (channel.Decrement ? -1 : 1)) & 0xFFFFFF;
		}

		transferByteCount--;
		i++;
	} while(transferByteCount > 0);

	if(channel.HdmaIndirectAddressing) {
		channel.TransferSize = srcAddress;
	} else {
		channel.HdmaTableAddress = srcAddress;
	}
}

void DmaController::ProcessHdmaChannels()
{
	if(_hdmaChannels) {
		_hdmaPending = true;

		for(int i = 0; i < 8; i++) {
			DmaChannelConfig &ch = _channel[i];
			if((_hdmaChannels & (1 << i)) == 0 || ch.HdmaFinished) {
				return;
			}

			//1. If DoTransfer is false, skip to step 3.
			if(ch.DoTransfer) {
				//2. For the number of bytes (1, 2, or 4) required for this Transfer Mode...
				RunHdmaTransfer(ch);
			}
				
			//3. Decrement $43xA.
			ch.HdmaLineCounterAndRepeat--;

			//4. Set DoTransfer to the value of Repeat.
			ch.DoTransfer = (ch.HdmaLineCounterAndRepeat & 0x80) != 0;

			//5. If Line Counter is zero...
			if((ch.HdmaLineCounterAndRepeat & 0x7F) == 0) {
				//"a. Read the next byte from Address into $43xA (thus, into both Line Counter and Repeat)."
				ch.HdmaLineCounterAndRepeat = _memoryManager->Read(ch.HdmaTableAddress++, MemoryOperationType::DmaRead);

				//"b. If Addressing Mode is Indirect, read two bytes from Address into Indirect Address(and increment Address by two bytes)."
				if(ch.HdmaIndirectAddressing) {
					if(ch.HdmaLineCounterAndRepeat == 0) {
						//"One oddity: if $43xA is 0 and this is the last active HDMA channel for this scanline, only load one byte for Address, 
						//and use the $00 for the low byte.So Address ends up incremented one less than otherwise expected, and one less CPU Cycle is used."
						uint8_t msb = _memoryManager->Read(ch.HdmaTableAddress++, MemoryOperationType::DmaRead);
						ch.TransferSize = (msb << 8);
					} else {
						uint8_t lsb = _memoryManager->Read(ch.HdmaTableAddress++, MemoryOperationType::DmaRead);
						uint8_t msb = _memoryManager->Read(ch.HdmaTableAddress++, MemoryOperationType::DmaRead);
						ch.TransferSize = (msb << 8) | lsb;
					}
				}

				//"c. If $43xA is zero, terminate this HDMA channel for this frame. The bit in $420c is not cleared, though, so it may be automatically restarted next frame."
				if(ch.HdmaLineCounterAndRepeat == 0) {
					ch.HdmaFinished = true;
				}

				//"d. Set DoTransfer to true."
				ch.DoTransfer = true;
			}
		}
	}
}

void DmaController::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x420B:
			//MDMAEN - DMA Enable
			for(int i = 0; i < 8; i++) {
				if(value & (1 << i)) {
					RunDma(_channel[i]);
				}
			}
			break;

		case 0x420C:
			//HDMAEN - HDMA Enable
			_hdmaChannels = value;
			break;

		case 0x4300: case 0x4310: case 0x4320: case 0x4330: case 0x4340: case 0x4350: case 0x4360: case 0x4370:
		{
			//DMAPx - DMA Control for Channel x
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.InvertDirection = (value & 0x80) != 0;
			channel.HdmaIndirectAddressing = (value & 0x40) != 0;
			channel.Decrement = (value & 0x10) != 0;
			channel.FixedTransfer = (value & 0x08) != 0;
			channel.TransferMode = value & 0x07;
			break;
		}

		case 0x4301: case 0x4311: case 0x4321: case 0x4331: case 0x4341: case 0x4351: case 0x4361: case 0x4371:
		{
			//BBADx - DMA Destination Register for Channel x
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.DestAddress = value;
			break;
		}

		case 0x4302: case 0x4312: case 0x4322: case 0x4332: case 0x4342: case 0x4352: case 0x4362: case 0x4372:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcAddress = (channel.SrcAddress & 0xFF00) | value;
			break;
		}

		case 0x4303: case 0x4313: case 0x4323: case 0x4333: case 0x4343: case 0x4353: case 0x4363: case 0x4373:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcAddress = (channel.SrcAddress & 0xFF) | (value << 8);
			break;
		}

		case 0x4304: case 0x4314: case 0x4324: case 0x4334: case 0x4344: case 0x4354: case 0x4364: case 0x4374:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcBank = value;
			break;
		}

		case 0x4305: case 0x4315: case 0x4325: case 0x4335: case 0x4345: case 0x4355: case 0x4365: case 0x4375:
		{
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.TransferSize = (channel.TransferSize & 0xFF00) | value;
			break;
		}

		case 0x4306: case 0x4316: case 0x4326: case 0x4336: case 0x4346: case 0x4356: case 0x4366: case 0x4376:
		{
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.TransferSize = (channel.TransferSize & 0xFF) | (value << 8);
			break;
		}

		case 0x4307: case 0x4317: case 0x4327: case 0x4337: case 0x4347: case 0x4357: case 0x4367: case 0x4377:
		{
			//DASBx - HDMA Indirect Address bank byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.HdmaBank = value;
			break;
		}

		case 0x4308: case 0x4318: case 0x4328: case 0x4338: case 0x4348: case 0x4358: case 0x4368: case 0x4378:
		{
			//A2AxL - HDMA Table Address low byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.HdmaTableAddress = (channel.HdmaTableAddress & 0xFF00) | value;
			break;
		}

		case 0x4309: case 0x4319: case 0x4329: case 0x4339: case 0x4349: case 0x4359: case 0x4369: case 0x4379:
		{
			//A2AxH - HDMA Table Address high byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.HdmaTableAddress = (value << 8) | (channel.HdmaTableAddress & 0xFF);
			break;
		}

		case 0x430A: case 0x431A: case 0x432A: case 0x433A: case 0x434A: case 0x435A: case 0x436A: case 0x437A:
		{
			//DASBx - HDMA Indirect Address bank byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.HdmaLineCounterAndRepeat = value;
			break;
		}
	}
}
