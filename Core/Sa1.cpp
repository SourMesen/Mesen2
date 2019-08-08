#include "stdafx.h"
#include "Sa1.h"
#include "Sa1Cpu.h"
#include "EmuSettings.h"
#include "Cpu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "MemoryMappings.h"
#include "BaseCartridge.h"
#include "RamHandler.h"
#include "Sa1VectorHandler.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"

Sa1::Sa1(Console* console)
{
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_memoryType = SnesMemoryType::Register;
	_lastAccessMemType = SnesMemoryType::PrgRom;
	_openBus = 0;
	_cart = _console->GetCartridge().get();
	_snesCpu = _console->GetCpu().get();
	
	_iRam = new uint8_t[Sa1::InternalRamSize];
	_iRamHandler.reset(new RamHandler(_iRam, 0, 0x800, SnesMemoryType::Sa1InternalRam));
	console->GetSettings()->InitializeRam(_iRam, 0x800);
	
	//Register the SA1 in the CPU's memory space ($22xx-$23xx registers)
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();
	_mappings.RegisterHandler(0x00, 0x3F, 0x2000, 0x2FFF, this);
	_mappings.RegisterHandler(0x80, 0xBF, 0x2000, 0x2FFF, this);
	
	cpuMappings->RegisterHandler(0x00, 0x3F, 0x3000, 0x3FFF, _iRamHandler.get());
	cpuMappings->RegisterHandler(0x80, 0xBF, 0x3000, 0x3FFF, _iRamHandler.get());

	_mappings.RegisterHandler(0x00, 0x3F, 0x3000, 0x3FFF, _iRamHandler.get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x3000, 0x3FFF, _iRamHandler.get());
	_mappings.RegisterHandler(0x00, 0x3F, 0x0000, 0x0FFF, _iRamHandler.get());
	_mappings.RegisterHandler(0x80, 0xBF, 0x0000, 0x0FFF, _iRamHandler.get());

	vector<unique_ptr<IMemoryHandler>> &saveRamHandlers = _cart->GetSaveRamHandlers();
	cpuMappings->RegisterHandler(0x40, 0x4F, 0x0000, 0xFFFF, saveRamHandlers);
	_mappings.RegisterHandler(0x40, 0x4F, 0x0000, 0xFFFF, saveRamHandlers);

	_cpu.reset(new Sa1Cpu(this, _console));
	_cpu->PowerOn();
	Reset();
}

void Sa1::Sa1RegisterWrite(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2209: 
			//SCNT (SNES CPU Control)
			_state.CpuMessageReceived = value & 0x0F;
			_state.UseCpuNmiVector = (value & 0x20) != 0;
			_state.UseCpuIrqVector = (value & 0x40) != 0;
			_state.CpuIrqRequested = (value & 0x80) != 0;

			ProcessInterrupts();
			break;

		case 0x220A: 
			//CIE (SA-1 CPU Interrupt Enable)
			_state.Sa1NmiEnabled = (value & 0x10) != 0;
			_state.DmaIrqEnabled = (value & 0x20) != 0;
			_state.TimerIrqEnabled = (value & 0x40) != 0;
			_state.Sa1IrqEnabled = (value & 0x80) != 0;

			ProcessInterrupts();
			break;

		case 0x220B: 
			//CIC (SA-1 CPU Interrupt Clear)
			if(value & 0x80) {
				_state.Sa1IrqRequested = false;
			}
			if(value & 0x20) {
				_state.DmaIrqFlag = false;
			}
			if(value & 0x10) {
				_state.Sa1NmiRequested = false;
			}
			ProcessInterrupts();
			break; 
		
		case 0x220C: _state.CpuNmiVector = (_state.CpuNmiVector & 0xFF00) | value; break; //SNV (SNES CPU NMI Vector - Low)
		case 0x220D: _state.CpuNmiVector = (_state.CpuNmiVector & 0xFF) | (value << 8); break; //SNV (SNES CPU NMI Vector - High)

		case 0x220E: _state.CpuIrqVector = (_state.CpuIrqVector & 0xFF00) | value; break; //SNV (SNES CPU IRQ Vector - Low)
		case 0x220F: _state.CpuIrqVector = (_state.CpuIrqVector & 0xFF) | (value << 8); break; //SIV (SNES CPU IRQ Vector - High)

		case 0x2210: 
			//TMC (H/V Timer Control)
			_state.HorizontalTimerEnabled = (value & 0x01) != 0;
			_state.VerticalTimerEnabled = (value & 0x02) != 0;
			_state.UseLinearTimer = (value & 0x80) != 0;
			if(value) {
				LogDebug("Using timer");
			}
			break; 

		case 0x2211:
			//CTR (CPU Timer restart)
			_state.LinearTimerValue = 0;
			LogDebug("Reset timer");
			break; 
		
		case 0x2212: _state.HTimer = (_state.HTimer & 0x0100) | value; LogDebug("Set timer"); break; //HCNT (Timer H-Count - Low)
		case 0x2213: _state.HTimer = (_state.HTimer & 0xFF) | ((value & 0x01) << 8); LogDebug("Set timer"); break; //HCNT (Timer H-Count - High)

		case 0x2214: _state.VTimer = (_state.VTimer & 0x0100) | value; LogDebug("Set timer"); break; //VCNT (Timer V-Count - Low)
		case 0x2215: _state.VTimer = (_state.VTimer & 0xFF) | ((value & 0x01) << 8); LogDebug("Set timer"); break; //VCNT (Timer V-Count - High)

		case 0x2225: 
			//BMAP (SA-1 BW-RAM Address Mapping)
			_state.Sa1BwBank = value & 0x7F;
			_state.Sa1BwMode = (value & 0x80);
			UpdateSaveRamMappings();
			break;
			
		case 0x2227: _state.Sa1BwWriteEnabled = (value & 0x80) != 0; break; //CBWE (SA-1 CPU BW-RAM Write Enable)
		case 0x222A: _state.Sa1IRamWriteProtect = value; break; //CIWP (SA-1 CPU I-RAM Write Protection)

		case 0x2230:
			//DCNT (DMA Control)
			_state.DmaSrcDevice = (Sa1DmaSrcDevice)(value & 0x03);
			_state.DmaDestDevice = (Sa1DmaDestDevice)((value & 0x04) >> 2);
			_state.DmaCharConvAuto = (value & 0x10) != 0;
			_state.DmaCharConv = (value & 0x20) != 0;
			_state.DmaPriority = (value & 0x40) != 0;
			_state.DmaEnabled = (value & 0x80) != 0;
			break;
		
		case 0x2231: case 0x2232: case 0x2233: case 0x2234: case 0x2235: case 0x2236: case 0x2237:
			WriteSharedRegisters(addr, value);
			break;

		case 0x2238: _state.DmaSize = (_state.DmaSize & 0xFF00) | value; break; //DTC (DMA terminal counter - Low)
		case 0x2239: _state.DmaSize = (_state.DmaSize & 0x00FF) | (value << 8); break; //DTC (DMA terminal counter - High)
		
		case 0x223F: _state.BwRam2BppMode = (value & 0x80) != 0; break; //BBF (Bitmap format)
		
		case 0x2240: case 0x2241: case 0x2242: case 0x2243:
		case 0x2244: case 0x2245: case 0x2246: case 0x2247:
			//BRF (Bitmap register file)
			_state.BitmapRegister1[addr & 0x07] = value;
			if(!value) {
				LogDebug("Bitmap register");
			}
			break;

		case 0x2248: case 0x2249: case 0x224A: case 0x224B:
		case 0x224C: case 0x224D: case 0x224E: case 0x224F:
			//BRF (Bitmap register file)
			_state.BitmapRegister2[addr & 0x07] = value;
			break; 

		case 0x2250: 
			//MCNT (Arithmetic Control)
			_state.MathOp = (Sa1MathOp)(value & 0x03);
			if(value & 0x02) {
				//"Note: Writing Bit1=1 resets the sum to zero."
				_state.MathOpResult = 0;
			}
			break;
		case 0x2251: _state.MultiplicandDividend = (_state.MultiplicandDividend & 0xFF00) | value; break; //MA (Arithmetic parameters - Multiplicand/Dividend - Low)
		case 0x2252: _state.MultiplicandDividend = (_state.MultiplicandDividend & 0x00FF) | (value << 8); break; //MA (Arithmetic parameters - Multiplicand/Dividend - High)
		case 0x2253: _state.MultiplierDivisor = (_state.MultiplierDivisor & 0xFF00) | value; break; //MB (Arithmetic parameters - Multiplier/Divisor - Low)
		case 0x2254: 
			//MB (Arithmetic parameters - Multiplier/Divisor - High)
			_state.MultiplierDivisor = (_state.MultiplierDivisor & 0x00FF) | (value << 8);

			//"Writing to 2254h starts the operation."
			CalculateMathOpResult();
			break; 
		
		case 0x2258: 
			//VBD (Variable length bit processing)
			_state.VariableLengthAutoIncrement = (value & 0x80) != 0;
			_state.VariableLength = value & 0x0F;
			if(value) {
				LogDebug("Variable length");
			}
			break;

		case 0x2259: _state.VariableLengthAddress = (_state.VariableLengthAddress & 0xFFFF00) | value; break; //VDA (Variable length data address - Low)
		case 0x225A: _state.VariableLengthAddress = (_state.VariableLengthAddress & 0xFF00FF) | (value << 8); break; //VDA (Variable length data address - Mid)
		case 0x225B: _state.VariableLengthAddress = (_state.VariableLengthAddress & 0x00FFFF) | (value << 16); break; //VDA (Variable length data address - High)		
	}
}

void Sa1::CpuRegisterWrite(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2200: 
			//CCNT (SA-1 CPU Control)
			if(!(value & 0x20) && _state.Sa1Reset) {
				//Reset the CPU, and sync cycle count
				_cpu->Reset();
				_cpu->IncreaseCycleCount(_memoryManager->GetMasterClock() / 2);
			}

			_state.Sa1MessageReceived = value & 0x0F;
			_state.Sa1NmiRequested = (value & 0x10) != 0;
			_state.Sa1Reset = (value & 0x20) != 0;
			_state.Sa1Wait = (value & 0x40) != 0;
			_state.Sa1IrqRequested = (value & 0x80) != 0;

			ProcessInterrupts();
			break;

		case 0x2201:
			//SIE (SNES CPU Interrupt Enable)
			_state.CpuIrqEnabled = (value & 0x80) != 0;
			_state.CharConvIrqEnabled = (value & 0x20) != 0;

			ProcessInterrupts();
			break;

		case 0x2202: 
			//SIC (SNES CPU Interrupt Clear)
			if(value & 0x80) {
				_state.CpuIrqRequested = false;
			}
			if(value & 0x20) {
				_state.CharConvIrqFlag = false;
			}
			ProcessInterrupts();
			break;

		case 0x2203: _state.Sa1ResetVector = (_state.Sa1ResetVector & 0xFF00) | value; break; //CRV (SA-1 Reset Vector - Low)
		case 0x2204: _state.Sa1ResetVector = (_state.Sa1ResetVector & 0xFF) | (value << 8); break; //CRV (SA-1 Reset Vector - High)

		case 0x2205: _state.Sa1NmiVector = (_state.Sa1NmiVector & 0xFF00) | value; break; //CRV (SA-1 NMI Vector - Low)
		case 0x2206: _state.Sa1NmiVector = (_state.Sa1NmiVector & 0xFF) | (value << 8); break; //CRV (SA-1 NMI Vector - High)

		case 0x2207: _state.Sa1IrqVector = (_state.Sa1IrqVector & 0xFF00) | value; break; //CRV (SA-1 IRQ Vector - Low)
		case 0x2208: _state.Sa1IrqVector = (_state.Sa1IrqVector & 0xFF) | (value << 8); break; //CRV (SA-1 IRQ Vector - High)

		case 0x2220: UpdateBank(0, value); break; //CXB (MMC Bank C)
		case 0x2221: UpdateBank(1, value); break; //DXB (MMC Bank D)
		case 0x2222: UpdateBank(2, value); break; //EXB (MMC Bank E)
		case 0x2223: UpdateBank(3, value); break; //FXB (MMC Bank F)

		case 0x2224: 
			//BMAPS (SNES CPU BW-RAM Address Mapping)
			if(_state.CpuBwBank != (value & 0x1F)) {
				_state.CpuBwBank = value & 0x1F;
				UpdateSaveRamMappings();
			}
			break;

		case 0x2226: _state.CpuBwWriteEnabled = (value & 0x80) != 0; break; //SBWE (SNES CPU BW-RAM Write Enable)
			
		case 0x2228: _state.BwWriteProtectedArea = (value & 0x0F); break; //BWPA (SNES CPU BW-RAM Write Protected Area)

		case 0x2229: _state.CpuIRamWriteProtect = value; break; //SIWP (SNES CPU I-RAM Write Protection)

		case 0x2231: case 0x2232: case 0x2233: case 0x2234: case 0x2235: case 0x2236: case 0x2237:
			WriteSharedRegisters(addr, value);
			break;
	}
}

void Sa1::WriteSharedRegisters(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2231:
			//CDMA (Character conversion DMA parameters) (Shared with SNES CPU)
			LogDebug("CDMA");
			break;

		case 0x2232: _state.DmaSrcAddr = (_state.DmaSrcAddr & 0xFFFF00) | value; break; //SDA (DMA source start address - Low) (Shared with SNES CPU)
		case 0x2233: _state.DmaSrcAddr = (_state.DmaSrcAddr & 0xFF00FF) | (value << 8); break; //SDA (DMA source start address - Mid) (Shared with SNES CPU)
		case 0x2234: _state.DmaSrcAddr = (_state.DmaSrcAddr & 0x00FFFF) | (value << 16); break; //SDA (DMA source start address - High) (Shared with SNES CPU)

		case 0x2235: _state.DmaDestAddr = (_state.DmaDestAddr & 0xFFFF00) | value; break; //DDA (DMA dest start address - Low) (Shared with SNES CPU)
		case 0x2236: 
			//DDA (DMA dest start address - Mid) (Shared with SNES CPU)
			_state.DmaDestAddr = (_state.DmaDestAddr & 0xFF00FF) | (value << 8);
			if(_state.DmaEnabled && !_state.DmaCharConv && _state.DmaDestDevice == Sa1DmaDestDevice::InternalRam) {
				_state.DmaRunning = true;
			}
			break; 

		case 0x2237:
			//DDA (DMA dest start address - High) (Shared with SNES CPU)
			_state.DmaDestAddr = (_state.DmaDestAddr & 0x00FFFF) | (value << 16);
			if(_state.DmaEnabled && !_state.DmaCharConv && _state.DmaDestDevice == Sa1DmaDestDevice::BwRam) {
				_state.DmaRunning = true;
			}
			break;
	}
}

uint8_t Sa1::Sa1RegisterRead(uint16_t addr)
{
	switch(addr) {
		case 0x2301: 
			//CFR (SA-1 Status Flags)
			return (
				_state.Sa1MessageReceived |
				(_state.Sa1NmiRequested << 4) |
				(_state.DmaIrqFlag << 5) |
				//TODO: Timer irq flag
				(_state.Sa1IrqRequested << 7)
			);
			
		case 0x2302: break; //HCR (SA-1 H Counter read - Low)
		case 0x2303: break; //HCR (SA-1 H Counter read - High)
			
		case 0x2304: break; //VCR (SA-1 V Counter read - Low)
		case 0x2305: break; //VCR (SA-1 V Counter read - High)
			
		case 0x2306: return _state.MathOpResult & 0xFF; //MR (Arithmetic result)
		case 0x2307: return (_state.MathOpResult >> 8) & 0xFF; //MR (Arithmetic result)
		case 0x2308: return (_state.MathOpResult >> 16) & 0xFF; break; //MR (Arithmetic result)
		case 0x2309: return (_state.MathOpResult >> 24) & 0xFF; break; //MR (Arithmetic result)
		case 0x230A: return (_state.MathOpResult >> 32) & 0xFF; break; //MR (Arithmetic result)

		case 0x230B: return _state.MathOverflow; break; //OF (Arithmetic overflow flag)

		case 0x230C: break; //VDP (Variable length data port - Low)
		case 0x230D: break; //VDP (Variable length data port - High)
	}

	LogDebug("[Debug] Read SA1 - missing register: $" + HexUtilities::ToHex(addr));

	return _openBus;
}

uint8_t Sa1::CpuRegisterRead(uint16_t addr)
{
	switch(addr) {
		case 0x2300: 
			//SFR (SNES CPU Status Flags)
			return (
				_state.CpuMessageReceived |
				(_state.UseCpuNmiVector << 4) |
				(_state.CharConvIrqFlag << 5) |
				(_state.UseCpuIrqVector << 6) |
				(_state.CpuIrqRequested << 7)
			);

		case 0x230E: break; //VC (Version code register)
	}

	LogDebug("[Debug] CPU - missing register: $" + HexUtilities::ToHex(addr));

	return _memoryManager->GetOpenBus();
}

void Sa1::ProcessInterrupts()
{
	if((_state.Sa1IrqRequested && _state.Sa1IrqEnabled) || (_state.DmaIrqFlag && _state.DmaIrqEnabled) || (_state.CharConvIrqFlag && _state.CharConvIrqEnabled)) {
		_cpu->SetIrqSource(IrqSource::Coprocessor);
	} else {
		_cpu->ClearIrqSource(IrqSource::Coprocessor);
	}

	if(_state.Sa1NmiRequested && _state.Sa1NmiEnabled) {
		_cpu->SetNmiFlag();
	} else {
		//...?
	}

	if(_state.CpuIrqRequested && _state.CpuIrqEnabled) {
		_snesCpu->SetIrqSource(IrqSource::Coprocessor);
	} else {
		_snesCpu->ClearIrqSource(IrqSource::Coprocessor);
	}
}

void Sa1::WriteSa1(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	IMemoryHandler *handler = _mappings.GetHandler(addr);
	if(handler) {
		_console->ProcessMemoryWrite<CpuType::Sa1>(addr, value, type);
		_lastAccessMemType = handler->GetMemoryType();
		_openBus = value;
		handler->Write(addr, value);
	} else {
		LogDebug("[Debug] Write SA1 - missing handler: $" + HexUtilities::ToHex(addr));
	}
}

uint8_t Sa1::ReadSa1(uint32_t addr, MemoryOperationType type)
{
	IMemoryHandler *handler = _mappings.GetHandler(addr);
	if(handler) {
		uint8_t value = handler->Read(addr);
		_lastAccessMemType = handler->GetMemoryType();
		_openBus = value;
		_console->ProcessMemoryRead<CpuType::Sa1>(addr, value, type);
		return value;
	} else {
		LogDebug("[Debug] Read SA1 - missing handler: $" + HexUtilities::ToHex(addr));
	}
	return _openBus;
}

uint8_t Sa1::Read(uint32_t addr)
{
	return Sa1RegisterRead(addr);
}

uint8_t Sa1::Peek(uint32_t addr)
{
	return Sa1RegisterRead(addr);
}

void Sa1::PeekBlock(uint8_t *output)
{
	for(int i = 0; i < 0x1000; i++) {
		output[i] = Sa1RegisterRead(i);
	}
}

void Sa1::Write(uint32_t addr, uint8_t value)
{
	Sa1RegisterWrite(addr, value);
}

AddressInfo Sa1::GetAbsoluteAddress(uint32_t address)
{
	return { -1, SnesMemoryType::Register };
}

void Sa1::Run()
{
	uint64_t targetCycle = _memoryManager->GetMasterClock() / 2;

	while(_cpu->GetCycleCount() < targetCycle) {
		if(_state.Sa1Wait || _state.Sa1Reset) {
			_cpu->IncreaseCycleCount<1>();
		} else if(_state.DmaRunning) {
			RunDma();
		} else {
			_cpu->Exec();
		}
	}
}

void Sa1::WriteInternalRam(uint32_t addr, uint8_t value)
{
	_iRam[addr & (Sa1::InternalRamSize - 1)] = value;
}

void Sa1::WriteBwRam(uint32_t addr, uint8_t value)
{
	_cart->DebugGetSaveRam()[addr & (_cart->DebugGetSaveRamSize() - 1)] = value;
}

void Sa1::RunDma()
{
	if(_state.DmaSize > 0) {
		_state.DmaSize--;
		
		if(_state.DmaSrcDevice == Sa1DmaSrcDevice::PrgRom && _state.DmaDestDevice == Sa1DmaDestDevice::InternalRam) {
			_cpu->IncreaseCycleCount<1>();

			if(GetSnesCpuMemoryType() == SnesMemoryType::PrgRom || GetSnesCpuMemoryType() == SnesMemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == SnesMemoryType::Sa1InternalRam) {
					_cpu->IncreaseCycleCount<1>();
				}
			}

			WriteInternalRam(_state.DmaDestAddr, ReadSa1(_state.DmaSrcAddr));
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::PrgRom && _state.DmaDestDevice == Sa1DmaDestDevice::BwRam) {
			_cpu->IncreaseCycleCount<2>();
			
			if(GetSnesCpuMemoryType() == SnesMemoryType::SaveRam) {
				_cpu->IncreaseCycleCount<2>();
			}

			WriteBwRam(_state.DmaDestAddr, ReadSa1(_state.DmaSrcAddr));
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::BwRam && _state.DmaDestDevice == Sa1DmaDestDevice::InternalRam) {
			_cpu->IncreaseCycleCount<2>();
				
			if(GetSnesCpuMemoryType() == SnesMemoryType::SaveRam || GetSnesCpuMemoryType() == SnesMemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == SnesMemoryType::SaveRam) {
					_cpu->IncreaseCycleCount<1>();
				}
			}

			WriteInternalRam(_state.DmaDestAddr, _cart->DebugGetSaveRam()[_state.DmaSrcAddr & (_cart->DebugGetSaveRamSize() - 1)]);
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::InternalRam && _state.DmaDestDevice == Sa1DmaDestDevice::BwRam) {
			_cpu->IncreaseCycleCount<2>();

			if(GetSnesCpuMemoryType() == SnesMemoryType::SaveRam || GetSnesCpuMemoryType() == SnesMemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == SnesMemoryType::SaveRam) {
					_cpu->IncreaseCycleCount<1>();
				}
			}

			WriteBwRam(_state.DmaDestAddr, _iRam[_state.DmaSrcAddr & (Sa1::InternalRamSize - 1)]);
		}

		_state.DmaSrcAddr++;
		_state.DmaDestAddr++;
	}

	if(_state.DmaSize == 0) {
		_state.DmaRunning = false;
		_state.DmaIrqFlag = true;
		ProcessInterrupts();
	}
}

void Sa1::UpdateBank(uint8_t index, uint8_t value)
{
	if(_state.Banks[index] != value) {
		_state.Banks[index] = value;
		UpdatePrgRomMappings();
	}
}

void Sa1::UpdatePrgRomMappings()
{
	vector<unique_ptr<IMemoryHandler>> &prgRomHandlers = _cart->GetPrgRomHandlers();
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();

	for(int i = 0; i < 2; i++) {
		MemoryMappings* mappings = i == 0 ? &_mappings : cpuMappings;
		mappings->RegisterHandler(0x00, 0x1F, 0x8000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[0] & 0x80) ? (_state.Banks[0] & 0x07) * 256 : 0);
		mappings->RegisterHandler(0x20, 0x3F, 0x8000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[1] & 0x80) ? (_state.Banks[1] & 0x07) * 256 : 256);
		mappings->RegisterHandler(0x80, 0x9F, 0x8000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[2] & 0x80) ? (_state.Banks[2] & 0x07) * 256 : 512);
		mappings->RegisterHandler(0xA0, 0xBF, 0x8000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[3] & 0x80) ? (_state.Banks[3] & 0x07) * 256 : 768);

		mappings->RegisterHandler(0xC0, 0xCF, 0x0000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[0] & 0x07) * 256);
		mappings->RegisterHandler(0xD0, 0xDF, 0x0000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[1] & 0x07) * 256);
		mappings->RegisterHandler(0xE0, 0xEF, 0x0000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[2] & 0x07) * 256);
		mappings->RegisterHandler(0xF0, 0xFF, 0x0000, 0xFFFF, prgRomHandlers, 0, (_state.Banks[3] & 0x07) * 256);
	}

	UpdateVectorMappings();
}

void Sa1::UpdateVectorMappings()
{
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();
	_sa1VectorHandler.reset(new Sa1VectorHandler(cpuMappings->GetHandler(0xF000), &_state, false));
	_cpuVectorHandler.reset(new Sa1VectorHandler(cpuMappings->GetHandler(0xF000), &_state, true));
	_mappings.RegisterHandler(0x00, 0x00, 0xF000, 0xFFFF, _sa1VectorHandler.get());
	cpuMappings->RegisterHandler(0x00, 0x00, 0xF000, 0xFFFF, _cpuVectorHandler.get());
}

void Sa1::UpdateSaveRamMappings()
{
	vector<unique_ptr<IMemoryHandler>> &saveRamHandlers = _cart->GetSaveRamHandlers();
	MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();
	for(int i = 0; i < 0x3F; i++) {
		_mappings.RegisterHandler(i, i, 0x6000, 0x7FFF, saveRamHandlers, 0, _state.Sa1BwBank * 2);
		_mappings.RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x7FFF, saveRamHandlers, 0, _state.Sa1BwBank * 2);

		cpuMappings->RegisterHandler(i, i, 0x6000, 0x7FFF, saveRamHandlers, 0, _state.CpuBwBank * 2);
		cpuMappings->RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x7FFF, saveRamHandlers, 0, _state.CpuBwBank * 2);
	}
}

void Sa1::CalculateMathOpResult()
{
	if((int)_state.MathOp & (int)Sa1MathOp::Sum) {
		uint64_t result = _state.MathOpResult + ((int16_t)_state.MultiplicandDividend * (int16_t)_state.MultiplierDivisor);
		_state.MathOverflow = (result >> 33) & 0x80;
		
		//Keep 40 bits
		_state.MathOpResult = result & 0xFFFFFFFFFF;
		_state.MultiplierDivisor = 0;
	} else {
		if(_state.MathOp == Sa1MathOp::Mul) {
			_state.MathOpResult = (uint32_t)((int16_t)_state.MultiplicandDividend * (int16_t)_state.MultiplierDivisor);

			//"The value in this register gets destroyed after both multiplication and division."
			_state.MultiplierDivisor = 0;
		} else {
			if(_state.MultiplierDivisor == 0) {
				//Division by 0 returns 0, remainder 0
				_state.MathOpResult = 0;
			} else {
				int16_t dividend = _state.MultiplicandDividend;
				uint16_t remainder = dividend % _state.MultiplierDivisor;
				if(dividend < 0) {
					remainder += _state.MultiplierDivisor;
				}
				uint16_t result = (dividend - remainder) / _state.MultiplierDivisor;
				_state.MathOpResult = (remainder << 16) | result;
			}

			//"The value in this register gets destroyed after both multiplication and division."
			_state.MultiplierDivisor = 0;

			//"The value in this register is kept intact after multiplication, but gets destroyed after division."
			_state.MultiplicandDividend = 0;
		}
	}
}

void Sa1::Reset()
{
	_state = {};
	CpuRegisterWrite(0x2200, 0x20);
	CpuRegisterWrite(0x2228, 0xFF);

	CpuRegisterWrite(0x2220, 0);
	CpuRegisterWrite(0x2221, 1);
	CpuRegisterWrite(0x2222, 2);
	CpuRegisterWrite(0x2223, 3);

	UpdatePrgRomMappings();
	UpdateSaveRamMappings();

	_cpu->Reset();
}

SnesMemoryType Sa1::GetSa1MemoryType()
{
	return _lastAccessMemType;
}

SnesMemoryType Sa1::GetSnesCpuMemoryType()
{
	return _memoryManager->GetMemoryTypeBusA();
}

uint8_t* Sa1::DebugGetInternalRam()
{
	return _iRam;
}

uint32_t Sa1::DebugGetInternalRamSize()
{
	return Sa1::InternalRamSize;
}

CpuState Sa1::GetCpuState()
{
	return _cpu->GetState();
}

MemoryMappings* Sa1::GetMemoryMappings()
{
	return &_mappings;
}

void Sa1::Serialize(Serializer &s)
{
	s.Stream(_cpu.get());

	s.Stream(
		_state.Sa1ResetVector, _state.Sa1IrqVector, _state.Sa1NmiVector, _state.Sa1IrqRequested, _state.Sa1IrqEnabled, _state.Sa1NmiRequested, _state.Sa1NmiEnabled,
		_state.Sa1Wait, _state.Sa1Reset, _state.DmaIrqEnabled, _state.TimerIrqEnabled, _state.Sa1MessageReceived, _state.CpuMessageReceived, _state.CpuIrqVector,
		_state.CpuNmiVector, _state.UseCpuIrqVector, _state.UseCpuNmiVector, _state.CpuIrqRequested, _state.CpuIrqEnabled, _state.CharConvIrqFlag, _state.CharConvIrqEnabled,
		_state.CpuBwBank, _state.CpuBwWriteEnabled, _state.Sa1BwBank, _state.Sa1BwMode, _state.Sa1BwWriteEnabled, _state.BwWriteProtectedArea, _state.BwRam2BppMode,
		_state.CpuIRamWriteProtect, _state.Sa1IRamWriteProtect, _state.DmaSrcAddr, _state.DmaDestAddr, _state.DmaSize, _state.DmaEnabled, _state.DmaPriority,
		_state.DmaCharConv, _state.DmaCharConvAuto, _state.DmaDestDevice, _state.DmaSrcDevice, _state.DmaRunning, _state.DmaIrqFlag, _state.HorizontalTimerEnabled,
		_state.VerticalTimerEnabled, _state.UseLinearTimer, _state.HTimer, _state.VTimer, _state.LinearTimerValue, _state.MathOp, _state.MultiplicandDividend,
		_state.MultiplierDivisor, _state.MathOpResult, _state.MathOverflow, _state.VariableLengthAutoIncrement, _state.VariableLength, _state.VariableLengthAddress,
		_state.Banks[0], _state.Banks[1], _state.Banks[2], _state.Banks[3],
		_state.BitmapRegister1[0], _state.BitmapRegister1[1], _state.BitmapRegister1[2], _state.BitmapRegister1[3],
		_state.BitmapRegister1[4], _state.BitmapRegister1[5], _state.BitmapRegister1[6], _state.BitmapRegister1[7],
		_state.BitmapRegister2[0], _state.BitmapRegister2[1], _state.BitmapRegister2[2], _state.BitmapRegister2[3],
		_state.BitmapRegister2[4], _state.BitmapRegister2[5], _state.BitmapRegister2[6], _state.BitmapRegister2[7]
	);

	s.Stream(_lastAccessMemType, _openBus);
	s.StreamArray(_iRam, Sa1::InternalRamSize);

	if(!s.IsSaving()) {
		UpdatePrgRomMappings();
		UpdateSaveRamMappings();
		ProcessInterrupts();
	}
}