#include "stdafx.h"
#include "Cpu.h"
#include "MemoryManager.h"
#include "../Utilities/Timer.h"

int main()
{
	shared_ptr<MemoryManager> memoryManager(new MemoryManager());
	shared_ptr<Cpu> cpu(new Cpu(memoryManager));
	Timer timer;
	while(cpu->opCount < 10000000) {
		cpu->Exec();
	}

	std::cout << "Time: " << std::to_string(timer.GetElapsedMS()) << std::endl;
	std::cout << "OP Count: " << std::to_string(cpu->opCount) << std::endl;
	std::cout << "OP/sec: " << std::to_string(cpu->opCount * 1000 / timer.GetElapsedMS()) << std::endl;
	while(true);
}