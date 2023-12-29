#pragma once
#include "pch.h"

#include "ScriptingContext.h"

#ifdef _WIN32
/* struct timeval */
#include <winsock2.h>
#endif

#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_DEFINED
#endif

#include "Python.h"

#ifdef DEBUG_WAS_DEFINED
#define _DEBUG
#endif


class PythonInterpreterHolder
{
private:
	volatile LONG* _count = nullptr;
	PyThreadState* _old = nullptr;

public:
	PythonInterpreterHolder(PyThreadState* state, volatile LONG* count)
	{
		if(state)
		{
			_count = count;
			InterlockedIncrement(_count);
			_old = PyThreadState_Swap(state);
		}
	}

	PythonInterpreterHolder(PythonInterpreterHolder&& other)
	{
		_count = other._count;
		_old = other._old;

		other._count = nullptr;
		other._old = nullptr;
	}

	~PythonInterpreterHolder()
	{
		if (_old)
			PyThreadState_Swap(_old);

		if(_count)
			InterlockedDecrement(_count);
	}
};

class PythonInterpreterHandler
{
private:
	PyThreadState* _python = nullptr;
	volatile LONG _count = 0;

public:
	PythonInterpreterHandler() {}
	PythonInterpreterHandler(PyThreadState* python) { _python = python; }
	PythonInterpreterHandler(PythonInterpreterHandler& other) = delete;

	PythonInterpreterHandler& operator=(PythonInterpreterHandler&& other)
	{
		if(this != &other)
		{
			// Move the state from 'other' to 'this'
			_python = other._python;
			_count = other._count;

			// Reset the state of 'other'
			other._python = nullptr;
			other._count = 0;
		}
		return *this;
	}

	void Detach()
	{
		if(_python)
		{
			Py_EndInterpreter(_python);
			_python = nullptr;
		}
	}

	~PythonInterpreterHandler()
	{
		Detach();
	}

	PythonInterpreterHolder AcquireSafe()
	{
		return PythonInterpreterHolder(_python, &_count);
	}

	bool IsExecuting() { return _count > 0; }
};

class PythonScriptingContext : public IScriptingContext
{
protected:
	struct MemoryRegistry
	{
		uint8_t* BaseAddress;
		MemoryType Type;
		std::vector<int> Addresses;
	};


protected:
	PythonInterpreterHandler _python;

	string _scriptName;
	deque<string> _logRows;
	SimpleLock _logLock;

	Debugger* _debugger = nullptr;
	MemoryDumper* _memoryDumper = nullptr;
	EmuSettings* _settings = nullptr;
	CpuType _defaultCpuType = {};
	MemoryType _defaultMemType = {};

	bool _allowSaveState = true;

	ScriptDrawSurface _drawSurface = ScriptDrawSurface::ConsoleScreen;
	vector<PyObject*> _eventCallbacks[(int)EventType::LastValue + 1];

	void LogError();

	std::vector<MemoryRegistry> _frameMemory;

	void FillOneFrameMemory(const MemoryRegistry& reg);
	void UpdateFrameMemory();

public:
	// Python apis
	bool ReadMemory(uint32_t addr, MemoryType mem, bool sgned, uint8_t& result);
	void* RegisterFrameMemory(MemoryType type, const std::vector<int>& addresses);
	bool UnregisterFrameMemory(void* ptr);

public:
	PythonScriptingContext(Debugger* debugger);
	~PythonScriptingContext();
	bool LoadScript(string scriptName, string path, string scriptContent, Debugger* debugger) override;

	void Log(const string& message) override;
	string GetLog() override;

	Debugger* GetDebugger() override;
	string GetScriptName() override;

	void SetDrawSurface(ScriptDrawSurface surface) override { _drawSurface = surface; }
	ScriptDrawSurface GetDrawSurface() override { return _drawSurface; }

	void CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType) override;
	void CallMemoryCallback(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType) override;

	int CallEventCallback(EventType type, CpuType cpuType) override;
	bool CheckInitDone() override;
	bool IsSaveStateAllowed() override;

	CpuType GetDefaultCpuType() override { return _defaultCpuType; }
	MemoryType GetDefaultMemType() override { return _defaultMemType; }

	void RefreshMemoryCallbackFlags() override;

	void RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, PyObject* obj);
	void UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, PyObject* obj);
	void RegisterEventCallback(EventType type, PyObject* obj);
	void UnregisterEventCallback(EventType type, PyObject* obj);
};
