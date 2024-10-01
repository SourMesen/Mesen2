#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Konami/VrcIrq.h"

enum class VRCVariant
{
	VRC2a,	//Mapper 22
	VRC2b,	//23
	VRC2c,	//25
	VRC4a,	//21
	VRC4b,	//25
	VRC4c,	//21
	VRC4d,	//25
	VRC4e,	//23
	VRC4_27, //27
	VRC6a,
	VRC6b
};

class VRC2_4 : public BaseMapper
{
	private:
		unique_ptr<VrcIrq> _irq;
		VRCVariant _variant = {};
		bool _useHeuristics = false;

		uint8_t _prgReg0 = 0;
		uint8_t _prgReg1 = 0;
		uint8_t _prgMode = 0;

		uint8_t _hiCHRRegs[8] = {};
		uint8_t _loCHRRegs[8] = {};

		uint8_t _latch = 0;

		void DetectVariant()
		{
			switch(_romInfo.MapperID) {
				default:
				case 21:
					//Conflicts: VRC4c
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC4a; break;
						case 1: _variant = VRCVariant::VRC4a; break;
						case 2: _variant = VRCVariant::VRC4c; break;
					}
					break;

				case 22: _variant = VRCVariant::VRC2a; break;

				case 23:
					//Conflicts: VRC4e
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC2b; break;
						case 2: _variant = VRCVariant::VRC4e; break;
						case 3: _variant = VRCVariant::VRC2b; break;
					}
					break;

				case 25:
					//Conflicts: VRC2c, VRC4d
					switch(_romInfo.SubMapperID) {
						default:
						case 0: _variant = VRCVariant::VRC4b; break;
						case 1: _variant = VRCVariant::VRC4b; break;
						case 2: _variant = VRCVariant::VRC4d; break;
						case 3: _variant = VRCVariant::VRC2c; break;
					}
					break;

				case 27: _variant = VRCVariant::VRC4_27; break; //Untested
			}

			_useHeuristics = (_romInfo.SubMapperID == 0) && _romInfo.MapperID != 22 && _romInfo.MapperID != 27;
		}

	protected:
		uint16_t GetPrgPageSize() override { return 0x2000; }
		uint16_t GetChrPageSize() override { return 0x0400; }
		bool AllowRegisterRead() override { return true; }
		bool EnableCpuClockHook() override { return true; }

		void InitMapper() override 
		{
			_irq.reset(new VrcIrq(_console));
			DetectVariant();

			//PRG mode only exists for VRC4+ (so keep it as 0 at all times for VRC2)
			_prgMode = _variant >= VRCVariant::VRC4a ? (GetPowerOnByte() & 0x01) : 0;

			_prgReg0 = GetPowerOnByte() & 0x1F;
			_prgReg1 = GetPowerOnByte() & 0x1F;
			_latch = false;

			for(int i = 0; i < 8; i++) {
				_loCHRRegs[i] = GetPowerOnByte() & 0x0F;
				_hiCHRRegs[i] = GetPowerOnByte() & 0x1F;
			}

			UpdateState();

			RemoveRegisterRange(0, 0xFFFF, MemoryOperation::Read);
			if(!_useHeuristics && _variant <= VRCVariant::VRC2c && _workRamSize == 0 && _saveRamSize == 0) {
				AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Any);
			}
		}
		
		void ProcessCpuClock() override
		{
			BaseProcessCpuClock();

			if((_useHeuristics && _romInfo.MapperID != 22) || _variant >= VRCVariant::VRC4a) {
				//Only VRC4 supports IRQs
				_irq->ProcessCpuClock();
			}
		}

		void UpdateState()
		{
			for(int i = 0; i < 8; i++) {
				uint32_t page = _loCHRRegs[i] | (_hiCHRRegs[i] << 4);
				if(_variant == VRCVariant::VRC2a) {
					//"On VRC2a (mapper 022) only the high 7 bits of the CHR regs are used -- the low bit is ignored.  Therefore, you effectively have to right-shift the CHR page by 1 to get the actual page number."
					page >>= 1;
				}
				SelectChrPage(i, page);
			}

			if(_prgMode == 0) {
				SelectPrgPage(0, _prgReg0);
				SelectPrgPage(1, _prgReg1);
				SelectPrgPage(2, -2);
				SelectPrgPage(3, -1);
			} else {
				SelectPrgPage(0, -2);
				SelectPrgPage(1, _prgReg1);
				SelectPrgPage(2, _prgReg0);
				SelectPrgPage(3, -1);
			}
		}

		uint8_t ReadRegister(uint16_t addr) override
		{
			//Microwire interface ($6000-$6FFF) (VRC2 only)
			return _latch | (_console->GetMemoryManager()->GetOpenBus() & 0xFE);
		}

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			if(addr < 0x8000) {
				//Microwire interface ($6000-$6FFF) (VRC2 only)
				_latch = value & 0x01;
				return;
			}

			addr = TranslateAddress(addr) & 0xF00F;

			if(addr >= 0x8000 && addr <= 0x8006) {
				_prgReg0 = value & 0x1F;
			} else if((_variant <= VRCVariant::VRC2c && addr >= 0x9000 && addr <= 0x9003) || (_variant >= VRCVariant::VRC4a && addr >= 0x9000 && addr <= 0x9001)) {
				uint8_t mask = 0x03;
				if(!_useHeuristics && (_variant >= VRCVariant::VRC2a && _variant <= VRCVariant::VRC2c)) {
					//When we are certain this is a VRC2 game, only use the first bit for mirroring selection
					mask = 0x01;
				}

				switch(value & mask) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
			} else if(_variant >= VRCVariant::VRC4a && addr >= 0x9002 && addr <= 0x9003) {
				_prgMode = (value >> 1) & 0x01;
			} else if(addr >= 0xA000 && addr <= 0xA006) {
				_prgReg1 = value & 0x1F;
			} else if(addr >= 0xB000 && addr <= 0xE006) {
				uint8_t regNumber = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
				bool lowBits = (addr & 0x01) == 0x00;
				if(lowBits) {
					//The other reg contains the low 4 bits
					_loCHRRegs[regNumber] = value & 0x0F;
				} else {
					//One reg contains the high 5 bits 
					_hiCHRRegs[regNumber] = value & 0x1F;
				}
			} else if(addr == 0xF000) {
				_irq->SetReloadValueNibble(value, false);
			} else if(addr == 0xF001) {
				_irq->SetReloadValueNibble(value, true);
			} else if(addr == 0xF002) {
				_irq->SetControlValue(value);
			} else if(addr == 0xF003) {
				_irq->AcknowledgeIrq();
			}

			UpdateState();
		}

	public:		
		uint16_t TranslateAddress(uint16_t addr)
		{
			uint32_t A0, A1;

			if(_useHeuristics) {
				switch(_variant) {
					case VRCVariant::VRC2c:
					case VRCVariant::VRC4b:
					case VRCVariant::VRC4d:
						//Mapper 25
						//ORing both values should make most games work.
						//VRC2c & VRC4b (Both uses the same bits)
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);

						//VRC4d
						A0 |= (addr >> 3) & 0x01;
						A1 |= (addr >> 2) & 0x01;
						break;
					case VRCVariant::VRC4a:
					case VRCVariant::VRC4c:
						//Mapper 21
						//VRC4a
						A0 = (addr >> 1) & 0x01;
						A1 = (addr >> 2) & 0x01;

						//VRC4c
						A0 |= (addr >> 6) & 0x01;
						A1 |= (addr >> 7) & 0x01;
						break;

					case VRCVariant::VRC2b:
					case VRCVariant::VRC4e:
						//Mapper 23
						//VRC2b
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;

						//VRC4e
						A0 |= (addr >> 2) & 0x01;
						A1 |= (addr >> 3) & 0x01;
						break;
					default:
						throw std::runtime_error("not supported");
						break;
				}
			} else {
				switch(_variant) {
					case VRCVariant::VRC2a:
						//Mapper 22
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);
						break;

					case VRCVariant::VRC4_27:
						//Mapper 27
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC2c:
					case VRCVariant::VRC4b:
						//Mapper 25
						A0 = (addr >> 1) & 0x01;
						A1 = (addr & 0x01);
						break;

					case VRCVariant::VRC4d:
						//Mapper 25
						A0 = (addr >> 3) & 0x01;
						A1 = (addr >> 2) & 0x01;
						break;

					case VRCVariant::VRC4a:
						//Mapper 21
						A0 = (addr >> 1) & 0x01;
						A1 = (addr >> 2) & 0x01;
						break;

					case VRCVariant::VRC4c:
						//Mapper 21
						A0 = (addr >> 6) & 0x01;
						A1 = (addr >> 7) & 0x01;
						break;

					case VRCVariant::VRC2b:
						//Mapper 23
						A0 = addr & 0x01;
						A1 = (addr >> 1) & 0x01;
						break;

					case VRCVariant::VRC4e:
						//Mapper 23
						A0 = (addr >> 2) & 0x01;
						A1 = (addr >> 3) & 0x01;
						break;

					default:
						throw std::runtime_error("not supported");
						break;
				}

			}

			return (addr & 0xFF00) | (A1 << 1) | A0;
		}

		void Serialize(Serializer& s) override
		{
			BaseMapper::Serialize(s);
			SVArray(_loCHRRegs, 8);
			SVArray(_hiCHRRegs, 8);
			SV(_prgReg0);
			SV(_prgReg1);
			SV(_prgMode);
			SV(_latch);
			SV(_irq);
		}
};