#include "stdafx.h"
#include "Cpu.h"
#include "../Utilities/Timer.h"

int main()
{
	uint8_t* memory = new uint8_t[0x1000000];
	memset(memory, 0, 0x1000000);
	ifstream testRom("..\\bin\\x64\\Debug\\6502_functional_test_v2.bin", ios::binary);
	if(testRom) {
		testRom.read((char*)memory+0x400, 0x10000);
	}

	shared_ptr<Cpu> cpu(new Cpu(memory, false));
	Timer timer;
	while(cpu->GetPc() != 0x32E9) {
		cpu->Exec();
	}

	std::cout << "Time: " << std::to_string(timer.GetElapsedMS()) << std::endl;
	std::cout << "OP Count: " << std::to_string(cpu->opCount) << std::endl;
	std::cout << "OP/sec: " << std::to_string(cpu->opCount * 1000 / timer.GetElapsedMS()) << std::endl;
/*
	memset(memory, 0, 0x1000000);
	ifstream testRom2("..\\bin\\x64\\Debug\\65C02_extended_opcodes_test.bin", ios::binary);
	if(testRom2) {
		testRom2.read((char*)memory, 0x10000);
	}

	cpu.reset(new Cpu(memory, true));
	timer.Reset();
	while(true) {
		cpu->Exec();
	}
	*/
	while(true);
}