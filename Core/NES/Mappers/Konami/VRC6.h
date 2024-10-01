#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Konami/VrcIrq.h"
#include "NES/Mappers/Audio/Vrc6Audio.h"

enum class VRCVariant;

class VRC6 : public BaseMapper
{
private:
	unique_ptr<VrcIrq> _irq;
	unique_ptr<Vrc6Audio> _audio;

	VRCVariant _model = {};
	uint8_t _bankingMode = 0;
	uint8_t _chrRegisters[8] = {};
	
	void UpdatePrgRamAccess()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, (_bankingMode & 0x80) ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess);
	}

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_audio.reset(new Vrc6Audio(_console));
		_irq.reset(new VrcIrq(_console));

		_irq->Reset();
		_audio->Reset();
		memset(_chrRegisters, 0, sizeof(_chrRegisters));
		SelectPrgPage(3, -1);
	}
	
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_chrRegisters, 8);
		SV(_irq);
		SV(_audio);
		SV(_bankingMode);

		if(!s.IsSaving()) {
			UpdatePrgRamAccess();
			UpdatePpuBanking();
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();
		_irq->ProcessCpuClock();
		_audio->Clock();
	}

	void SetPpuMapping(uint8_t bank, uint8_t page)
	{
		SetPpuMemoryMapping(0x2000 + bank * 0x400, 0x23FF + bank * 0x400, page);
		SetPpuMemoryMapping(0x3000 + bank * 0x400, 0x33FF + bank * 0x400, page);
	}

	void UpdatePpuBanking()
	{
		uint8_t mask = (_bankingMode & 0x20) ? 0xFE : 0xFF;
		uint8_t orMask = (_bankingMode & 0x20) ? 1 : 0;

		switch(_bankingMode & 0x03) {
			case 0:
				SelectChrPage(0, _chrRegisters[0]);
				SelectChrPage(1, _chrRegisters[1]);
				SelectChrPage(2, _chrRegisters[2]);
				SelectChrPage(3, _chrRegisters[3]);
				SelectChrPage(4, _chrRegisters[4]);
				SelectChrPage(5, _chrRegisters[5]);
				SelectChrPage(6, _chrRegisters[6]);
				SelectChrPage(7, _chrRegisters[7]);
				break;

			case 1:
				SelectChrPage(0, _chrRegisters[0] & mask);
				SelectChrPage(1, (_chrRegisters[0] & mask) | orMask);
				SelectChrPage(2, _chrRegisters[1] & mask);
				SelectChrPage(3, (_chrRegisters[1] & mask) | orMask);
				SelectChrPage(4, _chrRegisters[2] & mask);
				SelectChrPage(5, (_chrRegisters[2] & mask) | orMask);
				SelectChrPage(6, _chrRegisters[3] & mask);
				SelectChrPage(7, (_chrRegisters[3] & mask) | orMask);
				break;

			case 2: case 3:
				SelectChrPage(0, _chrRegisters[0]);
				SelectChrPage(1, _chrRegisters[1]);
				SelectChrPage(2, _chrRegisters[2]);
				SelectChrPage(3, _chrRegisters[3]);
				SelectChrPage(4, _chrRegisters[4] & mask);
				SelectChrPage(5, (_chrRegisters[4] & mask) | orMask);
				SelectChrPage(6, _chrRegisters[5] & mask);
				SelectChrPage(7, (_chrRegisters[5] & mask) | orMask);
				break;
		}
		
		if(_bankingMode & 0x10) {
			//CHR ROM nametables
			switch(_bankingMode & 0x2F) {
				case 0x20:
				case 0x27:
					SetPpuMapping(0, _chrRegisters[6] & 0xFE);
					SetPpuMapping(1, (_chrRegisters[6] & 0xFE) | 1);
					SetPpuMapping(2, _chrRegisters[7] & 0xFE);
					SetPpuMapping(3, (_chrRegisters[7] & 0xFE) | 1);
					break;

				case 0x23:
				case 0x24:
					SetPpuMapping(0, (_chrRegisters[6] & 0xFE));
					SetPpuMapping(1, (_chrRegisters[7] & 0xFE));
					SetPpuMapping(2, (_chrRegisters[6] & 0xFE) | 1);
					SetPpuMapping(3, (_chrRegisters[7] & 0xFE) | 1);
					break;

				case 0x28:
				case 0x2F:
					SetPpuMapping(0, _chrRegisters[6] & 0xFE);
					SetPpuMapping(1, _chrRegisters[6] & 0xFE);
					SetPpuMapping(2, _chrRegisters[7] & 0xFE);
					SetPpuMapping(3, _chrRegisters[7] & 0xFE);
					break;

				case 0x2B:
				case 0x2C:
					SetPpuMapping(0, (_chrRegisters[6] & 0xFE) | 1);
					SetPpuMapping(1, (_chrRegisters[7] & 0xFE) | 1);
					SetPpuMapping(2, (_chrRegisters[6] & 0xFE) | 1);
					SetPpuMapping(3, (_chrRegisters[7] & 0xFE) | 1);
					break;

				default:
					switch(_bankingMode & 0x07) {
						case 0:
						case 6:
						case 7:
							SetPpuMapping(0, _chrRegisters[6]);
							SetPpuMapping(1, _chrRegisters[6]);
							SetPpuMapping(2, _chrRegisters[7]);
							SetPpuMapping(3, _chrRegisters[7]);
							break;

						case 1:
						case 5:
							SetPpuMapping(0, _chrRegisters[4]);
							SetPpuMapping(1, _chrRegisters[5]);
							SetPpuMapping(2, _chrRegisters[6]);
							SetPpuMapping(3, _chrRegisters[7]);
							break;

						case 2:
						case 3:
						case 4:
							SetPpuMapping(0, _chrRegisters[6]);
							SetPpuMapping(1, _chrRegisters[7]);
							SetPpuMapping(2, _chrRegisters[6]);
							SetPpuMapping(3, _chrRegisters[7]);
							break;
					}
					break;
			}
		} else {
			//Regular nametables (CIRAM)
			switch(_bankingMode & 0x2F) {
				case 0x20:
				case 0x27:
					SetMirroringType(MirroringType::Vertical);
					break;

				case 0x23:
				case 0x24:
					SetMirroringType(MirroringType::Horizontal);
					break;

				case 0x28:
				case 0x2F:
					SetMirroringType(MirroringType::ScreenAOnly);
					break;

				case 0x2B:
				case 0x2C:
					SetMirroringType(MirroringType::ScreenBOnly);
					break;

				default:
					switch(_bankingMode & 0x07) {
						case 0:
						case 6:
						case 7:
							SetNametable(0, _chrRegisters[6] & 0x01);
							SetNametable(1, _chrRegisters[6] & 0x01);
							SetNametable(2, _chrRegisters[7] & 0x01);
							SetNametable(3, _chrRegisters[7] & 0x01);
							break;

						case 1:
						case 5:
							SetNametable(0, _chrRegisters[4] & 0x01);
							SetNametable(1, _chrRegisters[5] & 0x01);
							SetNametable(2, _chrRegisters[6] & 0x01);
							SetNametable(3, _chrRegisters[7] & 0x01);
							break;

						case 2:
						case 3:
						case 4:
							SetNametable(0, _chrRegisters[6] & 0x01);
							SetNametable(1, _chrRegisters[7] & 0x01);
							SetNametable(2, _chrRegisters[6] & 0x01);
							SetNametable(3, _chrRegisters[7] & 0x01);
							break;
					}
					break;
			}
		}
		UpdatePrgRamAccess();
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_model == VRCVariant::VRC6b) {
			addr = (addr & 0xFFFC) | ((addr & 0x01) << 1) | ((addr & 0x02) >> 1);
		}

		switch(addr & 0xF003) {
			case 0x8000: case 0x8001: case 0x8002: case 0x8003: 
				SelectPrgPage2x(0, (value & 0x0F) << 1); 
				break;

			case 0x9000: case 0x9001: case 0x9002:
			case 0x9003:
			case 0xA000: case 0xA001: case 0xA002:
			case 0xB000: case 0xB001: case 0xB002:
				_audio->WriteRegister(addr, value);
				break;

			case 0xB003:
				_bankingMode = value;
				UpdatePpuBanking();
				break;
				
			case 0xC000: case 0xC001: case 0xC002: case 0xC003:
				SelectPrgPage(2, value & 0x1F);
				break;

			case 0xD000: case 0xD001: case 0xD002: case 0xD003:
				_chrRegisters[addr & 0x03] = value;
				UpdatePpuBanking();
				break;

			case 0xE000: case 0xE001: case 0xE002: case 0xE003:
				_chrRegisters[4 + (addr & 0x03)] = value;
				UpdatePpuBanking();
				break;

			case 0xF000:
				_irq->SetReloadValue(value);
				break;

			case 0xF001:
				_irq->SetControlValue(value);
				break;

			case 0xF002:
				_irq->AcknowledgeIrq();
				break;
		}
	}

public:
	VRC6(VRCVariant model) : _model(model)
	{
	}
};