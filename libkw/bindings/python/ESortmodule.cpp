#include <Python.h>
#include <esort.h>

struct ESort_WalkerObject
{
    PyObject_HEAD
    ESort::Walker* walker;
};

static PyTypeObject ESort_WalkerType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "ESort.Walker",            /*tp_name*/
    sizeof(ESort_WalkerObject),/*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Walker objects",          /* tp_doc */
};

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
	char *command;
	int sts;
	
	if (!PyArg_ParseTuple(args, "s", &command))
		return NULL;
	sts = system(command);
	return Py_BuildValue("i", sts);
}

static PyMethodDef ESort_methods[] = {
	{"system",  spam_system, METH_VARARGS,
	 "Execute a shell command."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initESort(void)
{
	PyObject* m;
	
	ESort_WalkerType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&ESort_WalkerType) < 0) return;
	
	m = Py_InitModule3("ESort", ESort_methods,
		"Module gateway for C++ ESort library.");
		
	Py_INCREF(&ESort_WalkerType);
	PyModule_AddObject(m, "Walker", (PyObject *)&ESort_WalkerType);
}
