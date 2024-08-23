#include "pch.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaRomPrefetch.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

void GbaDmaController::Init(GbaCpu* cpu, GbaMemoryManager* memoryManager, GbaRomPrefetch* prefetcher)
{
	_cpu = cpu;
	_memoryManager = memoryManager;
	_prefetcher = prefetcher;
}

GbaDmaControllerState& GbaDmaController::GetState()
{
	return _state;
}

bool GbaDmaController::IsVideoCaptureDmaEnabled()
{
	return _state.Ch[3].Enabled && _state.Ch[3].Trigger == GbaDmaTrigger::Special;
}

int8_t GbaDmaController::DebugGetActiveChannel()
{
	return _dmaActiveChannel;
}

void GbaDmaController::TriggerDmaChannel(GbaDmaTrigger trigger, uint8_t channel, bool forceStop)
{
	GbaDmaChannel& ch = _state.Ch[channel];
	if(ch.Enabled && ch.Trigger == trigger) {
		if(forceStop) {
			ch.Repeat = false;
		}

		ch.Pending = true;
		_dmaPending = true;

		if(_dmaActiveChannel < 0 && !_dmaStartDelay) {
			//CPU runs for 2 more cycles before pausing for DMA
			_dmaStartDelay = 3;
			_memoryManager->SetPendingUpdateFlag();
		}
	}
}

void GbaDmaController::TriggerDma(GbaDmaTrigger trigger)
{
	for(int i = 0; i < 4; i++) {
		TriggerDmaChannel(trigger, i);
	}
}

void GbaDmaController::RunPendingDma(bool allowStartDma)
{
	if(_dmaStartDelay) {
		_dmaStartDelay--;
		if(_dmaStartDelay) {
			return;
		}
	}

	if(!allowStartDma || _memoryManager->IsBusLocked()) {
		//DMA can only start between cpu read/write cycles
		//and can't start if the bus is locked by the cpu (swap instruction)
		//Delay until DMA can start
		_dmaStartDelay++;
		return;
	}

	//Before starting DMA, an additional idle cycle executes (CPU is blocked during this)
	_memoryManager->ProcessInternalCycle();

	for(int i = 0; i < 4; i++) {
		if(_state.Ch[i].Pending) {
			RunDma(_state.Ch[i], i);
		}
	}

	//After stopping DMA, an additional idle cycle executes (CPU is blocked during this)
	_memoryManager->ProcessInternalCycle();
}

void GbaDmaController::RunDma(GbaDmaChannel& ch, uint8_t chIndex)
{
	uint16_t length = ch.LenLatch ? ch.LenLatch : (chIndex == 3 ? 0xFFFF : 0x3FFF);
	bool wordTransfer = ch.WordTransfer;
	GbaDmaAddrMode srcMode = ch.SrcMode;
	GbaDmaAddrMode destMode = ch.DestMode;
	if(ch.Trigger == GbaDmaTrigger::Special && chIndex != 3) {
		//Audio channel DMA forces these settings (games break otherwise)
		length = 4;
		wordTransfer = true;
		destMode = GbaDmaAddrMode::Fixed;
	}

	uint8_t offset = wordTransfer ? 4 : 2;
	GbaAccessModeVal mode = (wordTransfer ? GbaAccessMode::Word : GbaAccessMode::HalfWord) | GbaAccessMode::Dma;

	/*MessageManager::Log("run dma - $" + HexUtilities::ToHex(ch.SrcLatch) + " -> $" + HexUtilities::ToHex(ch.DestLatch) + " - len: " +
		HexUtilities::ToHex(length) + " - " +
		(wordTransfer ? "W" : "H") +
		(ch.SrcMode == GbaDmaAddrMode::Increment ? "I" : (ch.SrcMode == GbaDmaAddrMode::Decrement ? "D" : "F")) +
		(destMode == GbaDmaAddrMode::Increment ? "I" : (destMode == GbaDmaAddrMode::Decrement ? "D" : (destMode == GbaDmaAddrMode::IncrementReload ? "R" : "F")))
	);
	*/

	//Ignore lower bits for misaligned DMAs (some games break because of this)
	if(wordTransfer) {
		ch.DestLatch &= ~0x03;
		ch.SrcLatch &= ~0x03;
	} else {
		ch.DestLatch &= ~0x01;
		ch.SrcLatch &= ~0x01;
	}

	ch.Active = true;

	uint8_t srcBank = ch.SrcLatch >> 24;
	bool isRomSrc = srcBank >= 0x08 && srcBank <= 0x0D;
	uint32_t srcAddr = ch.SrcLatch;

	_dmaActiveChannel = chIndex;

	while(length-- > 0) {
		uint32_t value;
		if(srcAddr >= 0x2000000) {
			if(srcAddr & 0x8000000) {
				//DMA accessed ROM, suspend the prefetcher
				_prefetcher->SetSuspendState(true);
			}

			value = ch.ReadValue = _memoryManager->Read(mode, srcAddr);
			if(isRomSrc) {
				//If a DMA reads from ROM (cart) and writes to ROM (cart), the first ROM write will be sequential
				mode |= GbaAccessMode::Sequential;
			}
			if(!wordTransfer) {
				//Value kept in buffer is mirrored across both half-words when transfering half-words
				//Needed to pass mgba suite tests that perform a half-word transfer before performing
				//a DMA that tries to load data from the boot rom
				ch.ReadValue |= ch.ReadValue << 16;
			}
		} else {
			//Access to boot rom region is not allowed, return the previous value read by DMA
			_memoryManager->ProcessInternalCycle();
			if(wordTransfer) {
				value = ch.ReadValue;
			} else {
				//For half-word transfers, the value written depends on the destination address
				value = ch.ReadValue >> ((ch.DestLatch & 0x02) << 3);
			}
		}

		if(ch.DestLatch & 0x8000000) {
			//DMA accessed ROM, suspend the prefetcher
			_prefetcher->SetSuspendState(true);
		}

		_memoryManager->Write(mode, ch.DestLatch, value);
		mode |= GbaAccessMode::Sequential;

		switch(destMode) {
			case GbaDmaAddrMode::Increment: ch.DestLatch += offset; break;
			case GbaDmaAddrMode::Decrement: ch.DestLatch -= offset; break;
			case GbaDmaAddrMode::Fixed: break;
			case GbaDmaAddrMode::IncrementReload: ch.DestLatch += offset; break;
		}

		switch(srcMode) {
			case GbaDmaAddrMode::Increment: ch.SrcLatch += offset; break;
			case GbaDmaAddrMode::Decrement: ch.SrcLatch -= offset; break;
			case GbaDmaAddrMode::Fixed: break;
			case GbaDmaAddrMode::IncrementReload: break;
		}

		if(ch.SrcLatch >= 0x8000000 && ch.SrcLatch < 0xE000000) {
			if(!isRomSrc && ch.Destination >= 0x8000000 && ch.Destination < 0xE000000) {
				//When src moves from non-rom to rom and destination is in rom, use the destination address
				//Passes "burst-into-tears" test (but might be incorrect?)
				srcAddr = ch.Destination;
				isRomSrc = true;
			}

			if((srcMode == GbaDmaAddrMode::Decrement || srcMode == GbaDmaAddrMode::Fixed) && (srcAddr & 0x1FFFF) == 0) {
				//When on a 0x20000 boundary non-sequential timing is used (passes 128kb-boundary test)
				mode &= ~GbaAccessMode::Sequential;
			}

			//While the address is in the ROM region, all reads are sequential
			//even if the channel is set to decrement/fixed
			srcAddr += offset;
		} else {
			srcAddr = ch.SrcLatch;
		}

		if(_dmaPending) {
			//Check if channels with higher priority need to run
			for(int i = 0; i < chIndex; i++) {
				if(_state.Ch[i].Pending) {
					RunDma(_state.Ch[i], i);

					//Mark next access as non-sequential?
					mode &= ~GbaAccessMode::Sequential;
				}
			}
			_dmaActiveChannel = chIndex;
		}
	}

	_dmaActiveChannel = -1;
	_prefetcher->SetSuspendState(false);

	ch.Active = false;
	ch.Pending = false;
	
	_dmaPending = false;
	for(int i = 0; i < 4; i++) {
		_dmaPending |= _state.Ch[i].Pending;
	}

	if(!ch.Repeat || ch.Trigger == GbaDmaTrigger::Immediate) {
		ch.Enabled = false;
		ch.Control &= ~0x8000;
	} else {
		if(destMode == GbaDmaAddrMode::IncrementReload) {
			ch.DestLatch = ch.Destination;
		}
	}

	//Next access after DMA is never sequential - passes "force-nseq-access" test
	_cpu->ClearSequentialFlag();

	if(ch.IrqEnabled) {
		_memoryManager->SetIrqSource((GbaIrqSource)((int)GbaIrqSource::DmaChannel0 << chIndex));
	}
}

uint8_t GbaDmaController::ReadRegister(uint32_t addr)
{
	GbaDmaChannel& ch = _state.Ch[(addr - 0xB0) / 12];

	switch(addr) {
		case 0xB8: case 0xC4: case 0xD0: case 0xDC:
		case 0xB9: case 0xC5: case 0xD1: case 0xDD:
			return 0;

		case 0xBA: case 0xC6: case 0xD2: case 0xDE: return BitUtilities::GetBits<0>(ch.Control);
		case 0xBB: case 0xC7: case 0xD3: case 0xDF: return BitUtilities::GetBits<8>(ch.Control);
			
		default:
			//MessageManager::Log("Read unknown DMA register: " + HexUtilities::ToHex32(addr));
			return _memoryManager->GetOpenBus(addr);
	}
}

void GbaDmaController::WriteRegister(uint32_t addr, uint8_t value)
{
	uint8_t chIndex = (addr - 0xB0) / 12;
	GbaDmaChannel& ch = _state.Ch[chIndex];

	switch(addr) {
		case 0xB0: case 0xBC: case 0xC8: case 0xD4: BitUtilities::SetBits<0>(ch.Source, value); break;
		case 0xB1: case 0xBD: case 0xC9: case 0xD5: BitUtilities::SetBits<8>(ch.Source, value); break;
		case 0xB2: case 0xBE: case 0xCA: case 0xD6: BitUtilities::SetBits<16>(ch.Source, value); break;
		case 0xB3: case 0xBF: case 0xCB: case 0xD7: BitUtilities::SetBits<24>(ch.Source, value & (chIndex == 0 ? 0x07 : 0x0F)); break;

		case 0xB4: case 0xC0: case 0xCC: case 0xD8: BitUtilities::SetBits<0>(ch.Destination, value); break;
		case 0xB5: case 0xC1: case 0xCD: case 0xD9: BitUtilities::SetBits<8>(ch.Destination, value); break;
		case 0xB6: case 0xC2: case 0xCE: case 0xDA: BitUtilities::SetBits<16>(ch.Destination, value); break;
		case 0xB7: case 0xC3: case 0xCF: case 0xDB: BitUtilities::SetBits<24>(ch.Destination, value & (chIndex == 3 ? 0x0F : 0x07)); break;

		case 0xB8: case 0xC4: case 0xD0: case 0xDC: BitUtilities::SetBits<0>(ch.Length, value); break;
		case 0xB9: case 0xC5: case 0xD1: case 0xDD: BitUtilities::SetBits<8>(ch.Length, value & (chIndex == 3 ? 0xFF : 0x3F)); break;

		case 0xBA: case 0xC6: case 0xD2: case 0xDE:
			value &= 0xE0;
			BitUtilities::SetBits<0>(ch.Control, value);
			ch.DestMode = (GbaDmaAddrMode)((value >> 5) & 0x03);
			ch.SrcMode = (GbaDmaAddrMode)(((value >> 7) & 0x01) | ((ch.Control & 0x100) >> 7));
			break;

		case 0xBB: case 0xC7: case 0xD3: case 0xDF: {
			if(chIndex != 3) {
				//drq mode bit not available on channels 0 to 2
				value &= ~0x08;
			}

			BitUtilities::SetBits<8>(ch.Control, value);
			ch.SrcMode = (GbaDmaAddrMode)(((ch.Control & 0x80) >> 7) | ((value & 0x01) << 1));
			ch.Repeat = (value & 0x02);
			ch.WordTransfer = (value & 0x04);
			ch.DrqMode = (value & 0x08);
			ch.Trigger = (GbaDmaTrigger)((value >> 4) & 0x03);
			ch.IrqEnabled = (value & 0x40);
			bool enabled  = (value & 0x80);
			if(ch.Enabled != enabled && enabled) {
				ch.Enabled = true;
				ch.LenLatch = ch.Length;
				ch.SrcLatch = ch.Source;
				ch.DestLatch = ch.Destination;
				if(ch.Trigger == GbaDmaTrigger::Immediate) {
					TriggerDmaChannel(GbaDmaTrigger::Immediate, chIndex);
				}
			} else {
				ch.Enabled = enabled;
			}
			break;
		}

		default:
			MessageManager::Log("Write unknown DMA register: " + HexUtilities::ToHex32(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void GbaDmaController::Serialize(Serializer& s)
{
	for(int i = 0; i < 4; i++) {
		SVI(_state.Ch[i].ReadValue);

		SVI(_state.Ch[i].Destination);
		SVI(_state.Ch[i].Source);
		SVI(_state.Ch[i].Length);

		SVI(_state.Ch[i].DestLatch);
		SVI(_state.Ch[i].SrcLatch);
		SVI(_state.Ch[i].LenLatch);

		SVI(_state.Ch[i].Control);

		SVI(_state.Ch[i].DestMode);
		SVI(_state.Ch[i].SrcMode);

		SVI(_state.Ch[i].Repeat);
		SVI(_state.Ch[i].WordTransfer);
		SVI(_state.Ch[i].DrqMode);

		SVI(_state.Ch[i].Trigger);
		SVI(_state.Ch[i].IrqEnabled);
		SVI(_state.Ch[i].Enabled);
		SVI(_state.Ch[i].Active);

		SVI(_state.Ch[i].Pending);
	}

	SV(_dmaPending);
	SV(_dmaActiveChannel);
	SV(_dmaStartDelay);
}
