#ifndef Py_INTERNAL_EXCEPTIONS_H
#define Py_INTERNAL_EXCEPTIONS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif


/* runtime lifecycle */

extern PyStatus _PyExc_InitState(PyInterpreterState *);
extern PyStatus _PyExc_InitGlobalObjects(PyInterpreterState *);
extern PyStatus _PyExc_InitTypes(PyInterpreterState *);
extern void _PyExc_Fini(PyInterpreterState *);


/* other API */

extern void _PyExc_ClearExceptionGroupType(PyInterpreterState *);


#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_EXCEPTIONS_H */
