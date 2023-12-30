#include "pch.h"
#include "PythonScriptingContext.h"
#include "PythonApi.h"
#include "Utilities/FolderUtilities.h"

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
	{"addEventCallback", PythonAddEventCallback, METH_VARARGS, "Adds an event callback.  e.g. emu.addEventCallback(function, eventType.startFrame)"},
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

static PyObject* PythonEmuLog(PyObject* self, PyObject* args)
{
	PythonScriptingContext *context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context)
	{
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	// Extract the only argument as an object, convert to a string
	PyObject* pyMessage;
	if(!PyArg_ParseTuple(args, "O", &pyMessage))
	{
		PyErr_SetString(PyExc_TypeError, "Failed to parse arguments");
		return nullptr;
	}

	PyObject* pyStr = PyObject_Str(pyMessage);
	if(!pyStr)
	{
		PyErr_SetString(PyExc_TypeError, "Failed to convert argument to string");
		return nullptr;
	}

	string message = PyUnicode_AsUTF8(pyStr);
	context->Log(message);

	Py_RETURN_NONE;
}

static PyObject* PythonRead8(PyObject* self, PyObject* args)
{
	PythonScriptingContext* context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context) {
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	int address, kind;
	PyObject* py_signed;

	// Unpack the Python arguments. Expected: two integers and a boolean
	if(!PyArg_ParseTuple(args, "iiO", &address, &kind, &py_signed))
		return nullptr;

	// Check if py_signed is a boolean
	if(!PyBool_Check(py_signed)) {
		PyErr_SetString(PyExc_TypeError, "Third argument must be a boolean");
		return nullptr;
	}

	// Convert the PyObject to a C++ boolean
	bool sgn = PyObject_IsTrue(py_signed);

	uint8_t result = 0;
	bool success = context->ReadMemory(address, static_cast<MemoryType>(kind), sgn, result);

	// Convert the result to a Python object based on is_signed
	PyObject* pyResult = nullptr;
	if(sgn)
		pyResult = PyLong_FromLong(static_cast<int8_t>(result));
	else
		pyResult = PyLong_FromUnsignedLong(result);

	return pyResult;
}

static PyObject* PythonRegisterFrameMemory(PyObject* self, PyObject* args)
{
	PythonScriptingContext* context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context) {
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}

	std::vector<int> result;

	// Extract args as a (MemoryType enum, list of addresses)
	int memType = 0;
	PyObject* pyAddresses = nullptr;
	if(!PyArg_ParseTuple(args, "iO", &memType, &pyAddresses))
		return nullptr;

	// extract the list of addresses
	PyObject* pyIter = PyObject_GetIter(pyAddresses);
	if(!pyIter) {
		PyErr_SetString(PyExc_TypeError, "Second argument must be iterable");
		return nullptr;
	}

	PyObject* pyItem;
	while((pyItem = PyIter_Next(pyIter))) {
		if(!PyLong_Check(pyItem)) {
			PyErr_SetString(PyExc_TypeError, "Second argument must be a list of integers");
			return nullptr;
		}

		int addr = PyLong_AsLong(pyItem);
		result.push_back(addr);
	}

	void* ptr = context->RegisterFrameMemory(static_cast<MemoryType>(memType), result);
	if(ptr == nullptr)
		Py_RETURN_NONE;

	return PyLong_FromVoidPtr(ptr);
}

static PyObject* PythonUnregisterFrameMemory(PyObject* self, PyObject* args)
{
	PythonScriptingContext* context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context) {
		PyErr_SetString(PyExc_TypeError, "No registered python context.");
		return nullptr;
	}


	void* ptr;
	if(!PyArg_ParseTuple(args, "p", &ptr))
		return nullptr;

	bool result = context->UnregisterFrameMemory(ptr);
	if(result)
		Py_RETURN_TRUE;

	Py_RETURN_FALSE;
}

static PyObject* PythonAddEventCallback(PyObject* self, PyObject* args)
{
	PythonScriptingContext* context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context) {
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
	context->RegisterEventCallback(static_cast<EventType>(eventType), pyFunc);

	Py_RETURN_NONE;
}

static PyObject* PythonRemoveEventCallback(PyObject* self, PyObject* args)
{
	PythonScriptingContext* context = GetScriptingContextFromThreadState(PyThreadState_Get());
	if(!context) {
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
	context->UnregisterEventCallback(static_cast<EventType>(eventType), pyFunc);

	Py_RETURN_NONE;
}


static string ReadFileContents(const string& path)
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

std::unordered_map<PyThreadState*, PythonScriptingContext*> s_pythonContexts;

PyThreadState *InitializePython(PythonScriptingContext *context)
{
	if(!Py_IsInitialized())
	{
		PyImport_AppendInittab("emu", PyInit_mesen);
		Py_Initialize();
	}

	PyThreadState* curr = Py_NewInterpreter();
	s_pythonContexts[curr] = context;

	PyThreadState* old = PyThreadState_Swap(curr);

	string startupPath = FolderUtilities::GetHomeFolder();
	startupPath += "\\";
	startupPath += "PythonStartup.py";
	string startupScript = ReadFileContents(startupPath);

	bool result = !PyRun_SimpleString(startupScript.c_str());
	if(!result)
	{
		PyErr_Print();
		Py_EndInterpreter(curr);
		curr = nullptr;
	}

	return curr;
}

void ReportEndScriptingContext(PythonScriptingContext* ctx)
{
	for(auto it = s_pythonContexts.begin(); it != s_pythonContexts.end(); ++it)
	{
		if(it->second == ctx)
		{
			s_pythonContexts.erase(it);
			break;
		}
	}
}

PythonScriptingContext* GetScriptingContextFromThreadState(PyThreadState* state)
{
	auto it = s_pythonContexts.find(state);
	if(it != s_pythonContexts.end())
		return it->second;

	return nullptr;
}