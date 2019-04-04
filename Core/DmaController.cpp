#include "stdafx.h"
#include "DmaController.h"
#include "MemoryManager.h"
#include "MessageManager.h"
#include "../Utilities/Serializer.h"

static constexpr uint8_t _transferByteCount[8] = { 1, 2, 2, 4, 4, 4, 2, 4 };
static constexpr uint8_t _transferOffset[8][4] = {
	{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 },
	{ 0, 1, 2, 3 }, { 0, 1, 0, 1 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 }
};

DmaController::DmaController(MemoryManager *memoryManager)
{
	_memoryManager = memoryManager;
	Reset();

	//Power on values
	for(int j = 0; j < 8; j++) {
		for(int i = 0; i <= 0x0A; i++) {
			Write(0x4300 | i | (j << 4), 0xFF);
		}
	}
}

void DmaController::Reset()
{
	_hdmaChannels = 0;
}

bool DmaController::HasNmiIrqDelay()
{
	if(_nmiIrqDelayCounter > 0) {
		_nmiIrqDelayCounter--;
		return true;
	}
	return false;
}

void DmaController::CopyDmaByte(uint32_t addressBusA, uint16_t addressBusB, bool fromBtoA)
{
	if(fromBtoA) {
		if(addressBusB != 0x2180 || !_memoryManager->IsWorkRam(addressBusA)) {
			uint8_t valToWrite = _memoryManager->ReadDma(addressBusB, false);
			_memoryManager->WriteDma(addressBusA, valToWrite, true);
		} else {
			//$2180->WRAM do cause a write to occur (but no read), but the value written is invalid
			_memoryManager->IncrementMasterClockValue<4>();
			_memoryManager->WriteDma(addressBusA, 0xFF, true);
		}
	} else {
		if(addressBusB != 0x2180 || !_memoryManager->IsWorkRam(addressBusA)) {
			uint8_t valToWrite = _memoryManager->ReadDma(addressBusA, true);
			_memoryManager->WriteDma(addressBusB, valToWrite, false);
		} else {
			//WRAM->$2180 does not cause a write to occur
			_memoryManager->IncrementMasterClockValue<8>();
		}
	}
}

void DmaController::RunSingleTransfer(DmaChannelConfig &channel)
{
	const uint8_t *transferOffsets = _transferOffset[channel.TransferMode];
	uint8_t transferByteCount = _transferByteCount[channel.TransferMode];

	uint8_t i = 0;
	do {
		CopyDmaByte(
			(channel.SrcBank << 16) | channel.SrcAddress,
			0x2100 | (channel.DestAddress + transferOffsets[i]),
			channel.InvertDirection
		);

		if(!channel.FixedTransfer) {
			channel.SrcAddress += channel.Decrement ? -1 : 1;
		}

		channel.TransferSize--;
		transferByteCount--;
		i++;
	} while(channel.TransferSize > 0 && transferByteCount > 0 && !channel.InterruptedByHdma);
}

void DmaController::RunDma(DmaChannelConfig &channel)
{
	if(channel.InterruptedByHdma) {
		return;
	}

	do {
		//Manual DMA transfers run to the end of the transfer when started
		RunSingleTransfer(channel);

		//TODO : Run HDMA when needed, between 2 DMA transfers
	} while(channel.TransferSize > 0 && !channel.InterruptedByHdma);
}

void DmaController::InitHdmaChannels()
{
	if(_hdmaChannels) {
		//"The overhead is ~18 master cycles"
		_memoryManager->IncrementMasterClockValue<18>();
	}

	for(int i = 0; i < 8; i++) {
		DmaChannelConfig &ch = _channel[i];

		//Reset internal flags on every frame
		ch.HdmaFinished = false;
		ch.DoTransfer = false; //not resetting this causes graphical glitches in some games (Aladdin, Super Ghouls and Ghosts)

		if(_hdmaChannels & (1 << i)) {
			//"1. Copy AAddress into Address."
			ch.HdmaTableAddress = ch.SrcAddress;
			ch.InterruptedByHdma = true;

			//"2. Load $43xA (Line Counter and Repeat) from the table. I believe $00 will terminate this channel immediately."
			ch.HdmaLineCounterAndRepeat = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress, true);
			ch.HdmaTableAddress++;
			if(ch.HdmaLineCounterAndRepeat == 0) {
				ch.HdmaFinished = true;
			}

			//3. Load Indirect Address, if necessary.
			if(ch.HdmaIndirectAddressing) {
				uint8_t lsb = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);
				uint8_t msb = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);
				ch.TransferSize = (msb << 8) | lsb;

				//"and 24 master cycles for each channel set for indirect HDMA"
				_memoryManager->IncrementMasterClockValue<16>();
			} else {
				//"plus 8 master cycles for each channel set for direct HDMA"
				_memoryManager->IncrementMasterClockValue<4>();
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
	channel.InterruptedByHdma = true;

	uint8_t i = 0;
	if(channel.HdmaIndirectAddressing) {
		do {
			CopyDmaByte(
				(channel.HdmaBank << 16) | channel.TransferSize,
				0x2100 | (channel.DestAddress + transferOffsets[i]),
				channel.InvertDirection
			);
			channel.TransferSize++;
			transferByteCount--;
			i++;
		} while(transferByteCount > 0);
	} else {
		do {
			CopyDmaByte(
				(channel.SrcBank << 16) | channel.HdmaTableAddress,
				0x2100 | (channel.DestAddress + transferOffsets[i]),
				channel.InvertDirection
			);
			channel.HdmaTableAddress++;
			transferByteCount--;
			i++;
		} while(transferByteCount > 0);
	}
}

void DmaController::ProcessHdmaChannels()
{
	bool needOverhead = true;
	if(_hdmaChannels) {
		_hdmaPending = true;

		for(int i = 0; i < 8; i++) {
			if(_hdmaChannels & (1 << i)) {
				_channel[i].InterruptedByHdma = true;
			}
		}

		for(int i = 0; i < 8; i++) {
			DmaChannelConfig &ch = _channel[i];
			if((_hdmaChannels & (1 << i)) == 0 || ch.HdmaFinished) {
				continue;
			}

			if(needOverhead) {
				//"For each scanline during which HDMA is active (i.e.at least one channel has not yet terminated for the frame), there are ~18 master cycles overhead."
				_memoryManager->IncrementMasterClockValue<8>();
				needOverhead = false;
			}

			//"Each active channel incurs another 8 master cycles overhead for every scanline"
			_memoryManager->IncrementMasterClockValue<8>();

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
				ch.HdmaLineCounterAndRepeat = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);

				//"b. If Addressing Mode is Indirect, read two bytes from Address into Indirect Address(and increment Address by two bytes)."
				if(ch.HdmaIndirectAddressing) {
					if(ch.HdmaLineCounterAndRepeat == 0) {
						//"One oddity: if $43xA is 0 and this is the last active HDMA channel for this scanline, only load one byte for Address, 
						//and use the $00 for the low byte.So Address ends up incremented one less than otherwise expected, and one less CPU Cycle is used."
						uint8_t msb = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);
						ch.TransferSize = (msb << 8);
					} else {
						uint8_t lsb = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);
						uint8_t msb = _memoryManager->ReadDma((ch.SrcBank << 16) | ch.HdmaTableAddress++, true);
						ch.TransferSize = (msb << 8) | lsb;
					}

					//"If a new indirect address is required, 16 master cycles are taken to load it."
					_memoryManager->IncrementMasterClockValue<8>(); //minus 8 because the ReadDmas call will increment it by 4 twice
				}

				//"c. If $43xA is zero, terminate this HDMA channel for this frame. The bit in $420c is not cleared, though, so it may be automatically restarted next frame."
				if(ch.HdmaLineCounterAndRepeat == 0) {
					ch.HdmaFinished = true;
				}

				//"d. Set DoTransfer to true."
				ch.DoTransfer = true;
			}
		}

		//When DMA runs, the next instruction will not check the NMI/IRQ flags, which allows 2 instructions to run after DMA
		_nmiIrqDelayCounter = 2;
	}
}

void DmaController::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x420B: {
			//MDMAEN - DMA Enable
			if(value) {
				//"after the pause, wait 2-8 master cycles to reach a whole multiple of 8 master cycles since reset"
				uint8_t clocksToWait = 8 - (_memoryManager->GetMasterClock() % 8);
				_memoryManager->IncrementMasterClockValue(clocksToWait ? clocksToWait : 8);

				//"and an extra 8 master cycles overhead for the whole thing"
				_memoryManager->IncrementMasterClockValue<8>();
				for(int i = 0; i < 8; i++) {
					_channel[i].InterruptedByHdma = false;
				}

				for(int i = 0; i < 8; i++) {
					if(value & (1 << i)) {
						//"Then perform the DMA: 8 master cycles overhead and 8 master cycles per byte per channel"
						_memoryManager->IncrementMasterClockValue<8>();
						RunDma(_channel[i]);
					}
				}
				//"Then wait 2-8 master cycles to reach a whole number of CPU Clock cycles since the pause"
				clocksToWait = 8 - (_memoryManager->GetMasterClock() % 8);
				_memoryManager->IncrementMasterClockValue(clocksToWait ? clocksToWait : 8);

				//When DMA runs, the next instruction will not check the NMI/IRQ flags, which allows 2 instructions to run after DMA
				_nmiIrqDelayCounter = 2;
			}
			break;
		}

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
			channel.UnusedFlag = (value & 0x20) != 0;
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

uint8_t DmaController::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4300: case 0x4310: case 0x4320: case 0x4330: case 0x4340: case 0x4350: case 0x4360: case 0x4370:
		{
			//DMAPx - DMA Control for Channel x
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return (
				(channel.InvertDirection ? 0x80 : 0) |
				(channel.HdmaIndirectAddressing ? 0x40 : 0) |
				(channel.UnusedFlag ? 0x20 : 0) |
				(channel.Decrement ? 0x10 : 0) |
				(channel.FixedTransfer ? 0x08 : 0) |
				(channel.TransferMode & 0x07)
			);
		}

		case 0x4301: case 0x4311: case 0x4321: case 0x4331: case 0x4341: case 0x4351: case 0x4361: case 0x4371:
		{
			//BBADx - DMA Destination Register for Channel x
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.DestAddress;
		}

		case 0x4302: case 0x4312: case 0x4322: case 0x4332: case 0x4342: case 0x4352: case 0x4362: case 0x4372:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.SrcAddress & 0xFF;
		}

		case 0x4303: case 0x4313: case 0x4323: case 0x4333: case 0x4343: case 0x4353: case 0x4363: case 0x4373:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return (channel.SrcAddress >> 8) & 0xFF;
		}

		case 0x4304: case 0x4314: case 0x4324: case 0x4334: case 0x4344: case 0x4354: case 0x4364: case 0x4374:
		{
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.SrcBank;
		}

		case 0x4305: case 0x4315: case 0x4325: case 0x4335: case 0x4345: case 0x4355: case 0x4365: case 0x4375:
		{
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.TransferSize & 0xFF;
		}

		case 0x4306: case 0x4316: case 0x4326: case 0x4336: case 0x4346: case 0x4356: case 0x4366: case 0x4376:
		{
			//DASxL - DMA Size / HDMA Indirect Address low byte(x = 0 - 7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return (channel.TransferSize >> 8) & 0xFF;
		}

		case 0x4307: case 0x4317: case 0x4327: case 0x4337: case 0x4347: case 0x4357: case 0x4367: case 0x4377:
		{
			//DASBx - HDMA Indirect Address bank byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.HdmaBank;
		}

		case 0x4308: case 0x4318: case 0x4328: case 0x4338: case 0x4348: case 0x4358: case 0x4368: case 0x4378:
		{
			//A2AxL - HDMA Table Address low byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.HdmaTableAddress & 0xFF;
		}

		case 0x4309: case 0x4319: case 0x4329: case 0x4339: case 0x4349: case 0x4359: case 0x4369: case 0x4379:
		{
			//A2AxH - HDMA Table Address high byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return (channel.HdmaTableAddress >> 8) & 0xFF;
		}

		case 0x430A: case 0x431A: case 0x432A: case 0x433A: case 0x434A: case 0x435A: case 0x436A: case 0x437A:
		{
			//DASBx - HDMA Indirect Address bank byte (x=0-7)
			DmaChannelConfig &channel = _channel[(addr & 0x70) >> 4];
			return channel.HdmaLineCounterAndRepeat;
		}
	}
	return _memoryManager->GetOpenBus();
}

void DmaController::Serialize(Serializer &s)
{
	s.Stream(_hdmaPending, _hdmaChannels, _nmiIrqDelayCounter);
	for(int i = 0; i < 8; i++) {
		s.Stream(
			_channel[i].Decrement, _channel[i].DestAddress, _channel[i].DoTransfer, _channel[i].FixedTransfer,
			_channel[i].HdmaBank, _channel[i].HdmaFinished, _channel[i].HdmaIndirectAddressing,
			_channel[i].HdmaLineCounterAndRepeat, _channel[i].HdmaTableAddress, _channel[i].InterruptedByHdma,
			_channel[i].InvertDirection, _channel[i].SrcAddress, _channel[i].SrcBank, _channel[i].TransferMode,
			_channel[i].TransferSize, _channel[i].UnusedFlag
		);
	}
}
