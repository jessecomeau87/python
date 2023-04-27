/* typing accelerator C extension: _typing module. */

#define Py_BUILD_CORE

#include "Python.h"
#include "internal/pycore_interp.h"
#include "clinic/_typingmodule.c.h"

/*[clinic input]
module _typing

[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=1db35baf1c72942b]*/

/* helper function to make typing.NewType.__call__ method faster */

/*[clinic input]
_typing._idfunc -> object

    x: object
    /

[clinic start generated code]*/

static PyObject *
_typing__idfunc(PyObject *module, PyObject *x)
/*[clinic end generated code: output=63c38be4a6ec5f2c input=49f17284b43de451]*/
{
    return Py_NewRef(x);
}


static PyMethodDef typing_methods[] = {
    _TYPING__IDFUNC_METHODDEF
    {NULL, NULL, 0, NULL}
};

PyDoc_STRVAR(typing_doc,
"Accelerators for the typing module.\n");

static int
_typing_exec(PyObject *m)
{
    PyInterpreterState *interp = PyInterpreterState_Get();

#define EXPORT_TYPE(name, typename) \
    if (PyModule_AddObjectRef(m, name, \
                              (PyObject *)interp->cached_objects.typename) < 0) { \
        return -1; \
    }

    EXPORT_TYPE("TypeVar", typevar_type);
    EXPORT_TYPE("TypeVarTuple", typevartuple_type);
    EXPORT_TYPE("ParamSpec", paramspec_type);
    EXPORT_TYPE("ParamSpecArgs", paramspecargs_type);
    EXPORT_TYPE("ParamSpecKwargs", paramspeckwargs_type);
    EXPORT_TYPE("TypeAliasType", typealias_type);
    EXPORT_TYPE("Generic", generic_type);
#undef EXPORT_TYPE
    return 0;
}

static struct PyModuleDef_Slot _typingmodule_slots[] = {
    {Py_mod_exec, _typing_exec},
    {0, NULL}
};

static struct PyModuleDef typingmodule = {
        PyModuleDef_HEAD_INIT,
        "_typing",
        typing_doc,
        0,
        typing_methods,
        _typingmodule_slots,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC
PyInit__typing(void)
{
    return PyModuleDef_Init(&typingmodule);
}
