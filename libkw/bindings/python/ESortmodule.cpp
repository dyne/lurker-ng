#include <Python.h>
#include <esort.h>

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

static PyMethodDef ESortMethods[] = {
	{"system",  spam_system, METH_VARARGS,
	 "Execute a shell command."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initspam(void)
{
	(void) Py_InitModule("esort", ESortMethods);
}
