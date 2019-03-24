#include "stdafx.h"
#include "CallstackManager.h"
#include "Debugger.h"
#include "DebugBreakHelper.h"

CallstackManager::CallstackManager(Debugger* debugger)
{
	_debugger = debugger;
}

void CallstackManager::Push(uint32_t srcAddr, uint32_t destAddr, uint32_t returnAddress, StackFrameFlags flags)
{
	if(_callstack.size() >= 511) {
		//Ensure callstack stays below 512 entries - games can use various tricks that could keep making the callstack grow
		_callstack.pop_front();
	}

	StackFrameInfo stackFrame;
	stackFrame.Source = srcAddr;
	stackFrame.Target = destAddr;

	stackFrame.Return = returnAddress;

	stackFrame.Flags = flags;

	_callstack.push_back(stackFrame);
}

void CallstackManager::Pop(uint32_t destAddress)
{
	if(_callstack.empty()) {
		return;
	}

	StackFrameInfo prevFrame = _callstack.back();
	_callstack.pop_back();

	uint32_t returnAddr = prevFrame.Return;

	if(!_callstack.empty() && destAddress != returnAddr) {
		//Mismatch, pop that stack frame and add the new one
		bool foundMatch = false;
		for(int i = (int)_callstack.size() - 1; i >= 0; i--) {
			if(destAddress == _callstack[i].Return) {
				//Found a matching stack frame, unstack until that point
				foundMatch = true;
				for(int j = (int)_callstack.size() - i - 1; j >= 0; j--) {
					_callstack.pop_back();
				}
				break;
			}
		}

		if(!foundMatch) {
			//Couldn't find a matching frame, replace the current one
			Push(returnAddr, destAddress, returnAddr, StackFrameFlags::None);
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
