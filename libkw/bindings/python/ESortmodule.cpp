#include <Python.h>
#include <esort.h>

static PyObject* ESortError;

struct Walker
{
    PyObject_HEAD
    ESort::Walker* walker;
};

static void Walker_dealloc(Walker* self)
{
	if (self->walker)
	{
		delete self->walker;
		self->walker = 0;
	}
}

// can use default new since it zeros members

static int Walker_init(Walker* self, PyObject* args, PyObject* kwds)
{
	PyErr_SetString(ESortError, "Cannot create Walker objects directly");
	return -1;
}

static PyObject* Walker_advance(Walker* self, PyObject* args, PyObject* kwds)
{
	if (!PyArg_ParseTuple(args, "")) return NULL;
	
	int out = self->walker->advance();
	if (out != -1 || errno == 0)
		return Py_BuildValue("i", out);
	
	PyErr_SetFromErrno(ESortError);
	return NULL;
}

static PyMethodDef Walker_methods[] = {
	{"advance", (PyCFunction)Walker_advance, METH_NOARGS,
	 "Move the walker to the next record"
	},
	{NULL}  /* Sentinel */
};

static PyTypeObject WalkerType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "ESort.Walker",            /*tp_name*/
    sizeof(Walker),            /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Walker_dealloc,/*tp_dealloc*/
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
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Walker_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Walker_init,     /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

struct Reader
{
    PyObject_HEAD
    ESort::Reader* reader;
};

static void Reader_dealloc(Reader* self)
{
	if (self->reader)
	{
		delete self->reader;
		self->reader = 0;
	}
}

// can use default new since it zeros members

static int Reader_init(Reader* self, PyObject* args, PyObject* kwds)
{
	static char *kwlist[] = { "database", "synced", "unique", "blockSize", "keySize", NULL};
	
	char* database;
	int synced = 1, unique = 1;
	unsigned long blockSize = 8192, keySize = 255;
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|iill", kwlist, 
		&database, &synced, &unique, &blockSize, &keySize))
		return -1; 
	
	self->reader = ESort::Reader::opendb(database,
		ESort::Parameters(synced, unique, blockSize, keySize)).release();
	if (self->reader == 0)
	{
		PyErr_SetFromErrno(ESortError);
		return -1;
	}
	
	return 0;
}

static PyObject* Reader_seek(Reader* self, PyObject* args, PyObject* kwds)
{
	static char *kwlist[] = { "key", "direction", NULL};
	
	char* key;
	int   keylen;
	int   direction = 1;
	
 	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|i", kwlist, 
		&key, &keylen, &direction))
		return NULL;
	
	if (direction != 1 && direction != 2)
	{
		PyErr_SetString(ESortError, "Direction must be ESort.FORWARD or ESort.BACKWARD");
		return NULL;
	}
	
	ESort::Direction dir = (ESort::Direction)direction;
	
	ESort::auto_ptr<ESort::Walker> walker(self->reader->seek(
		ESort::string(key, keylen), dir));
	if (!walker.get())
	{
		PyErr_SetFromErrno(ESortError);
		return NULL;
	}
	
	Walker* w = PyObject_New(Walker, &WalkerType);
	if (!w) return NULL;
	
	w->walker = walker.release();
	return (PyObject*)w;
}

static PyObject* Reader_seekp(Reader* self, PyObject* args, PyObject* kwds)
{
	static char *kwlist[] = { "prefix", "key", "direction", NULL};
	
	char* prefix;
	int   prefixlen;
	char* key;
	int   keylen;
	int   direction = 1;
	
 	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#s#|i", kwlist, 
		&prefix, &prefixlen, &key, &keylen, &direction))
		return NULL;
	
	if (direction != 1 && direction != 2)
	{
		PyErr_SetString(ESortError, "Direction must be ESort.FORWARD or ESort.BACKWARD");
		return NULL;
	}
	
	ESort::Direction dir = (ESort::Direction)direction;
	
	ESort::auto_ptr<ESort::Walker> walker(self->reader->seekp(
		ESort::string(prefix, prefixlen), ESort::string(key, keylen), dir));
	if (!walker.get())
	{
		PyErr_SetFromErrno(ESortError);
		return NULL;
	}
	
	Walker* w = PyObject_New(Walker, &WalkerType);
	if (!w) return NULL;
	
	w->walker = walker.release();
	return (PyObject*)w;
}

static PyMethodDef Reader_methods[] = {
	{"seek", (PyCFunction)Reader_seek, METH_NOARGS,
	 "Seek to a location in the database and obtain a walker"
	},
	{"seekp", (PyCFunction)Reader_seekp, METH_NOARGS,
	 "Seek to a location restricted within prefix*"
	},
	{NULL}  /* Sentinel */
};

static PyTypeObject ReaderType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "ESort.Reader",            /*tp_name*/
    sizeof(Reader),            /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Reader_dealloc,/*tp_dealloc*/
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
    "Reader objects",          /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Reader_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Reader_init,     /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static PyMethodDef ESort_methods[] = {
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initESort(void)
{
	PyObject* m;
	
	ESortError = PyErr_NewException("ESort.error", NULL, NULL);
	if (ESortError == 0) return;
	if (PyType_Ready(&WalkerType) < 0) return;
	if (PyType_Ready(&ReaderType) < 0) return;
	
	m = Py_InitModule3("ESort", ESort_methods,
		"Module gateway for C++ ESort library.");
	
	Py_INCREF(ESortError);
	PyModule_AddObject(m, "error", ESortError);		
	Py_INCREF(&WalkerType);
	PyModule_AddObject(m, "Walker", (PyObject *)&WalkerType);
	Py_INCREF(&ReaderType);
	PyModule_AddObject(m, "Reader", (PyObject *)&ReaderType);
}
