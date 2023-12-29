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
static PyObject* PythonRegisterFrameMemory(PyObject* self, PyObject* args);
static PyObject* PythonUnregisterFrameMemory(PyObject* self, PyObject* args);
static PyObject* PythonAddEventCallback(PyObject* self, PyObject* args);
static PyObject* PythonRemoveEventCallback(PyObject* self, PyObject* args);

static PyMethodDef MyMethods[] = {
	{"log", PythonEmuLog, METH_VARARGS, "Logging function"},
	{"read8", PythonRead8, METH_VARARGS, "Read 8-bit value from memory"},
	{"registerFrameMemory", PythonRegisterFrameMemory, METH_VARARGS, "Register memory addresses (and sizes) to be tracked and updated every frame."},
	{"unregisterFrameMemory", PythonUnregisterFrameMemory, METH_VARARGS, "Unregister frame memory updates."},
	{"addEventCallback", PythonAddEventCallback, METH_VARARGS, "Adds an event callback.  e.g. emu.addEventCallback(function, emu.eventType.startFrame)"},
	{"removeEventCallback", PythonRemoveEventCallback, METH_VARARGS, "Removes an event callback."},
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
	if(!s_context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	if(!s_context)
		return nullptr;

	s_context->Log(message);

	Py_RETURN_NONE;
}

static PyObject* PythonRead8(PyObject* self, PyObject* args)
{
	if(!s_context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

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
	PyObject* pyResult = nullptr;
	if (sgn)
		pyResult = PyLong_FromLong(static_cast<int8_t>(result));
	else
		pyResult = PyLong_FromUnsignedLong(result);

	return pyResult;
}

static PyObject* PythonRegisterFrameMemory(PyObject* self, PyObject* args)
{
	if(!s_context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	std::vector<std::pair<uint16_t, MemoryType>> result;

	// Check if args is a sequence
	if(!PySequence_Check(args)) {
		PyErr_SetString(PyExc_TypeError, "Expected a sequence");
		return nullptr;
	}

	Py_ssize_t length = PySequence_Size(args);
	for(Py_ssize_t i = 0; i < length; ++i) {
		PyObject* item = PySequence_GetItem(args, i);
		if(!item || !PyTuple_Check(item) || PyTuple_Size(item) != 3) {
			PyErr_SetString(PyExc_TypeError, "Expected a sequence of 3-tuples");
			Py_XDECREF(item);
			return nullptr;
		}

		PyObject* py_address = PyTuple_GetItem(item, 0);
		PyObject* py_kind = PyTuple_GetItem(item, 1);

		if(!py_address || !py_kind || !PyLong_Check(py_address) || !PyLong_Check(py_kind)) {
			PyErr_SetString(PyExc_TypeError, "Each tuple must contain (address, memoryType)");
			Py_XDECREF(item);
			return nullptr;
		}

		uint16_t address = static_cast<uint16_t>(PyLong_AsUnsignedLong(py_address));
		MemoryType kind = static_cast<MemoryType>(PyLong_AsLong(py_kind));

		result.push_back(std::pair(address, kind));
		Py_DECREF(item);
	}

	void* ptr = s_context->RegisterFrameMemory(result);
	if (ptr == nullptr)
		Py_RETURN_NONE;

	return PyLong_FromVoidPtr(ptr);
}

static PyObject* PythonUnregisterFrameMemory(PyObject* self, PyObject* args)
{
	if(!s_context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	void* ptr;
	if(!PyArg_ParseTuple(args, "p", &ptr))
		return nullptr;

	bool result = s_context->UnregisterFrameMemory(ptr);
	if (result)
		Py_RETURN_TRUE;

	Py_RETURN_FALSE;
}

static PyObject* PythonAddEventCallback(PyObject* self, PyObject* args)
{
	if(!s_context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	// Extract args as a (PyObject function, EventType enum)
	PyObject* pyFunc;
	int eventType;
	if(!PyArg_ParseTuple(args, "Oi", &pyFunc, &eventType))
		return nullptr;

	// Check if pyFunc is a function
	if(!PyCallable_Check(pyFunc)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be callable");
		return nullptr;
	}

	// Check if eventType is a valid EventType
	if(eventType < 0 || eventType >(int)EventType::LastValue) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be a valid EventType");
		return nullptr;
	}

	// Add the callback to the list of callbacks
	s_context->RegisterEventCallback(static_cast<EventType>(eventType), pyFunc);

	Py_RETURN_NONE;
}

static PyObject* PythonRemoveEventCallback(PyObject* self, PyObject* args)
{
	if(!s_context) {
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	// Extract args as a (PyObject function, EventType enum)
	PyObject* pyFunc;
	int eventType;
	if(!PyArg_ParseTuple(args, "Oi", &pyFunc, &eventType))
		return nullptr;

	// Check if pyFunc is a function
	if(!PyCallable_Check(pyFunc)) {
		PyErr_SetString(PyExc_TypeError, "First argument must be callable");
		return nullptr;
	}

	// Check if eventType is a valid EventType
	if(eventType < 0 || eventType >(int)EventType::LastValue) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be a valid EventType");
		return nullptr;
	}

	// Remove the callback from the list of callbacks
	s_context->UnregisterEventCallback(static_cast<EventType>(eventType), pyFunc);

	Py_RETURN_NONE;
}


void* PythonScriptingContext::RegisterFrameMemory(const std::vector<std::pair<uint16_t, MemoryType>>& addresses)
{
	if (addresses.empty())
		return nullptr;

	uint16_t *result = new uint16_t[addresses.size() * 2];
	memset(result, 0, addresses.size() * 2 * sizeof(uint16_t));

	MemoryRegistry reg = {};
	reg.BaseAddress = result;
	reg.Values = addresses;

	FillOneFrameMemory(reg);

	return result;
}

bool PythonScriptingContext::UnregisterFrameMemory(void* ptr)
{
	int count = _frameMemory.size();

	_frameMemory.erase(std::remove_if(_frameMemory.begin(), _frameMemory.end(), [ptr](const MemoryRegistry& entry) { return ptr == entry.BaseAddress; }), _frameMemory.end());

	if (count == _frameMemory.size())
		return false;

	delete[] ptr;
	return true;
}

void PythonScriptingContext::FillOneFrameMemory(MemoryRegistry& reg)
{
	auto ptr = reg.BaseAddress;
	for(auto& addr : reg.Values)
	{
		uint8_t value = _memoryDumper->GetMemoryValue(addr.second, addr.first);
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
	InitializePython();

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
	if (type == EventType::StartFrame)
		UpdateFrameMemory();

	int count = 0;
	for(PyObject* callback : _eventCallbacks[(int)type])
	{
 		PyObject* pFunc = callback;
		PyObject* pArgs = PyTuple_New(1);
		PyObject* pValue = PyLong_FromLong((long)cpuType);

		PyTuple_SetItem(pArgs, 0, pValue);

		PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
		if(pResult == NULL)
			LogError();
		else
			Py_DECREF(pResult);

		Py_DECREF(pArgs);
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