#include "pch.h"
#include "WS/WsSerial.h"
#include "WS/WsConsole.h"
#include "Utilities/Serializer.h"

WsSerial::WsSerial(WsConsole* console)
{
	_console = console;
}

uint8_t WsSerial::Read(uint16_t port)
{
	switch(port) {
		case 0xB1: return _state.ReceiveBuffer;

		case 0xB3:
			UpdateState();
			return (
				(_state.HasReceiveData ? 0x01 : 0) |
				(_state.ReceiveOverflow ? 0x02 : 0) |
				(_state.HasSendData ? 0 : 0x04) |
				(_state.HighSpeed ? 0x40 : 0) |
				(_state.Enabled ? 0x80 : 0)
			);
	}

	return 0;
}

void WsSerial::Write(uint16_t port, uint8_t value)
{
	switch(port) {
		case 0xB1:
			_state.SendBuffer = value;
			_state.HasSendData = true;
			_state.SendClock = _console->GetMasterClock();
			break;

		case 0xB3:
			_state.Enabled = value & 0x80;
			_state.HighSpeed = value & 0x40;
			if(value & 0x20) {
				_state.ReceiveOverflow = false;
			}
			break;
	}
}

void WsSerial::UpdateState()
{
	if(_state.HasSendData) {
		//At 9600 bauds/s, it takes 3200 master clocks to send 1 byte
		//At 38400 (high speed mode), it takes 800 master clocks
		int cyclesPerByte = _state.HighSpeed ? 800 : 3200;
		uint64_t cyclesElapsed = _console->GetMasterClock() - _state.SendClock;
		if(cyclesElapsed > cyclesPerByte) {
			_state.HasSendData = false;
		}
	}
}

bool WsSerial::HasSendIrq()
{
	if(_state.HasSendData) {
		UpdateState();
	}
	return _state.Enabled && !_state.HasSendData;
}

void WsSerial::Serialize(Serializer& s)
{
	SV(_state.Enabled);
	SV(_state.HighSpeed);
	SV(_state.HasReceiveData);
	SV(_state.HasSendData);
	SV(_state.SendBuffer);
	SV(_state.SendClock);
	SV(_state.ReceiveBuffer);
	SV(_state.ReceiveOverflow);
}
