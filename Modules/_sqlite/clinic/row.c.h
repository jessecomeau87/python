/*[clinic input]
preserve
[clinic start generated code]*/

#if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)
#  include "pycore_gc.h"            // PyGC_Head
#  include "pycore_runtime.h"       // _Py_ID()
#endif


static PyObject *
pysqlite_row_new_impl(PyTypeObject *type, pysqlite_Cursor *cursor,
                      PyObject *data);

static PyObject *
pysqlite_row_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyObject *return_value = NULL;
    PyTypeObject *base_tp = clinic_state()->RowType;
    pysqlite_Cursor *cursor;
    PyObject *data;

    if ((type == base_tp || type->tp_init == base_tp->tp_init) &&
        !_PyArg_NoKeywords("Row", kwargs)) {
        goto exit;
    }
    if (!_PyArg_CheckPositional("Row", PyTuple_GET_SIZE(args), 2, 2)) {
        goto exit;
    }
    PyTypeObject *argument_1_tp = clinic_state()->CursorType;
    if (!PyObject_TypeCheck(PyTuple_GET_ITEM(args, 0), argument_1_tp)) {
        _PyArg_BadArgument("Row", "argument 1",
                           argument_1_tp->tp_name,
                           PyTuple_GET_ITEM(args, 0));
        goto exit;
    }
    cursor = (pysqlite_Cursor *)PyTuple_GET_ITEM(args, 0);
    if (!PyTuple_Check(PyTuple_GET_ITEM(args, 1))) {
        _PyArg_BadArgument("Row", "argument 2", "tuple", PyTuple_GET_ITEM(args, 1));
        goto exit;
    }
    data = PyTuple_GET_ITEM(args, 1);
    return_value = pysqlite_row_new_impl(type, cursor, data);

exit:
    return return_value;
}

PyDoc_STRVAR(pysqlite_row_keys__doc__,
"keys($self, /)\n"
"--\n"
"\n"
"Returns the keys of the row.");

#define PYSQLITE_ROW_KEYS_METHODDEF    \
    {"keys", (PyCFunction)pysqlite_row_keys, METH_NOARGS, pysqlite_row_keys__doc__},

static PyObject *
pysqlite_row_keys_impl(pysqlite_Row *self);

static PyObject *
pysqlite_row_keys(pysqlite_Row *self, PyObject *Py_UNUSED(ignored))
{
    return pysqlite_row_keys_impl(self);
}
/*[clinic end generated code: output=62db2e4c2c44d2bd input=a9049054013a1b77]*/
