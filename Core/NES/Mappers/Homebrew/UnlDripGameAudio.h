#pragma once
#include "pch.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"

class UnlDripGameAudio : public BaseExpansionAudio
{
private:
	uint8_t _buffer[256] = {};
	uint8_t _readPos = 0;
	uint8_t _writePos = 0;
	bool _bufferFull = false;
	bool _bufferEmpty = false;

	uint16_t _freq = 0;
	uint16_t _timer = 0;
	uint8_t _volume = 0;
	int16_t _prevOutput = 0;

protected:
	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);
		SVArray(_buffer, 256);
		SV(_readPos);
		SV(_writePos);
		SV(_bufferFull);
		SV(_bufferEmpty);
		SV(_freq);
		SV(_timer);
		SV(_volume);
		SV(_prevOutput);
	}

	void ClockAudio() override
	{
		if(_bufferEmpty) {
			return;
		}

		_timer--;
		if(_timer == 0) {
			//Each time the timer reaches zero, it is reloaded and a byte is removed from the
			//channel's FIFO and is output (with 0x80 being the 'center' voltage) at the
			//channel's specified volume.
			_timer = _freq;

			if(_readPos == _writePos) {
				_bufferFull = false;
			}

			_readPos++;
			SetOutput(((int)_buffer[_readPos] - 0x80) * _volume);

			if(_readPos == _writePos) {
				_bufferEmpty = true;
			}
		}
	}

	void SetOutput(int16_t output)
	{
		_console->GetApu()->AddExpansionAudioDelta(AudioChannel::VRC7, (output - _prevOutput) * 3);
		_prevOutput = output;
	}

	void ResetBuffer()
	{
		memset(_buffer, 0, 256);
		_readPos = 0;
		_writePos = 0;
		_bufferFull = false;
		_bufferEmpty = true;
	}

public:
	UnlDripGameAudio(NesConsole* console) : BaseExpansionAudio(console)
	{
		_freq = 0;
		_timer = 0;
		_volume = 0;
		_prevOutput = 0;
		ResetBuffer();
	}

	uint8_t ReadRegister()
	{
		uint8_t result = 0;
		if(_bufferFull) {
			result |= 0x80;
		}
		if(_bufferEmpty) {
			result |= 0x40;
		}
		return result;
	}

	void WriteRegister(uint16_t addr, uint8_t value)
	{
		switch(addr & 0x03) {
			case 0:
				//Writing any value will silence the corresponding sound channel
				//When a channel's Clear FIFO register is written to, its timer is reset to the
				//last written frequency and it is silenced, outputting a 'center' voltage.
				ResetBuffer();
				SetOutput(0);
				_timer = _freq;
				break;

			case 1:
				//Writing a value will insert it into the FIFO.
				if(_readPos == _writePos) {
					//When data is written to an empty channel's Data Port, the channel's timer is
					//reloaded from the Period registers and playback begins immediately.
					_bufferEmpty = false;
					SetOutput((value - 0x80) * _volume);
					_timer = _freq;
				}

				_buffer[_writePos++] = value;
				if(_readPos == _writePos) {
					_bufferFull = true;
				}
				break;

			case 2:
				//Specifies channel playback rate, in cycles per sample (lower 8 bits)
				_freq = (_freq & 0x0F00) | value;
				break;

			case 3:
				//Specifies channel playback rate, in cycles per sample (higher 8 bits) (bits 0-3)
				//Specifies channel playback volume (bits 4-7)
				_freq = (_freq & 0xFF) | ((value & 0x0F) << 8);
				_volume = (value & 0xF0) >> 4;

				if(!_bufferEmpty) {
					//Updates to a channel's Period do not take effect until the current
					//sample has finished playing, but updates to a channel's Volume take effect immediately.
					SetOutput(((int)_buffer[_readPos] - 0x80) * _volume);
				}
				break;
		}
	}
};
