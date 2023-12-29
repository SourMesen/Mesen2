#include "pch.h"
#include <algorithm>
#include <regex>
#include "Debugger/ScriptingContext.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/ScriptManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/SaveStateManager.h"
#include "Utilities/magic_enum.hpp"
#include "Shared/EventType.h"
#include "Utilities/FolderUtilities.h"

/* struct timeval */
#include <winsock2.h>

#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_DEFINED
#endif

#include "Python.h"

#ifdef DEBUG_WAS_DEFINED
#define _DEBUG
#endif

static PyObject* EmuLog(PyObject* self, PyObject* args);

static PyMethodDef MyMethods[] = {
	{"log", EmuLog, METH_VARARGS, "Logging function"},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef emumodule = {
	 PyModuleDef_HEAD_INIT,
	 "emu",
	 NULL,
	 -1,
	 MyMethods
};


PyMODINIT_FUNC PyInit_mesen(void)
{
	return PyModule_Create(&emumodule);
}

static PythonScriptingContext* s_context = nullptr;

static PyObject* EmuLog(PyObject* self, PyObject* args)
{
	const char* message;
	if(!PyArg_ParseTuple(args, "s", &message)) {
		return nullptr;
	}

	if(!s_context)
		return nullptr;

	s_context->Log(message);

	Py_RETURN_NONE;
}



PythonScriptingContext::PythonScriptingContext(Debugger* debugger)
{
	_debugger = debugger;
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
		PyObject* pRepr = PyObject_Repr(pExcValue);
		Log(PyUnicode_AsUTF8(pRepr));
		Py_DecRef(pRepr);
		Py_DecRef(pExcTraceback);
	}
}

bool PythonScriptingContext::LoadScript(string scriptName, string path, string scriptContent, Debugger*)
{
	_scriptName = scriptName;

	PyImport_AppendInittab("emu", PyInit_mesen);
	if(!Py_IsInitialized())
		Py_Initialize();

	FILE* f = nullptr;
	string startup = FolderUtilities::GetHomeFolder();
	startup += "\\";
	startup += "PythonStartup.py";
	auto err = fopen_s(&f, startup.c_str(), "r");

	if(err != 0 || f == nullptr) {
		std::cerr << "Failed to open file." << std::endl;
		return 1; // Or handle the error as needed
	}

	std::stringstream buffer;
	char ch;

	while((ch = fgetc(f)) != EOF) {
		buffer << ch;
	}

	fclose(f);
	string str = buffer.str();

	int error = PyRun_SimpleString(str.c_str());
	
	if (error)
		LogError();

	error = PyRun_SimpleString(scriptContent.c_str());
	if(error)
		LogError();

	return true;
}

void PythonScriptingContext::Log(string message)
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
		ss << msg << "\n";
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
	return 0;
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

void PythonScriptingContext::RegisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference)
{

}
void PythonScriptingContext::UnregisterMemoryCallback(CallbackType type, int startAddr, int endAddr, MemoryType memType, CpuType cpuType, int reference)
{

}
void PythonScriptingContext::RegisterEventCallback(EventType type, int reference)
{

}

void PythonScriptingContext::UnregisterEventCallback(EventType type, int reference)
{

}