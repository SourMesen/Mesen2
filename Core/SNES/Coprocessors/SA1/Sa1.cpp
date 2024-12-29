#include "pch.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/SA1/Sa1Cpu.h"
#include "SNES/Coprocessors/SA1/Sa1VectorHandler.h"
#include "SNES/Coprocessors/SA1/Sa1IRamHandler.h"
#include "SNES/Coprocessors/SA1/Sa1BwRamHandler.h"
#include "SNES/Coprocessors/SA1/CpuBwRamHandler.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryMappings.h"
#include "SNES/RamHandler.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/BatteryManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryOperationType.h"

Sa1::Sa1(SnesConsole* console)
{
	_console = console;
	_emu = console->GetEmulator();
	_memoryManager = console->GetMemoryManager();
	_lastAccessMemType = MemoryType::SnesPrgRom;
	_openBus = 0;
	_cart = _console->GetCartridge();
	_snesCpu = _console->GetCpu();
	
	_iRam = new uint8_t[Sa1::InternalRamSize];
	_emu->RegisterMemory(MemoryType::Sa1InternalRam, _iRam, Sa1::InternalRamSize);
	_iRamHandler.reset(new Sa1IRamHandler(_iRam));
	_console->InitializeRam(_iRam, 0x800);
	
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

	if(_cart->DebugGetSaveRamSize() > 0) {
		_bwRamHandler.reset(new Sa1BwRamHandler(_cart->DebugGetSaveRam(), _cart->DebugGetSaveRamSize(), &_state));
		for(int i = 0; i <= 0x3F; i++) {
			//SA-1: 00-3F:6000-7FFF + 80-BF:6000-7FFF
			_mappings.RegisterHandler(i, i, 0x6000, 0x7FFF, _bwRamHandler.get());
			_mappings.RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x7FFF, _bwRamHandler.get());
		}
		for(int i = 0; i <= 0x0F; i++) {
			//SA-1: 60-6F:0000-FFFF
			_mappings.RegisterHandler(i + 0x60, i + 0x60, 0x0000, 0xFFFF, _bwRamHandler.get());
		}
	}

	vector<unique_ptr<IMemoryHandler>> &saveRamHandlers = _cart->GetSaveRamHandlers();
	for(unique_ptr<IMemoryHandler> &handler : saveRamHandlers) {
		_cpuBwRamHandlers.push_back(unique_ptr<IMemoryHandler>(new CpuBwRamHandler(handler.get(), &_state, this)));
	}
	cpuMappings->RegisterHandler(0x40, 0x4F, 0x0000, 0xFFFF, _cpuBwRamHandlers);
	_mappings.RegisterHandler(0x40, 0x4F, 0x0000, 0xFFFF, saveRamHandlers);

	_cpu.reset(new Sa1Cpu(this, _emu));
	_cpu->PowerOn();
	Reset();
}

Sa1::~Sa1()
{
	delete[] _iRam;
}

void Sa1::Sa1RegisterWrite(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x2209: 
			//SCNT (SNES CPU Control)
			_state.CpuMessageReceived = value & 0x0F;
			_state.UseCpuNmiVector = (value & 0x10) != 0;
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
			if(_state.Sa1BwBank != (value & 0x7F) || _state.Sa1BwMode != (value & 0x80)) {
				_state.Sa1BwBank = value & 0x7F;
				_state.Sa1BwMode = (value & 0x80);
				UpdateSaveRamMappings();
			}
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
			if(!_state.DmaEnabled) {
				_state.CharConvCounter = 0;
			}
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
			if(addr == 0x2247 && _state.DmaEnabled && _state.DmaCharConv && !_state.DmaCharConvAuto) {
				RunCharConvertType2();
			}
			break;

		case 0x2248: case 0x2249: case 0x224A: case 0x224B:
		case 0x224C: case 0x224D: case 0x224E: case 0x224F:
			//BRF (Bitmap register file)
			_state.BitmapRegister2[addr & 0x07] = value;
			if(addr == 0x224F && _state.DmaEnabled && _state.DmaCharConv && !_state.DmaCharConvAuto) {
				RunCharConvertType2();
			}
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
			_state.VarLenAutoInc = (value & 0x80) != 0;
			_state.VarLenBitCount = value == 0 ? 16 : (value & 0x0F);

			if(!_state.VarLenAutoInc) {
				IncVarLenPosition();
			}
			break;

		case 0x2259: _state.VarLenAddress = (_state.VarLenAddress & 0xFFFF00) | value; break; //VDA (Variable length data address - Low)
		case 0x225A: _state.VarLenAddress = (_state.VarLenAddress & 0xFF00FF) | (value << 8); break; //VDA (Variable length data address - Mid)
		case 0x225B: 
			//VDA (Variable length data address - High)
			_state.VarLenAddress = (_state.VarLenAddress & 0x00FFFF) | (value << 16); 
			_state.VarLenCurrentBit = 0;
			break;
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
			_state.CharConvFormat = std::min(value & 0x03, 2);
			switch(_state.CharConvFormat) {
				case 0: _state.CharConvBpp = 8; break;
				case 1: _state.CharConvBpp = 4; break;
				case 2: _state.CharConvBpp = 2; break;
			}
			_state.CharConvWidth = std::min((value & 0x1C) >> 2, 5);

			if(value & 0x80) {
				//End of character conversion type 1
				_state.CharConvDmaActive = false;
			}
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
			} else if(_state.DmaCharConv && _state.DmaCharConvAuto) {
				_state.CharConvDmaActive = true;
				_state.CharConvIrqFlag = true;
				ProcessInterrupts();
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

		case 0x230C: {
			//VDP (Variable length data port - Low)
			uint32_t data = ReadSa1(_state.VarLenAddress) | (ReadSa1(_state.VarLenAddress + 1) << 8) | (ReadSa1(_state.VarLenAddress + 2) << 16);
			return data >> _state.VarLenCurrentBit;
		}

		case 0x230D: {
			//VDP (Variable length data port - High)
			uint32_t data = ReadSa1(_state.VarLenAddress) | (ReadSa1(_state.VarLenAddress + 1) << 8) | (ReadSa1(_state.VarLenAddress + 2) << 16);
			uint8_t value = data >> (_state.VarLenCurrentBit + 8);
			if(_state.VarLenAutoInc) {
				IncVarLenPosition();
			}
			return value;
		}
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
	if((_state.Sa1IrqRequested && _state.Sa1IrqEnabled) || (_state.DmaIrqFlag && _state.DmaIrqEnabled)) {
		_cpu->SetIrqSource(SnesIrqSource::Coprocessor);
	} else {
		_cpu->ClearIrqSource(SnesIrqSource::Coprocessor);
	}

	if(_state.Sa1NmiRequested && _state.Sa1NmiEnabled) {
		_cpu->SetNmiFlag(1);
	}

	if((_state.CpuIrqRequested && _state.CpuIrqEnabled) || (_state.CharConvIrqFlag && _state.CharConvIrqEnabled)) {
		_snesCpu->SetIrqSource(SnesIrqSource::Coprocessor);
	} else {
		_snesCpu->ClearIrqSource(SnesIrqSource::Coprocessor);
	}
}

void Sa1::WriteSa1(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_emu->ProcessMemoryWrite<CpuType::Sa1>(addr, value, type)) {
		IMemoryHandler *handler = _mappings.GetHandler(addr);
		if(handler) {
			_lastAccessMemType = handler->GetMemoryType();
			_openBus = value;
			handler->Write(addr, value);
		} else {
			LogDebug("[Debug] Write SA1 - missing handler: $" + HexUtilities::ToHex(addr));
		}
	}
}

uint8_t Sa1::ReadSa1(uint32_t addr, MemoryOperationType type)
{
	IMemoryHandler *handler = _mappings.GetHandler(addr);
	uint8_t value;
	if(handler) {
		value = handler->Read(addr);
		_lastAccessMemType = handler->GetMemoryType();
		_openBus = value;
	} else {
		value = _openBus;
		LogDebug("[Debug] Read SA1 - missing handler: $" + HexUtilities::ToHex(addr));
	}
	_emu->ProcessMemoryRead<CpuType::Sa1>(addr, value, type);
	return value;
}

uint8_t Sa1::Read(uint32_t addr)
{
	return Sa1RegisterRead(addr);
}

uint8_t Sa1::Peek(uint32_t addr)
{
	//Not implemented
	return 0;
}

void Sa1::PeekBlock(uint32_t addr, uint8_t *output)
{
	memset(output, 0, 0x1000);
}

void Sa1::Write(uint32_t addr, uint8_t value)
{
	Sa1RegisterWrite(addr, value);
}

AddressInfo Sa1::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::None };
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

			if(GetSnesCpuMemoryType() == MemoryType::SnesPrgRom || GetSnesCpuMemoryType() == MemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == MemoryType::Sa1InternalRam) {
					_cpu->IncreaseCycleCount<1>();
				}
			}

			WriteInternalRam(_state.DmaDestAddr, ReadSa1(_state.DmaSrcAddr));
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::PrgRom && _state.DmaDestDevice == Sa1DmaDestDevice::BwRam) {
			_cpu->IncreaseCycleCount<2>();
			
			if(GetSnesCpuMemoryType() == MemoryType::SnesSaveRam) {
				_cpu->IncreaseCycleCount<2>();
			}

			WriteBwRam(_state.DmaDestAddr, ReadSa1(_state.DmaSrcAddr));
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::BwRam && _state.DmaDestDevice == Sa1DmaDestDevice::InternalRam) {
			_cpu->IncreaseCycleCount<2>();
				
			if(GetSnesCpuMemoryType() == MemoryType::SnesSaveRam || GetSnesCpuMemoryType() == MemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == MemoryType::SnesSaveRam) {
					_cpu->IncreaseCycleCount<1>();
				}
			}

			WriteInternalRam(_state.DmaDestAddr, _cart->DebugGetSaveRam()[_state.DmaSrcAddr & (_cart->DebugGetSaveRamSize() - 1)]);
		} else if(_state.DmaSrcDevice == Sa1DmaSrcDevice::InternalRam && _state.DmaDestDevice == Sa1DmaDestDevice::BwRam) {
			_cpu->IncreaseCycleCount<2>();

			if(GetSnesCpuMemoryType() == MemoryType::SnesSaveRam || GetSnesCpuMemoryType() == MemoryType::Sa1InternalRam) {
				_cpu->IncreaseCycleCount<1>();
				if(GetSnesCpuMemoryType() == MemoryType::SnesSaveRam) {
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
	_cpuVectorHandler.reset(new Sa1VectorHandler(cpuMappings->GetHandler(0xF000), &_state));
	cpuMappings->RegisterHandler(0x00, 0x00, 0xF000, 0xFFFF, _cpuVectorHandler.get());
}

void Sa1::UpdateSaveRamMappings()
{
	vector<unique_ptr<IMemoryHandler>> &saveRamHandlers = _cart->GetSaveRamHandlers();
	if(saveRamHandlers.size() > 0) {
		MemoryMappings* cpuMappings = _memoryManager->GetMemoryMappings();
		uint32_t bank1 = (_state.CpuBwBank * 2) % saveRamHandlers.size();
		uint32_t bank2 = (_state.CpuBwBank * 2 + 1) % saveRamHandlers.size();
		for(int i = 0; i <= 0x3F; i++) {
			//S-CPU: 00-3F:6000-7FFF + 80-BF:6000-7FFF
			cpuMappings->RegisterHandler(i, i, 0x6000, 0x6FFF, saveRamHandlers[bank1].get());
			cpuMappings->RegisterHandler(i, i, 0x7000, 0x7FFF, saveRamHandlers[bank2].get());
			cpuMappings->RegisterHandler(i + 0x80, i + 0x80, 0x6000, 0x6FFF, saveRamHandlers[bank1].get());
			cpuMappings->RegisterHandler(i + 0x80, i + 0x80, 0x7000, 0x7FFF, saveRamHandlers[bank2].get());
		}
	}
}

void Sa1::IncVarLenPosition()
{
	_state.VarLenCurrentBit += _state.VarLenBitCount;
	_state.VarLenAddress += _state.VarLenCurrentBit >> 3;
	_state.VarLenCurrentBit &= 0x07;
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

uint8_t Sa1::ReadCharConvertType1(uint32_t addr)
{
	uint8_t mask = (_state.CharConvBpp * 8) - 1;
	
	if((addr & mask) == 0) {
		//Trying to read the first byte of a new tile, convert it and write its contents to IRAM
		uint8_t* bwRam = _cart->DebugGetSaveRam();
		uint32_t bwRamMask = _cart->DebugGetSaveRamSize() - 1;

		uint8_t tilesPerLine = (1 << _state.CharConvWidth);
		uint32_t bytesPerLine = (tilesPerLine * 8) >> _state.CharConvFormat;
		uint32_t tileNumber = ((addr - _state.DmaSrcAddr) & bwRamMask) >> (6 - _state.CharConvFormat);
		uint8_t tileX = tileNumber & (tilesPerLine - 1);
		uint32_t tileY = (tileNumber >> _state.CharConvWidth);
		uint32_t srcAddr = _state.DmaSrcAddr + tileY * 8 * bytesPerLine + tileX * _state.CharConvBpp;

		for(int y = 0; y < 8; y++) {
			//For each row, get the original pixels (in a packed format, 4 pixels per byte in 2bpp, 2 pixels per byte in 4bpp, etc.)
			uint64_t data = 0;
			for(int i = 0; i < _state.CharConvBpp; i++) {
				data |= (uint64_t)bwRam[(srcAddr + i) & bwRamMask] << (i * 8);
			}
			srcAddr += bytesPerLine;

			uint8_t result[8] = {};
			for(int x = 0; x < 8; x++) {
				//For each column (pixels in the tile), convert to VRAM format
				for(int i = 0; i < _state.CharConvBpp; i++) {
					result[i] |= (data & 0x01) << (7 - x);
					data >>= 1;
				}
			}

			//Copy all converted bytes to IRAM (in PPU VRAM format)
			for(int i = 0; i < _state.CharConvBpp; i++) {
				uint8_t offset = (y << 1) + ((i >> 1) << 4) + (i & 0x01);
				_iRam[(_state.DmaDestAddr + offset) & 0x7FF] = result[i];
			}
		}
	}

	return _iRam[(_state.DmaDestAddr + (addr & mask)) & 0x7FF];
}

void Sa1::RunCharConvertType2()
{
	uint8_t* bmpRegs = (_state.CharConvCounter & 0x01) ? _state.BitmapRegister2 : _state.BitmapRegister1;

	uint16_t dest = _state.DmaDestAddr & 0x7FF;
	dest &= ~((_state.CharConvBpp << 4) - 1); //ignore lower 5-7 bits of the address based on BPP
	dest += (_state.CharConvCounter & 0x07) * 2; //first 2 bit planes are together, each tile starts 2 bytes later
	dest += (_state.CharConvCounter & 0x08) * _state.CharConvBpp; //number of bytes per tile row

	//Convert 1 pixel per byte format (no matter BPP) to VRAM format
	for(int i = 0; i < _state.CharConvBpp; i++) {
		//For each bitplane, grab the matching bit for all 8 pixels and convert to VRAM format
		uint8_t value = 0;
		for(int j = 0; j < 8; j++) {
			value |= ((bmpRegs[j] >> i) & 0x01) << (7 - j);
		}

		//Write the converted VRAM-format byte to IRAM
		uint8_t offset = ((i >> 1) << 4) + (i & 0x01);
		_iRam[dest + offset] = value;
	}

	_state.CharConvCounter = (_state.CharConvCounter + 1) & 0x0F;
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

MemoryType Sa1::GetSa1MemoryType()
{
	return _lastAccessMemType;
}

bool Sa1::IsSnesCpuFastRomSpeed()
{
	//TODO: Does DMA always count as SlowROM speed regardless of the CPU's speed when DMA began?
	return _memoryManager->GetCpuSpeed() == 6;
}

MemoryType Sa1::GetSnesCpuMemoryType()
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

Sa1State& Sa1::GetState()
{
	return _state;
}

SnesCpuState& Sa1::GetCpuState()
{
	return _cpu->GetState();
}

uint16_t Sa1::ReadVector(uint16_t vector)
{
	switch(vector) {
		case Sa1Cpu::NmiVector: return _state.Sa1NmiVector;
		case Sa1Cpu::ResetVector: return _state.Sa1ResetVector;
		case Sa1Cpu::IrqVector: return _state.Sa1IrqVector;
	}

	//BRK/COP vectors are taken from ROM
	uint8_t low = ReadSa1(vector);
	uint8_t high = ReadSa1(vector + 1);
	return (high << 8) | low;
}

MemoryMappings* Sa1::GetMemoryMappings()
{
	return &_mappings;
}

void Sa1::LoadBattery()
{
	if(_cpuBwRamHandlers.empty()) {
		//When there is no actual save RAM and the battery flag is set, IRAM is backed up instead
		//Used by Pachi-Slot Monogatari - PAL Kougyou Special
		_emu->GetBatteryManager()->LoadBattery(".srm", _iRam, Sa1::InternalRamSize);
	}
}

void Sa1::SaveBattery()
{
	if(_cpuBwRamHandlers.empty()) {
		_emu->GetBatteryManager()->SaveBattery(".srm", _iRam, Sa1::InternalRamSize);
	}
}


void Sa1::Serialize(Serializer &s)
{
	SV(_cpu);

	SV(_state.Sa1ResetVector); SV(_state.Sa1IrqVector); SV(_state.Sa1NmiVector); SV(_state.Sa1IrqRequested); SV(_state.Sa1IrqEnabled); SV(_state.Sa1NmiRequested); SV(_state.Sa1NmiEnabled);
	SV(_state.Sa1Wait); SV(_state.Sa1Reset); SV(_state.DmaIrqEnabled); SV(_state.TimerIrqEnabled); SV(_state.Sa1MessageReceived); SV(_state.CpuMessageReceived); SV(_state.CpuIrqVector);
	SV(_state.CpuNmiVector); SV(_state.UseCpuIrqVector); SV(_state.UseCpuNmiVector); SV(_state.CpuIrqRequested); SV(_state.CpuIrqEnabled); SV(_state.CharConvIrqFlag); SV(_state.CharConvIrqEnabled);
	SV(_state.CpuBwBank); SV(_state.CpuBwWriteEnabled); SV(_state.Sa1BwBank); SV(_state.Sa1BwMode); SV(_state.Sa1BwWriteEnabled); SV(_state.BwWriteProtectedArea); SV(_state.BwRam2BppMode);
	SV(_state.CpuIRamWriteProtect); SV(_state.Sa1IRamWriteProtect); SV(_state.DmaSrcAddr); SV(_state.DmaDestAddr); SV(_state.DmaSize); SV(_state.DmaEnabled); SV(_state.DmaPriority);
	SV(_state.DmaCharConv); SV(_state.DmaCharConvAuto); SV(_state.DmaDestDevice); SV(_state.DmaSrcDevice); SV(_state.DmaRunning); SV(_state.DmaIrqFlag); SV(_state.HorizontalTimerEnabled);
	SV(_state.VerticalTimerEnabled); SV(_state.UseLinearTimer); SV(_state.HTimer); SV(_state.VTimer); SV(_state.LinearTimerValue); SV(_state.MathOp); SV(_state.MultiplicandDividend);
	SV(_state.MultiplierDivisor); SV(_state.MathOpResult); SV(_state.MathOverflow); SV(_state.VarLenAutoInc); SV(_state.VarLenBitCount); SV(_state.VarLenAddress);
	SV(_state.Banks[0]); SV(_state.Banks[1]); SV(_state.Banks[2]); SV(_state.Banks[3]);
	SV(_state.BitmapRegister1[0]); SV(_state.BitmapRegister1[1]); SV(_state.BitmapRegister1[2]); SV(_state.BitmapRegister1[3]);
	SV(_state.BitmapRegister1[4]); SV(_state.BitmapRegister1[5]); SV(_state.BitmapRegister1[6]); SV(_state.BitmapRegister1[7]);
	SV(_state.BitmapRegister2[0]); SV(_state.BitmapRegister2[1]); SV(_state.BitmapRegister2[2]); SV(_state.BitmapRegister2[3]);
	SV(_state.BitmapRegister2[4]); SV(_state.BitmapRegister2[5]); SV(_state.BitmapRegister2[6]); SV(_state.BitmapRegister2[7]);
	SV(_state.CharConvDmaActive); SV(_state.CharConvBpp); SV(_state.CharConvFormat); SV(_state.CharConvWidth); SV(_state.CharConvCounter);
	SV(_state.VarLenCurrentBit);

	SV(_lastAccessMemType); SV(_openBus);
	SVArray(_iRam, Sa1::InternalRamSize);

	if(!s.IsSaving()) {
		UpdatePrgRomMappings();
		UpdateSaveRamMappings();
		ProcessInterrupts();
	}
}