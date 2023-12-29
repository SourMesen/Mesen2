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
#include "Debugger/MemoryDumper.h"

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

static PyObject* PythonEmuLog(PyObject* self, PyObject* args);

static PyObject* PythonRead8(PyObject* self, PyObject* args);

static PyMethodDef MyMethods[] = {
	{"log", PythonEmuLog, METH_VARARGS, "Logging function"},
//	{"read8", PythonRead8, METH_VARARGS, "Read 8-bit value from memory"},
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

static PyObject* PythonEmuLog(PyObject* self, PyObject* args)
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

static PyObject* PythonRead8(PyObject* self, PyObject* args)
{
	if (!s_context)
		return nullptr;

	int address, kind;
	PyObject* py_signed;

	// Unpack the Python arguments. Expected: two integers and a boolean
	if(!PyArg_ParseTuple(args, "iiO", &address, &kind, &py_signed))
		return nullptr;

	// Check if py_signed is a boolean
	if (!PyBool_Check(py_signed))
	{
		PyErr_SetString(PyExc_TypeError, "Third argument must be a boolean");
		return nullptr;
	}

	// Convert the PyObject to a C++ boolean
	bool sgn = PyObject_IsTrue(py_signed);

	uint8_t result = 0;
	bool success = s_context->ReadMemory(address, static_cast<MemoryType>(kind), sgn, result);

	// Convert the result to a Python object based on is_signed
	if (sgn)
		return PyLong_FromLong(static_cast<int8_t>(result));
	else
		return PyLong_FromUnsignedLong(result);
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
		PyObject* pRepr = PyObject_Repr(pExcValue);
		Log(PyUnicode_AsUTF8(pRepr));
		Py_DecRef(pRepr);
		Py_DecRef(pExcTraceback);
	}
}


void PythonScriptingContext::InitializePython()
{
	if(Py_IsInitialized())
		return;

	PyImport_AppendInittab("emu", PyInit_mesen);
	Py_Initialize();

	string startupPath = FolderUtilities::GetHomeFolder();
	startupPath += "\\";
	startupPath += "PythonStartup.py";
	string startupScript = ReadFileContents(startupPath);

	if(startupScript.empty())
	{
		Log("Failed to load Python startup script.");
	}
	else
	{
		int error = PyRun_SimpleString(startupScript.c_str());

		if(error)
			LogError();
	}
}


string PythonScriptingContext::ReadFileContents(const string& path)
{
	FILE* f = nullptr;
	auto err = fopen_s(&f, path.c_str(), "r");

	if(err != 0 || f == nullptr) {
		std::cerr << "Failed to open file." << std::endl;
		return ""; // Or handle the error as needed
	}

	std::stringstream buffer;
	char ch;

	while((ch = fgetc(f)) != EOF) {
		buffer << ch;
	}

	fclose(f);
	return buffer.str();
}

bool PythonScriptingContext::LoadScript(string scriptName, string path, string scriptContent, Debugger*)
{
	_scriptName = scriptName;

	int error = PyRun_SimpleString(scriptContent.c_str());
	if (error)
	{
		LogError();
		return false;
	}

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