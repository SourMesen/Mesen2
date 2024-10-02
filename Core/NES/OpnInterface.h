#pragma once
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "Utilities/Audio/ymfm/ymfm_opn.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class OpnInterface : public ymfm::ymfm_interface, public ISerializable
{
public:
	static constexpr uint32_t ClockRate = 8000000;

private:
	ymfm::ymf288 _opn;
	NesConsole* _console = nullptr;
	vector<uint8_t> _adpcmRom;
	uint32_t _timers[2] = {};
	uint32_t _busyCounter = 0;

public:
	OpnInterface(NesConsole* console, vector<uint8_t>& adpcmRom) : _opn(*this)
	{
		_adpcmRom = adpcmRom;

		//Ensure the size is 8kb
		_adpcmRom.resize(0x2000);

		_console = console;
		_opn.set_fidelity(ymfm::OPN_FIDELITY_MED);
		_opn.reset();
	}

	void ymfm_set_busy_end(uint32_t clocks) override
	{
		_busyCounter = clocks;
	}

	uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override
	{
		if(type == ymfm::access_class::ACCESS_ADPCM_A) {
			return _adpcmRom[address];
		} else {
			return 0;
		}
	}

	bool ymfm_is_busy() override
	{
		return _busyCounter != 0;
	}

	void ymfm_set_timer(uint32_t timerId, int32_t duration) override
	{
		_timers[timerId] = duration < 0 ? 0 : (uint32_t)duration;
	}

	void ymfm_update_irq(bool irqEnabled) override
	{
		if(irqEnabled) {
			_console->GetCpu()->SetIrqSource(IRQSource::Epsm);
		} else {
			_console->GetCpu()->ClearIrqSource(IRQSource::Epsm);
		}
	}

	void Exec()
	{
		if(_busyCounter) {
			_busyCounter--;
		}

		if(_timers[0] && --_timers[0] == 0) {
			m_engine->engine_timer_expired(0);
		}

		if(_timers[1] && --_timers[1] == 0) {
			m_engine->engine_timer_expired(1);
		}
	}

	void Write(uint8_t addr, uint8_t value)
	{
		_opn.write(addr, value);
	}

	uint32_t GetSampleRate()
	{
		return _opn.sample_rate(OpnInterface::ClockRate);
	}

	void GenerateSamples(vector<int16_t>& samples)
	{
		ymfm::ymf288::output_data output;
		_opn.generate(&output, 1);

		samples.push_back(output.data[0] + (output.data[2] >> 2));
		samples.push_back(output.data[1] + (output.data[2] >> 2));
	}

	void Serialize(Serializer& s) override
	{
		vector<uint8_t> opnData;
		ymfm::ymfm_saved_state state = ymfm::ymfm_saved_state(opnData, s.IsSaving());

		SVArray(_timers, 2);
		SV(_busyCounter);

		if(s.IsSaving()) {
			_opn.save_restore(state);
			SVVector(opnData);
		} else {
			SVVector(opnData);
			_opn.save_restore(state);
		}
	}
};
