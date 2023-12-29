#include "pch.h"
#include <algorithm>
#include "Debugger/PythonScriptingContext.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/ScriptManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/SaveStateManager.h"
#include "Utilities/magic_enum.hpp"
#include "Shared/EventType.h"
#include "Debugger/MemoryDumper.h"
#include "PythonApi.h"

PythonScriptingContext* s_context;


void* PythonScriptingContext::RegisterFrameMemory(MemoryType type, const std::vector<int>& addresses)
{
	if (addresses.empty())
		return nullptr;

	uint8_t *result = new uint8_t[addresses.size() * 2];
	memset(result, 0, addresses.size() * sizeof(uint8_t));

	MemoryRegistry reg = {};
	reg.Type = type;
	reg.BaseAddress = result;
	reg.Addresses = addresses;

	_frameMemory.push_back(reg);
	FillOneFrameMemory(reg);

	return result;
}

bool PythonScriptingContext::UnregisterFrameMemory(void* ptr)
{
	size_t count = _frameMemory.size();

	_frameMemory.erase(std::remove_if(_frameMemory.begin(), _frameMemory.end(), [ptr](const MemoryRegistry& entry) { return ptr == entry.BaseAddress; }), _frameMemory.end());

	if (count == _frameMemory.size())
		return false;

	delete[] ptr;
	return true;
}

void PythonScriptingContext::FillOneFrameMemory(const MemoryRegistry& reg)
{
	auto ptr = reg.BaseAddress;
	for(auto addr : reg.Addresses)
	{
		uint8_t value = _memoryDumper->GetMemoryValue(reg.Type, addr, true);
		*ptr++ = value;
	}
}

void PythonScriptingContext::UpdateFrameMemory()
{
	for (auto& reg : _frameMemory)
		FillOneFrameMemory(reg);
}

bool PythonScriptingContext::ReadMemory(uint32_t addr, MemoryType memType, bool sgned, uint8_t& result)
{
	uint8_t value = _memoryDumper->GetMemoryValue(memType, addr, true);
	result = value;
	return true;
}

PythonScriptingContext::PythonScriptingContext(Debugger* debugger)
{
	_debugger = debugger;
	_memoryDumper = debugger->GetMemoryDumper();
	_settings = debugger->GetEmulator()->GetSettings();
	_defaultCpuType = debugger->GetEmulator()->GetCpuTypes()[0];
	_defaultMemType = DebugUtilities::GetCpuMemoryType(_defaultCpuType);

	s_context = this;
}

PythonScriptingContext::~PythonScriptingContext()
{
	s_context = nullptr;

	Py_IsInitialized();
	if (Py_IsInitialized())
		Py_Finalize();
}

void PythonScriptingContext::LogError()
{
	PyObject* pExcType, * pExcValue, * pExcTraceback;
	PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
	if(pExcType != NULL) {
		PyObject* pRepr = PyObject_Repr(pExcType);
		Log(PyUnicode_AsUTF8(pRepr));
		Py_DecRef(pRepr);
		Py_DecRef(pExcType);
	}

	if(pExcValue != NULL) {
		PyObject* pRepr = PyObject_Repr(pExcValue);
		Log(PyUnicode_AsUTF8(pRepr));
		Py_DecRef(pRepr);
		Py_DecRef(pExcValue);
	}

	if(pExcTraceback != NULL) {
		PyObject* pRepr = PyObject_Repr(pExcTraceback);
		Log(PyUnicode_AsUTF8(pRepr));
		Py_DecRef(pRepr);
		Py_DecRef(pExcTraceback);
	}
}


bool PythonScriptingContext::LoadScript(string scriptName, string path, string scriptContent, Debugger*)
{
	if(!InitializePython())
	{
		LogError();
		return false;
	}

	_scriptName = scriptName;

	int error = PyRun_SimpleString(scriptContent.c_str());
	if (error)
	{
		LogError();
		return false;
	}

	return true;
}

void PythonScriptingContext::Log(const string& message)
{
	auto lock = _logLock.AcquireSafe();
	_logRows.push_back(message);
	if(_logRows.size() > 500) {
		_logRows.pop_front();
	}
}

string PythonScriptingContext::GetLog()
{
	auto lock = _logLock.AcquireSafe();
	stringstream ss;
	for(string& msg : _logRows) {
		ss << msg;
	}
	return ss.str();
}

Debugger* PythonScriptingContext::GetDebugger()
{
	return _debugger;
}

string PythonScriptingContext::GetScriptName()
{
	return _scriptName;
}

void PythonScriptingContext::CallMemoryCallback(AddressInfo relAddr, uint8_t& value, CallbackType type, CpuType cpuType)
{
}

void PythonScriptingContext::CallMemoryCallback(AddressInfo relAddr, uint16_t& value, CallbackType type, CpuType cpuType)
{
}

void PythonScriptingContext::CallMemoryCallback(AddressInfo relAddr, uint32_t& value, CallbackType type, CpuType cpuType)
{
}

int PythonScriptingContext::CallEventCallback(EventType type, CpuType cpuType)
{
	if (type == EventType::StartFrame)
		UpdateFrameMemory();

	int count = 0;
	for(PyObject* callback : _eventCallbacks[(int)type])
	{
 		PyObject* pFunc = callback;
		PyObject* pArgs = PyTuple_New(1);
		PyObject* pValue = PyLong_FromLong((int)cpuType);
		if(!pValue) {
			LogError();
			continue;
		}

		PyObject* pResult = PyObject_CallFunctionObjArgs(pFunc, pValue, NULL);
		if(pResult != NULL)
			Py_DECREF(pResult);
		else
			LogError();

		Py_DECREF(pValue);
		count++;
	}

	return count;
}


bool PythonScriptingContext::CheckInitDone()
{
	return Py_IsInitialized();
}

bool PythonScriptingContext::IsSaveStateAllowed()
{
	return _allowSaveState;
}


void PythonScriptingContext::RefreshMemoryCallbackFlags()
{
}

void PythonScriptingContext::RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, PyObject* obj)
{

}
void PythonScriptingContext::UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, PyObject* obj)
{

}
void PythonScriptingContext::RegisterEventCallback(EventType type, PyObject *obj)
{
	_eventCallbacks[(int)type].push_back(obj);
}

void PythonScriptingContext::UnregisterEventCallback(EventType type, PyObject* obj)
{
	_eventCallbacks[(int)type].erase(std::remove(_eventCallbacks[(int)type].begin(), _eventCallbacks[(int)type].end(), obj), _eventCallbacks[(int)type].end());
}