// Implementation of PEP 585: support list[int] etc.
#ifndef Py_GENERICALIASOBJECT_H
#define Py_GENERICALIASOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_LIMITED_API
PyAPI_FUNC(PyObject *) _Py_apply_parameters(PyObject *, PyObject *, PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) _Py_make_parameters(PyObject *);
#endif

PyAPI_FUNC(PyObject *) Py_GenericAlias(PyObject *, PyObject *);
PyAPI_DATA(PyTypeObject) Py_GenericAliasType;

#ifdef __cplusplus
}
#endif
#endif /* !Py_GENERICALIASOBJECT_H */
