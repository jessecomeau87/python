// This file is generated by Tools/cases_generator/tier2_generator.py
// from:
//   Python/bytecodes.c
// Do not edit!

#include "Python.h"

#include "pycore_call.h"
#include "pycore_ceval.h"
#include "pycore_dict.h"
#include "pycore_emscripten_signal.h"
#include "pycore_executor_externals.h"
#include "pycore_intrinsics.h"
#include "pycore_long.h"
#include "pycore_opcode_metadata.h"
#include "pycore_opcode_utils.h"
#include "pycore_optimizer.h"
#include "pycore_range.h"
#include "pycore_setobject.h"
#include "pycore_sliceobject.h"
#include "pycore_descrobject.h"

#include "ceval_macros.h"

#define TIER_TWO 2

        PyObject ** _COPY_FREE_VARS_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer, int oparg) {
            /* Copy closure variables to free variables */
            PyCodeObject *co = _PyFrame_GetCode(frame);
            assert(PyFunction_Check(frame->f_funcobj));
            PyObject *closure = ((PyFunctionObject *)frame->f_funcobj)->func_closure;
            assert(oparg == co->co_nfreevars);
            int offset = co->co_nlocalsplus - oparg;
            for (int i = 0; i < oparg; ++i) {
                PyObject *o = PyTuple_GET_ITEM(closure, i);
                frame->localsplus[offset + i] = Py_NewRef(o);
            }
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_BOUND_METHOD_EXACT_ARGS_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer, int oparg) {
            PyObject *callable;
            PyObject *func;
            PyObject *self;
            callable = stack_pointer[-2 - oparg];
            STAT_INC(CALL, hit);
            self = Py_NewRef(((PyMethodObject *)callable)->im_self);
            stack_pointer[-1 - oparg] = self;  // Patch stack as it is used by _INIT_CALL_PY_EXACT_ARGS
            func = Py_NewRef(((PyMethodObject *)callable)->im_func);
            stack_pointer[-2 - oparg] = func;  // This is used by CALL, upon deoptimization
            Py_DECREF(callable);
            stack_pointer[-2 - oparg] = func;
            stack_pointer[-1 - oparg] = self;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_0_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer) {
            int oparg;
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            oparg = 0;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_1_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer) {
            int oparg;
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            oparg = 1;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_2_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer) {
            int oparg;
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            oparg = 2;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_3_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer) {
            int oparg;
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            oparg = 3;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_4_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer) {
            int oparg;
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            oparg = 4;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _INIT_CALL_PY_EXACT_ARGS_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer, int oparg) {
            PyObject **args;
            PyObject *self_or_null;
            PyObject *callable;
            _PyInterpreterFrame *new_frame;
            args = &stack_pointer[-oparg];
            self_or_null = stack_pointer[-1 - oparg];
            callable = stack_pointer[-2 - oparg];
            int has_self = (self_or_null != NULL);
            STAT_INC(CALL, hit);
            PyFunctionObject *func = (PyFunctionObject *)callable;
            new_frame = _PyFrame_PushUnchecked(tstate, func, oparg + has_self);
            PyObject **first_non_self_local = new_frame->localsplus + has_self;
            new_frame->localsplus[0] = self_or_null;
            for (int i = 0; i < oparg; i++) {
                first_non_self_local[i] = args[i];
            }
            stack_pointer[-2 - oparg] = (PyObject *)new_frame;
            stack_pointer += -1 - oparg;
            return stack_pointer;
        }

        PyObject ** _SET_FUNCTION_ATTRIBUTE_func(PyThreadState *tstate, _PyInterpreterFrame *frame, PyObject **stack_pointer, int oparg) {
            PyObject *func;
            PyObject *attr;
            func = stack_pointer[-1];
            attr = stack_pointer[-2];
            assert(PyFunction_Check(func));
            PyFunctionObject *func_obj = (PyFunctionObject *)func;
            switch(oparg) {
                case MAKE_FUNCTION_CLOSURE:
                assert(func_obj->func_closure == NULL);
                func_obj->func_closure = attr;
                break;
                case MAKE_FUNCTION_ANNOTATIONS:
                assert(func_obj->func_annotations == NULL);
                func_obj->func_annotations = attr;
                break;
                case MAKE_FUNCTION_KWDEFAULTS:
                assert(PyDict_CheckExact(attr));
                assert(func_obj->func_kwdefaults == NULL);
                func_obj->func_kwdefaults = attr;
                break;
                case MAKE_FUNCTION_DEFAULTS:
                assert(PyTuple_CheckExact(attr));
                assert(func_obj->func_defaults == NULL);
                func_obj->func_defaults = attr;
                break;
                default:
                Py_UNREACHABLE();
            }
            stack_pointer[-2] = func;
            stack_pointer += -1;
            return stack_pointer;
        }

