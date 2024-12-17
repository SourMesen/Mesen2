#include "pch.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/IDebugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Profiler.h"

CallstackManager::CallstackManager(Debugger* debugger, IDebugger* cpuDebugger)
{
	_debugger = debugger;
	_profiler.reset(new Profiler(debugger, cpuDebugger));
}

CallstackManager::~CallstackManager()
{
}

void CallstackManager::Push(AddressInfo &src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t returnAddress, uint32_t returnStackPointer, StackFrameFlags flags)
{
	if(_callstack.size() >= 511) {
		//Ensure callstack stays below 512 entries - games can use various tricks that could keep making the callstack grow
		_callstack.pop_front();
	}

	StackFrameInfo stackFrame;
	stackFrame.Source = srcAddr;
	stackFrame.AbsSource = src;
	stackFrame.Target = destAddr;
	stackFrame.AbsTarget = dest;
	stackFrame.Return = returnAddress;
	stackFrame.ReturnStackPointer = returnStackPointer;
	stackFrame.AbsReturn = ret;

	stackFrame.Flags = flags;

	_callstack.push_back(stackFrame);
	_profiler->StackFunction(dest, flags);
}

void CallstackManager::Pop(AddressInfo& dest, uint32_t destAddress, uint32_t stackPointer)
{
	if(_callstack.empty()) {
		return;
	}

	StackFrameInfo prevFrame = _callstack.back();
	_callstack.pop_back();
	_profiler->UnstackFunction();

	uint32_t returnAddr = prevFrame.Return;

	if(!_callstack.empty() && destAddress != returnAddr) {
		//Mismatch, try to find a matching address higher in the stack
		bool foundMatch = false;
		for(int i = (int)_callstack.size() - 1; i >= 0; i--) {
			if(destAddress == _callstack[i].Return) {
				//Found a matching stack frame, unstack until that point
				foundMatch = true;
				for(int j = (int)_callstack.size() - i - 1; j >= 0; j--) {
					_callstack.pop_back();
					_profiler->UnstackFunction();
				}
				break;
			}
		}

		if(!foundMatch) {
			//Couldn't find a matching frame
			//If the new stack pointer doesn't match the last frame, push a new frame for it
			//Otherwise, presume that the code has returned to the last function on the stack
			//This can happen in some patterns, e.g if putting call parameters after the JSR
			//call, and manipulating the stack upon return to return to the code after the
			//parameters.
			if(_callstack.back().ReturnStackPointer != stackPointer) {
				Push(prevFrame.AbsReturn, returnAddr, dest, destAddress, prevFrame.AbsReturn, returnAddr, stackPointer, StackFrameFlags::None);
			}
		}
	}
}

void CallstackManager::GetCallstack(StackFrameInfo* callstackArray, uint32_t &callstackSize)
{
	DebugBreakHelper helper(_debugger);
	int i = 0;
	for(StackFrameInfo &info : _callstack) {
		callstackArray[i] = info;
		i++;
	}
	callstackSize = i;
}

int32_t CallstackManager::GetReturnAddress()
{
	DebugBreakHelper helper(_debugger);
	if(_callstack.empty()) {
		return -1;
	}
	return _callstack.back().Return;
}

int64_t CallstackManager::GetReturnStackPointer()
{
	DebugBreakHelper helper(_debugger);
	if(_callstack.empty()) {
		return -1;
	}
	return _callstack.back().ReturnStackPointer;
}

Profiler* CallstackManager::GetProfiler()
{
	return _profiler.get();
}

void CallstackManager::Clear()
{
	_callstack.clear();
	_profiler->ResetState();
}
