#include <Python.h>


typedef PyObject * (*mapredfunc) (PyObject *);

typedef struct {
  PyObject_HEAD
  PyObject *data;
  mapredfunc func;
} mapredobject;

static PyTypeObject mapredtype;

PyObject *
mapred_new(PyObject *data, mapredfunc func) {
  mapredobject *mro;
  mro = PyObject_GC_New(mapredobject, &mapredtype);
  if (mro == NULL)
    return NULL;
  mro->data = PyObject_GetIter(data);
  mro->func = func;
  _PyObject_GC_TRACK(mro);
  return (PyObject *) mro;
}

static void
mapred_dealloc(mapredobject *mro) {
  _PyObject_GC_UNTRACK(mro);
  Py_XDECREF(mro->data);
  PyObject_GC_Del(mro);
}

static int
mapred_traverse(mapredobject *mro, visitproc visit, void *arg) {
  Py_VISIT(mro->data);
  return 0;
}

static PyObject *
mapred_iternext(mapredobject *mro) {
  PyObject *pair, *ret;
  pair = PyIter_Next(mro->data);
  if (pair == NULL)
    return NULL;
  ret = mro->func(pair);
  Py_DECREF(pair);
  if (ret == NULL)
    ret = mapred_iternext(mro);
  return ret;
}

static PyTypeObject mapredtype = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "mapred",                                /* tp_name */
  sizeof(mapredobject),                    /* tp_basicsize */
  0,                                       /* tp_itemsize */
  (destructor)mapred_dealloc,              /* tp_dealloc */
  0,                                       /* tp_print */
  0,                                       /* tp_getattr */
  0,                                       /* tp_setattr */
  0,                                       /* tp_compare */
  0,                                       /* tp_repr */
  0,                                       /* tp_as_number */
  0,                                       /* tp_as_sequence */
  0,                                       /* tp_as_mapping */
  0,                                       /* tp_hash */
  0,                                       /* tp_call */
  0,                                       /* tp_str */
  PyObject_GenericGetAttr,                 /* tp_getattro */
  0,                                       /* tp_setattro */
  0,                                       /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
  0,                                       /* tp_doc */
  (traverseproc)mapred_traverse,           /* tp_traverse */
  0,                                       /* tp_clear */
  0,                                       /* tp_richcompare */
  0,                                       /* tp_weaklistoffset */
  PyObject_SelfIter,                       /* tp_iter */
  (iternextfunc)mapred_iternext,           /* tp_iternext */
  0,                                       /* tp_methods */
};


static PyObject *
map(PyObject *pair) {
  PyObject *val, *ret;
  char *valstr;
  int valsize, len = 0;
  val = PySequence_GetItem(pair, 1);
  valstr = PyString_AS_STRING(val);
  valsize = PyString_GET_SIZE(val);
  while (valstr[len] != ' ' && valstr[len] != '\0') len++;
  if (len == valsize) {
    ret = NULL;
  } else {
    ret = Py_BuildValue("Ni", PyString_FromStringAndSize(valstr, len), 1);
  }
  Py_DECREF(val);
  return ret;
}

static PyObject *
reduce(PyObject *pair) {
  PyObject *values, *iterator, *item;
  long sum = 0;
  values = PySequence_GetItem(pair, 1);
  iterator = PyObject_GetIter(values);
  if (iterator == NULL)
    return 0;
  while ((item = PyIter_Next(iterator))) {
    sum += PyInt_AS_LONG(item);
    Py_DECREF(item);
  }
  Py_DECREF(iterator);
  Py_DECREF(values);
  return Py_BuildValue("Nl", PySequence_GetItem(pair, 0), sum);
}


static PyObject *
mapper(PyObject *self, PyObject *args) {
  PyObject *data;
  if (PyTuple_GET_SIZE(args) != 1)
    return NULL;
  data = PyTuple_GET_ITEM(args, 0);
  return mapred_new(data, map);
}

static PyObject *
reducer(PyObject *self, PyObject *args) {
  PyObject *data;
  if (PyTuple_GET_SIZE(args) != 1)
    return NULL;
  data = PyTuple_GET_ITEM(args, 0);
  return mapred_new(data, reduce);
}

static PyMethodDef CIPCountMethods[] = {
  {"mapper", mapper, METH_VARARGS, "C IP count mapper."},
  {"reducer", reducer, METH_VARARGS, "C IP count reducer."},
  {NULL, NULL, 0, NULL} /* sentinel */
};

PyMODINIT_FUNC
initcipcount_impl(void) {
  Py_InitModule("cipcount_impl", CIPCountMethods);
}
