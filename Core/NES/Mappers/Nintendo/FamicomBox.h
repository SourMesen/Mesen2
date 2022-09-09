#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/InternalRamHandler.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

//SSS-NROM-256 (Famicom box menu board)
//Info from here: http://kevtris.org/mappers/famicombox/index6.html
class FamicomBox : public BaseMapper
{
private:
	uint8_t _regs[8] = {};

protected:
	uint32_t GetDipSwitchCount() override { return 8; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }	
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }
	uint32_t GetInternalRamSize() override { return 0x2000; }

	void InitMapper() override
	{
		_regs[7] = 0xFF;

		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);

		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 8);
	}

	void Reset(bool softReset) override
	{
		for(int i = 0; i < 6; i++) {
			_regs[i] = 0;
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		switch(addr & 7) {
			case 0: 
				/*
					5000R: Device which caused an exception (0 = this device caused the exception)
					-----
					0 - 6.82Hz interrupt source hit
					1 - 8 bit timer expired @ 5003W
					2 - controller(s) were read
					3 - keyswitch was rotated
					4 - money was inserted
					5 - reset button was pressed
					6 - watchdog timer expired (reading either controller resets this timer)
					7 - Pin 4 of "CATV connector" went high/open
				*/
				_regs[0] = 0xFF; // clear all exceptions
				return _regs[0];
			
			case 1:
				//5001R (not implemented)
				break;

			case 2:
				/*
				5002R Dip switch register
					DIP Switches Info:
						0x01 - Self Test.  When on, unit will continuously self-test
						0x02 - Coin timeout period. off = 10 minutes, on = 20 minutes
						0x04 - not used
						0x08 - Famicombox menu time. off = 7 sec, on = 12 sec

						0x10 - attract time
						0x20 - attract time
						00: 12 seconds
						01: 17 seconds
						10: 23 seconds
						11: 7 seconds

						0x40 - Mode
						0x80 - Mode
						00: Key Mode
						01: CATV Mode
						10: Coin Mode
						11: Free Play
				*/
				return GetDipSwitches();
			
			case 3:
				/*5003R
					0 - key position 0 (1 = in this position)
					1 - key position 1
					2 - key position 2
					3 - key position 3
					4 - key position 4
					5 - key position 6
					6 - money system enabled (pin 9 of 3199)
					7 - pin 10 of 3199
				*/
				return 0x00;	// 0, 1 - attract
										// 2
										// 4    - menu
										// 8    - self check and game casette check
										// 10   - lock?
										// 20   - game title & count display
			case 4:
				//5004R - DB-25 pins
				return 0;

			case 5:
				//5005R: The enable for this runs to pin 28 of the expansion connector.
				return 0;

			case 6:
				//5006R: The enable for this runs to pin 27 of the expansion connector.
				return 0;
			
			case 7:
				/*
					0 - TV type selection (1 = game, 0 = TV)
					1 - keyswitch turned (1 = in middle of positions)
					2 - 0 = zapper grounded
					3 - pin 21 of exp. conn. (inverted)
					4 - state of pin 8 of "CATV" connector,  0 = low, 1 = high
					5 - relay position. 0 = position A, 1 = position B
					6 - pin 22 of exp. conn. (inverted)
					7 - 5005.5W (inverted)
				*/
				return 0x22;	// TV type, key not turned, relay B
		}
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//None of this is implemented
		switch(addr & 0x07) {
			case 0:
				/*
					5000W: Exception Trap enable register (reset to 00h on powerup only)
					-----
					0 - 6.82Hz interrupt source (0 = enable)
					1 - 8 bit timer expiration @ 5003W (1 = enable)
					2 - controller reads (1 = enable)
					3 - keyswitch rotation (1 = enable)
					4 - money insertion (1 = enable)
					5 - reset button (1 = enable)
					6 - not used
					7 - "CATV connector" pin 4 detection (1 = enable)
				*/
				break;

			case 1:
				/*
					5001W Money handling register (reset to 00h on power up only)
					-----
					0 - pin 1 of 2D(3199)
					1 - pin 2 of 2D(3199)
					2 - pin 3 of 2D(3199)
					3 - pin 4 of 2D(3199)
					4 - pin 12 of 2D(3199)
					5 - pin 14 of 2D(3199)
					6 - enable pin 7 of "CATV" connector, 1 = enable.when in TV mode, output goes low. else output stays high
					7 - inverter->pin 8 of "CATV" connector
				*/
				break;

			case 2:
				/*
					5002W LED & memory protect register (reset to 00h when CPU is reset)
					-----

					0 - LED sel 0
					1 - LED sel 1
					2 - LED sel 2
					3 - LED sel 3
					4 - Mem Protect 0
					5 - Mem protect 1
					6 - Mem protect 2
					7 - LED flash- high = flash, low = steady

					LED sel: 01-0fh selects one of the LEDs on the front, 0 = no LEDs

					Mem protect:

					0   - no RAM is writable
					1   - 0000-07ffh is writable
					2   - 0000-0fffh is writable
					3   - 0000-17ffh is writable
					4-7 - 0000-1fffh is writable
				*/
				break;

			case 3:
				/*
					5003W 8 bit down counter, for attract mode timing
					-----

					Writing to this location loads the counter with the 8 bit value written                                      .
					It is clocked at a 6.8274Hz rate.  When the timer wraps from 00h to ffh
					it triggers an exception, if it is enabled. (See 5000W and 5000R)
					This timer is used for the "attract" mode.  It lets the game run for
					several seconds before it brings the menu back.
				*/
				break;

			case 4:
				/*
					5004W Cart control register (reset to 00h when CPU is reset)
					-----

					0 - cart 0
					1 - cart 1
					2 - cart 2
					3 - cart 3
					4 - row select 0
					5 - row select 1
					6 - Lock.  writing a 1 here unmaps all hardware
					7 - NC

					cart:  These 4 bits select which cartridge is active.  Cartridge 0 is the
					menu cart inside the unit, carts 01-0fh are the carts on the front.

					row select: These bits select which row of carts is active.

					0 - only the internal menu cart is selected.
					1 - carts 1-5 are selected
					2 - carts 6-10 are selected
					3 - carts 11-15 are selected

					Proper cart selection requires setting up BOTH the desired cart # (00-0fh), AND
					the proper row.  The menu software uses a small table to do this.
				*/
				break;

			case 5:
				/*
					5005W CATV and joypad control (reset to 00h on power up only)
					-----
					0 - 1 = flip latching relay to position A (coil on pins 1 & 10)
					1 - connects to 3199
					2 - turn zapper on. 1 = turn on
					3 - enable 40% input of modulator.  1 = enable, 0 = disable
					4 - NC
					5 - maps to 5007R.7
					6 - joypad enable- 1 = disable, 0 = enable
					7 - joypad swap. 1 = normal, 0 = swap        swapping only swaps D0 and CLK
				*/
				break;

			case 6:
				/*
					5006W (not reset, unknown contents at powerup)
					-----
					0 - DB-25 pin 6
					1 - DB-25 pin 15
					2 - DB-25 pin 7
					3 - DB-25 pin 16
					4 - DB-25 pin 8
					5 - DB-25 pin 17
					6 - DB-25 pin 9
					7 - DB-25 pin 18
				*/
				break;

			case 7:
				/*
					5007W (not implemented)
					-----
					The enable for this runs to pin 26 of the expansion connector.
				*/
				break;
		}
	}
};
