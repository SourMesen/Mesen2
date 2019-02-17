#include "stdafx.h"
#include "DmaController.h"
#include "MemoryManager.h"
#include "MessageManager.h"

DmaController::DmaController(MemoryManager *memoryManager)
{
	_memoryManager = memoryManager;
}

void DmaController::RunSingleTransfer(DmaChannelConfig &channel, uint32_t &bytesLeft)
{
	const uint8_t *transferOffsets = _transferOffset[channel.TransferMode];
	uint8_t transferByteCount = _transferByteCount[channel.TransferMode];

	uint8_t i = 0;
	while(bytesLeft > 0 && transferByteCount > 0) {
		if(channel.InvertDirection) {
			uint8_t valToWrite = _memoryManager->Read(0x2100 | channel.DestAddress + transferOffsets[i], MemoryOperationType::DmaRead);
			_memoryManager->Write(channel.SrcAddress, valToWrite, MemoryOperationType::DmaWrite);
		} else {
			uint8_t valToWrite = _memoryManager->Read(channel.SrcAddress, MemoryOperationType::DmaRead);
			_memoryManager->Write(0x2100 | channel.DestAddress + transferOffsets[i], valToWrite, MemoryOperationType::DmaWrite);
		}

		if(!channel.FixedTransfer) {
			channel.SrcAddress = (channel.SrcAddress + (channel.Decrement ? -1 : 1)) & 0xFFFFFF;
		}

		transferByteCount--;
		bytesLeft--;
		i++;
	}
}

void DmaController::RunDma(DmaChannelConfig &channel)
{
	//"Note, however, that writing $0000 to this register actually results in a transfer of $10000 bytes, not 0."
	uint32_t bytesLeft = channel.TransferSize ? channel.TransferSize : 0x10000;

	MessageManager::Log("Run DMA: " + HexUtilities::ToHex(channel.DestAddress) + " -> " + HexUtilities::ToHex(channel.SrcAddress) + " Bytes: " + std::to_string(bytesLeft));

	while(bytesLeft > 0) {
		//Manual DMA transfers run to the end of the transfer when started
		RunSingleTransfer(channel, bytesLeft);
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
			if(value > 0) {
				MessageManager::DisplayMessage("Debug", "Unsupported HDMA operation");
			}
			break;

		case 0x4300: case 0x4310: case 0x4320: case 0x4330: case 0x4340: case 0x4350: case 0x4360: case 0x4370:
		{
			//DMAPx - DMA Control for Channel x
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.InvertDirection = (value & 0x80) != 0;
			channel.HdmaPointers = (value & 0x40) != 0;
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

		case 0x4302: case 0x4312: case 0x4322: case 0x4332: case 0x4342: case 0x4352: case 0x4362: case 0x4372:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcAddress = (channel.SrcAddress & 0xFFFF00) | value;
			break;
		}

		case 0x4303: case 0x4313: case 0x4323: case 0x4333: case 0x4343: case 0x4353: case 0x4363: case 0x4373:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcAddress = (channel.SrcAddress & 0xFF00FF) | (value << 8);
			break;
		}

		case 0x4304: case 0x4314: case 0x4324: case 0x4334: case 0x4344: case 0x4354: case 0x4364: case 0x4374:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			channel.SrcAddress = (channel.SrcAddress & 0x00FFFF) | (value << 16);
			break;
		}
	}
}
