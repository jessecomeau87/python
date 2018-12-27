/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(sys_displayhook__doc__,
"displayhook($module, object, /)\n"
"--\n"
"\n"
"Print an object to sys.stdout and also save it in builtins._");

#define SYS_DISPLAYHOOK_METHODDEF    \
    {"displayhook", (PyCFunction)sys_displayhook, METH_O, sys_displayhook__doc__},

PyDoc_STRVAR(sys_excepthook__doc__,
"excepthook($module, exctype, value, traceback, /)\n"
"--\n"
"\n"
"Handle an exception by displaying it with a traceback on sys.stderr.");

#define SYS_EXCEPTHOOK_METHODDEF    \
    {"excepthook", (PyCFunction)(void(*)(void))sys_excepthook, METH_FASTCALL, sys_excepthook__doc__},

static PyObject *
sys_excepthook_impl(PyObject *module, PyObject *exctype, PyObject *value,
                    PyObject *traceback);

static PyObject *
sys_excepthook(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *exctype;
    PyObject *value;
    PyObject *traceback;

    if (!_PyArg_UnpackStack(args, nargs, "excepthook",
        3, 3,
        &exctype, &value, &traceback)) {
        goto exit;
    }
    return_value = sys_excepthook_impl(module, exctype, value, traceback);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_exc_info__doc__,
"exc_info($module, /)\n"
"--\n"
"\n"
"Return current exception information.\n"
"\n"
"Return information about the most recent exception caught by an except clause in the current stack frame or in an older stack frame.");

#define SYS_EXC_INFO_METHODDEF    \
    {"exc_info", (PyCFunction)sys_exc_info, METH_NOARGS, sys_exc_info__doc__},

static PyObject *
sys_exc_info_impl(PyObject *module);

static PyObject *
sys_exc_info(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_exc_info_impl(module);
}

PyDoc_STRVAR(sys_exit__doc__,
"exit($module, status=None, /)\n"
"--\n"
"\n"
"Exit the interpreter by raising SystemExit(status).\n"
"\n"
"If the status is omitted or None, it defaults to zero (i.e., success).\n"
"If the status is an integer, it will be used as the system exit status.\n"
"If it is another kind of object, it will be printed and the system\n"
"exit status will be one (i.e., failure).");

#define SYS_EXIT_METHODDEF    \
    {"exit", (PyCFunction)(void(*)(void))sys_exit, METH_FASTCALL, sys_exit__doc__},

static PyObject *
sys_exit_impl(PyObject *module, PyObject *status);

static PyObject *
sys_exit(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *status = NULL;

    if (!_PyArg_UnpackStack(args, nargs, "exit",
        0, 1,
        &status)) {
        goto exit;
    }
    return_value = sys_exit_impl(module, status);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_getdefaultencoding__doc__,
"getdefaultencoding($module, /)\n"
"--\n"
"\n"
"Return the current default string encoding used by the Unicode implementation.");

#define SYS_GETDEFAULTENCODING_METHODDEF    \
    {"getdefaultencoding", (PyCFunction)sys_getdefaultencoding, METH_NOARGS, sys_getdefaultencoding__doc__},

static PyObject *
sys_getdefaultencoding_impl(PyObject *module);

static PyObject *
sys_getdefaultencoding(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getdefaultencoding_impl(module);
}

PyDoc_STRVAR(sys_getfilesystemencoding__doc__,
"getfilesystemencoding($module, /)\n"
"--\n"
"\n"
"Return the encoding used to convert Unicode filenames in operating system filenames.");

#define SYS_GETFILESYSTEMENCODING_METHODDEF    \
    {"getfilesystemencoding", (PyCFunction)sys_getfilesystemencoding, METH_NOARGS, sys_getfilesystemencoding__doc__},

static PyObject *
sys_getfilesystemencoding_impl(PyObject *module);

static PyObject *
sys_getfilesystemencoding(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getfilesystemencoding_impl(module);
}

PyDoc_STRVAR(sys_getfilesystemencodeerrors__doc__,
"getfilesystemencodeerrors($module, /)\n"
"--\n"
"\n"
"Return the error mode used to convert Unicode filenames in operating system filenames.");

#define SYS_GETFILESYSTEMENCODEERRORS_METHODDEF    \
    {"getfilesystemencodeerrors", (PyCFunction)sys_getfilesystemencodeerrors, METH_NOARGS, sys_getfilesystemencodeerrors__doc__},

static PyObject *
sys_getfilesystemencodeerrors_impl(PyObject *module);

static PyObject *
sys_getfilesystemencodeerrors(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getfilesystemencodeerrors_impl(module);
}

PyDoc_STRVAR(sys_intern__doc__,
"intern($module, string, /)\n"
"--\n"
"\n"
"``Intern\'\' the given string.\n"
"\n"
"This enters the string in the (global) table of interned strings whose purpose\n"
"is to speed up dictionary lookups. Return the string itself or the previously\n"
"interned string object with the same value.");

#define SYS_INTERN_METHODDEF    \
    {"intern", (PyCFunction)sys_intern, METH_O, sys_intern__doc__},

static PyObject *
sys_intern_impl(PyObject *module, PyObject *s);

static PyObject *
sys_intern(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    PyObject *s;

    if (!PyUnicode_Check(arg)) {
        _PyArg_BadArgument("intern", "str", arg);
        goto exit;
    }
    if (PyUnicode_READY(arg) == -1) {
        goto exit;
    }
    s = arg;
    return_value = sys_intern_impl(module, s);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_gettrace__doc__,
"gettrace($module, /)\n"
"--\n"
"\n"
"Return the global debug tracing function set with sys.settrace.\n"
"\n"
"See the debugger chapter in the library manual.");

#define SYS_GETTRACE_METHODDEF    \
    {"gettrace", (PyCFunction)sys_gettrace, METH_NOARGS, sys_gettrace__doc__},

static PyObject *
sys_gettrace_impl(PyObject *module);

static PyObject *
sys_gettrace(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_gettrace_impl(module);
}

PyDoc_STRVAR(sys_getprofile__doc__,
"getprofile($module, /)\n"
"--\n"
"\n"
"Return the profiling function set with sys.setprofile.\n"
"\n"
"See the profiler chapter in the library manual.");

#define SYS_GETPROFILE_METHODDEF    \
    {"getprofile", (PyCFunction)sys_getprofile, METH_NOARGS, sys_getprofile__doc__},

static PyObject *
sys_getprofile_impl(PyObject *module);

static PyObject *
sys_getprofile(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getprofile_impl(module);
}

PyDoc_STRVAR(sys_setcheckinterval__doc__,
"setcheckinterval($module, n, /)\n"
"--\n"
"\n"
"Tell the Python interpreter to check for asynchronous events every n instructions.\n"
"\n"
"This also affects how often thread switches occur.");

#define SYS_SETCHECKINTERVAL_METHODDEF    \
    {"setcheckinterval", (PyCFunction)sys_setcheckinterval, METH_O, sys_setcheckinterval__doc__},

static PyObject *
sys_setcheckinterval_impl(PyObject *module, int n);

static PyObject *
sys_setcheckinterval(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int n;

    if (PyFloat_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,
                        "integer argument expected, got float" );
        goto exit;
    }
    n = _PyLong_AsInt(arg);
    if (n == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_setcheckinterval_impl(module, n);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_getcheckinterval__doc__,
"getcheckinterval($module, /)\n"
"--\n"
"\n"
"Return the current check interval; see sys.setcheckinterval().");

#define SYS_GETCHECKINTERVAL_METHODDEF    \
    {"getcheckinterval", (PyCFunction)sys_getcheckinterval, METH_NOARGS, sys_getcheckinterval__doc__},

static PyObject *
sys_getcheckinterval_impl(PyObject *module);

static PyObject *
sys_getcheckinterval(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getcheckinterval_impl(module);
}

PyDoc_STRVAR(sys_setswitchinterval__doc__,
"setswitchinterval($module, interval, /)\n"
"--\n"
"\n"
"Set the ideal thread switching delay inside the Python interpreter.\n"
"\n"
"The actual frequency of switching threads can be lower if the\n"
"interpreter executes long sequences of uninterruptible code\n"
"(this is implementation-specific and workload-dependent).\n"
"\n"
"The parameter must represent the desired switching delay in seconds\n"
"A typical value is 0.005 (5 milliseconds).");

#define SYS_SETSWITCHINTERVAL_METHODDEF    \
    {"setswitchinterval", (PyCFunction)sys_setswitchinterval, METH_O, sys_setswitchinterval__doc__},

static PyObject *
sys_setswitchinterval_impl(PyObject *module, double interval);

static PyObject *
sys_setswitchinterval(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    double interval;

    interval = PyFloat_AsDouble(arg);
    if (PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_setswitchinterval_impl(module, interval);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_getswitchinterval__doc__,
"getswitchinterval($module, /)\n"
"--\n"
"\n"
"Return the current thread switch interval; see sys.setswitchinterval().");

#define SYS_GETSWITCHINTERVAL_METHODDEF    \
    {"getswitchinterval", (PyCFunction)sys_getswitchinterval, METH_NOARGS, sys_getswitchinterval__doc__},

static double
sys_getswitchinterval_impl(PyObject *module);

static PyObject *
sys_getswitchinterval(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    PyObject *return_value = NULL;
    double _return_value;

    _return_value = sys_getswitchinterval_impl(module);
    if ((_return_value == -1.0) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyFloat_FromDouble(_return_value);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_setrecursionlimit__doc__,
"setrecursionlimit($module, limit, /)\n"
"--\n"
"\n"
"Set the maximum depth of the Python interpreter stack to n.\n"
"\n"
"This limit prevents infinite recursion from causing an overflow of the C\n"
"stack and crashing Python.  The highest possible limit is platform-\n"
"dependent.");

#define SYS_SETRECURSIONLIMIT_METHODDEF    \
    {"setrecursionlimit", (PyCFunction)sys_setrecursionlimit, METH_O, sys_setrecursionlimit__doc__},

static PyObject *
sys_setrecursionlimit_impl(PyObject *module, int new_limit);

static PyObject *
sys_setrecursionlimit(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int new_limit;

    if (PyFloat_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,
                        "integer argument expected, got float" );
        goto exit;
    }
    new_limit = _PyLong_AsInt(arg);
    if (new_limit == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_setrecursionlimit_impl(module, new_limit);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_set_coroutine_origin_tracking_depth__doc__,
"set_coroutine_origin_tracking_depth($module, /, depth)\n"
"--\n"
"\n"
"Enable or disable origin tracking for coroutine objects in this thread.\n"
"\n"
"Coroutine objects will track \'depth\' frames of traceback information about\n"
"where they came from, available in their cr_origin attribute. Set depth of 0\n"
"to disable.");

#define SYS_SET_COROUTINE_ORIGIN_TRACKING_DEPTH_METHODDEF    \
    {"set_coroutine_origin_tracking_depth", (PyCFunction)(void(*)(void))sys_set_coroutine_origin_tracking_depth, METH_FASTCALL|METH_KEYWORDS, sys_set_coroutine_origin_tracking_depth__doc__},

static PyObject *
sys_set_coroutine_origin_tracking_depth_impl(PyObject *module, int depth);

static PyObject *
sys_set_coroutine_origin_tracking_depth(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"depth", NULL};
    static _PyArg_Parser _parser = {"i:set_coroutine_origin_tracking_depth", _keywords, 0};
    int depth;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &depth)) {
        goto exit;
    }
    return_value = sys_set_coroutine_origin_tracking_depth_impl(module, depth);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_get_coroutine_origin_tracking_depth__doc__,
"get_coroutine_origin_tracking_depth($module, /)\n"
"--\n"
"\n"
"Check status of origin tracking for coroutine objects in this thread.");

#define SYS_GET_COROUTINE_ORIGIN_TRACKING_DEPTH_METHODDEF    \
    {"get_coroutine_origin_tracking_depth", (PyCFunction)sys_get_coroutine_origin_tracking_depth, METH_NOARGS, sys_get_coroutine_origin_tracking_depth__doc__},

static int
sys_get_coroutine_origin_tracking_depth_impl(PyObject *module);

static PyObject *
sys_get_coroutine_origin_tracking_depth(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    PyObject *return_value = NULL;
    int _return_value;

    _return_value = sys_get_coroutine_origin_tracking_depth_impl(module);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromLong((long)_return_value);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_set_coroutine_wrapper__doc__,
"set_coroutine_wrapper($module, wrapper, /)\n"
"--\n"
"\n"
"Set a wrapper for coroutine objects.");

#define SYS_SET_COROUTINE_WRAPPER_METHODDEF    \
    {"set_coroutine_wrapper", (PyCFunction)sys_set_coroutine_wrapper, METH_O, sys_set_coroutine_wrapper__doc__},

PyDoc_STRVAR(sys_get_coroutine_wrapper__doc__,
"get_coroutine_wrapper($module, /)\n"
"--\n"
"\n"
"Return the wrapper for coroutine objects set by sys.set_coroutine_wrapper.");

#define SYS_GET_COROUTINE_WRAPPER_METHODDEF    \
    {"get_coroutine_wrapper", (PyCFunction)sys_get_coroutine_wrapper, METH_NOARGS, sys_get_coroutine_wrapper__doc__},

static PyObject *
sys_get_coroutine_wrapper_impl(PyObject *module);

static PyObject *
sys_get_coroutine_wrapper(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_get_coroutine_wrapper_impl(module);
}

PyDoc_STRVAR(sys_getrecursionlimit__doc__,
"getrecursionlimit($module, /)\n"
"--\n"
"\n"
"Return the current value of the recursion limit.\n"
"\n"
"The recursion limit is the maximum depth of the Python interpreter stack.\n"
"This limit prevents infinite recursion from causing an overflow of the C\n"
"stack and crashing Python.");

#define SYS_GETRECURSIONLIMIT_METHODDEF    \
    {"getrecursionlimit", (PyCFunction)sys_getrecursionlimit, METH_NOARGS, sys_getrecursionlimit__doc__},

static PyObject *
sys_getrecursionlimit_impl(PyObject *module);

static PyObject *
sys_getrecursionlimit(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getrecursionlimit_impl(module);
}

#if defined(MS_WINDOWS)

PyDoc_STRVAR(sys_getwindowsversion__doc__,
"getwindowsversion($module, /)\n"
"--\n"
"\n"
"Return information about the running version of Windows as a named tuple.\n"
"\n"
"The members are named: major, minor, build, platform, service_pack,\n"
"service_pack_major, service_pack_minor, suite_mask, product_type and\n"
"platform_version. For backward compatibility, only the first 5 items are\n"
"available by indexing. All elements are numbers, except service_pack and\n"
"platform_type which are strings, and platform_version which is a 3-tuple.\n"
"Platform is always 2. Product_type may be 1 for a workstation, 2 for a\n"
"domain controller, 3 for a server. Platform_version is a 3-tuple containing\n"
"a version number that is intended for identifying the OS rather than\n"
"feature detection.");

#define SYS_GETWINDOWSVERSION_METHODDEF    \
    {"getwindowsversion", (PyCFunction)sys_getwindowsversion, METH_NOARGS, sys_getwindowsversion__doc__},

static PyObject *
sys_getwindowsversion_impl(PyObject *module);

static PyObject *
sys_getwindowsversion(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getwindowsversion_impl(module);
}

#endif /* defined(MS_WINDOWS) */

#if defined(HAVE_DLOPEN)

PyDoc_STRVAR(sys_setdlopenflags__doc__,
"setdlopenflags($module, flags, /)\n"
"--\n"
"\n"
"Set the flags used by the interpreter for dlopen calls.\n"
"\n"
"This is used, for example, when the interpreter loads extension modules.\n"
"Among other things, this will enable a lazy resolving of symbols when\n"
"importing a module, if called as sys.setdlopenflags(0).  To share symbols\n"
"across extension modules, call as sys.setdlopenflags(os.RTLD_GLOBAL).\n"
"Symbolic names for the flag modules can be found in the os module\n"
"(RTLD_xxx constants, e.g. os.RTLD_LAZY).");

#define SYS_SETDLOPENFLAGS_METHODDEF    \
    {"setdlopenflags", (PyCFunction)sys_setdlopenflags, METH_O, sys_setdlopenflags__doc__},

static PyObject *
sys_setdlopenflags_impl(PyObject *module, int new_val);

static PyObject *
sys_setdlopenflags(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int new_val;

    if (PyFloat_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,
                        "integer argument expected, got float" );
        goto exit;
    }
    new_val = _PyLong_AsInt(arg);
    if (new_val == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_setdlopenflags_impl(module, new_val);

exit:
    return return_value;
}

#endif /* defined(HAVE_DLOPEN) */

#if defined(HAVE_DLOPEN)

PyDoc_STRVAR(sys_getdlopenflags__doc__,
"getdlopenflags($module, /)\n"
"--\n"
"\n"
"Return the current value of the flags that are used for dlopen calls.\n"
"\n"
"The flag constants are defined in the os module.");

#define SYS_GETDLOPENFLAGS_METHODDEF    \
    {"getdlopenflags", (PyCFunction)sys_getdlopenflags, METH_NOARGS, sys_getdlopenflags__doc__},

static PyObject *
sys_getdlopenflags_impl(PyObject *module);

static PyObject *
sys_getdlopenflags(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getdlopenflags_impl(module);
}

#endif /* defined(HAVE_DLOPEN) */

#if defined(USE_MALLOPT)

PyDoc_STRVAR(sys_mdebug__doc__,
"mdebug($module, flag, /)\n"
"--\n"
"\n");

#define SYS_MDEBUG_METHODDEF    \
    {"mdebug", (PyCFunction)sys_mdebug, METH_O, sys_mdebug__doc__},

static PyObject *
sys_mdebug_impl(PyObject *module, int flag);

static PyObject *
sys_mdebug(PyObject *module, PyObject *arg)
{
    PyObject *return_value = NULL;
    int flag;

    if (PyFloat_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,
                        "integer argument expected, got float" );
        goto exit;
    }
    flag = _PyLong_AsInt(arg);
    if (flag == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_mdebug_impl(module, flag);

exit:
    return return_value;
}

#endif /* defined(USE_MALLOPT) */

PyDoc_STRVAR(sys_getsizeof__doc__,
"getsizeof($module, /, object, default=None)\n"
"--\n"
"\n"
"Return the size of object in bytes.");

#define SYS_GETSIZEOF_METHODDEF    \
    {"getsizeof", (PyCFunction)(void(*)(void))sys_getsizeof, METH_FASTCALL|METH_KEYWORDS, sys_getsizeof__doc__},

static PyObject *
sys_getsizeof_impl(PyObject *module, PyObject *object, PyObject *dflt);

static PyObject *
sys_getsizeof(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    static const char * const _keywords[] = {"object", "default", NULL};
    static _PyArg_Parser _parser = {"O|O:getsizeof", _keywords, 0};
    PyObject *object;
    PyObject *dflt = NULL;

    if (!_PyArg_ParseStackAndKeywords(args, nargs, kwnames, &_parser,
        &object, &dflt)) {
        goto exit;
    }
    return_value = sys_getsizeof_impl(module, object, dflt);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_getrefcount__doc__,
"getrefcount($module, object, /)\n"
"--\n"
"\n"
"Return the reference count of object.\n"
"\n"
"The count returned is generally one higher than you might expect,\n"
"because it includes the (temporary) reference as an argument to\n"
"getrefcount().");

#define SYS_GETREFCOUNT_METHODDEF    \
    {"getrefcount", (PyCFunction)sys_getrefcount, METH_O, sys_getrefcount__doc__},

static Py_ssize_t
sys_getrefcount_impl(PyObject *module, PyObject *object);

static PyObject *
sys_getrefcount(PyObject *module, PyObject *object)
{
    PyObject *return_value = NULL;
    Py_ssize_t _return_value;

    _return_value = sys_getrefcount_impl(module, object);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}

#if defined(Py_REF_DEBUG)

PyDoc_STRVAR(sys_gettotalrefcount__doc__,
"gettotalrefcount($module, /)\n"
"--\n"
"\n");

#define SYS_GETTOTALREFCOUNT_METHODDEF    \
    {"gettotalrefcount", (PyCFunction)sys_gettotalrefcount, METH_NOARGS, sys_gettotalrefcount__doc__},

static Py_ssize_t
sys_gettotalrefcount_impl(PyObject *module);

static PyObject *
sys_gettotalrefcount(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    PyObject *return_value = NULL;
    Py_ssize_t _return_value;

    _return_value = sys_gettotalrefcount_impl(module);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}

#endif /* defined(Py_REF_DEBUG) */

PyDoc_STRVAR(sys_getallocatedblocks__doc__,
"getallocatedblocks($module, /)\n"
"--\n"
"\n"
"Return the number of memory blocks currently allocated, regardless of their size.");

#define SYS_GETALLOCATEDBLOCKS_METHODDEF    \
    {"getallocatedblocks", (PyCFunction)sys_getallocatedblocks, METH_NOARGS, sys_getallocatedblocks__doc__},

static Py_ssize_t
sys_getallocatedblocks_impl(PyObject *module);

static PyObject *
sys_getallocatedblocks(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    PyObject *return_value = NULL;
    Py_ssize_t _return_value;

    _return_value = sys_getallocatedblocks_impl(module);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}

#if defined(COUNT_ALLOCS)

PyDoc_STRVAR(sys_getcounts__doc__,
"getcounts($module, /)\n"
"--\n"
"\n");

#define SYS_GETCOUNTS_METHODDEF    \
    {"getcounts", (PyCFunction)sys_getcounts, METH_NOARGS, sys_getcounts__doc__},

static PyObject *
sys_getcounts_impl(PyObject *module);

static PyObject *
sys_getcounts(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_getcounts_impl(module);
}

#endif /* defined(COUNT_ALLOCS) */

PyDoc_STRVAR(sys__getframe__doc__,
"_getframe($module, depth=0, /)\n"
"--\n"
"\n"
"Return a frame object from the call stack.\n"
"\n"
"If optional integer depth is given, return the frame object that many\n"
"calls below the top of the stack.  If that is deeper than the call stack,\n"
"ValueError is raised.  The default for depth is zero, returning the frame\n"
"at the top of the call stack.\n"
"\n"
"This function should be used for internal and specialized purposes only.");

#define SYS__GETFRAME_METHODDEF    \
    {"_getframe", (PyCFunction)(void(*)(void))sys__getframe, METH_FASTCALL, sys__getframe__doc__},

static PyObject *
sys__getframe_impl(PyObject *module, int depth);

static PyObject *
sys__getframe(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    int depth = 0;

    if (!_PyArg_ParseStack(args, nargs, "|i:_getframe",
        &depth)) {
        goto exit;
    }
    return_value = sys__getframe_impl(module, depth);

exit:
    return return_value;
}

PyDoc_STRVAR(sys__current_frames__doc__,
"_current_frames($module, /)\n"
"--\n"
"\n"
"Return a dictionary mapping each current thread T\'s thread id to T\'s current stack frame.\n"
"\n"
"This function should be used for specialized purposes only.");

#define SYS__CURRENT_FRAMES_METHODDEF    \
    {"_current_frames", (PyCFunction)sys__current_frames, METH_NOARGS, sys__current_frames__doc__},

static PyObject *
sys__current_frames_impl(PyObject *module);

static PyObject *
sys__current_frames(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys__current_frames_impl(module);
}

PyDoc_STRVAR(sys_call_tracing__doc__,
"call_tracing($module, func, args, /)\n"
"--\n"
"\n"
"Call func(*args), while tracing is enabled.\n"
"\n"
"The tracing state is saved, and restored afterwards.  This is intended\n"
"to be called from a debugger from a checkpoint, to recursively debug\n"
"some other code.");

#define SYS_CALL_TRACING_METHODDEF    \
    {"call_tracing", (PyCFunction)(void(*)(void))sys_call_tracing, METH_FASTCALL, sys_call_tracing__doc__},

static PyObject *
sys_call_tracing_impl(PyObject *module, PyObject *func, PyObject *funcargs);

static PyObject *
sys_call_tracing(PyObject *module, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *func;
    PyObject *funcargs;

    if (!_PyArg_ParseStack(args, nargs, "OO!:call_tracing",
        &func, &PyTuple_Type, &funcargs)) {
        goto exit;
    }
    return_value = sys_call_tracing_impl(module, func, funcargs);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_callstats__doc__,
"callstats($module, /)\n"
"--\n"
"\n"
"Return a tuple of function call statistics.\n"
"\n"
"A tuple is returned only if CALL_PROFILE was defined when Python was\n"
"built.  Otherwise, this returns None.\n"
"\n"
"When enabled, this function returns detailed, implementation-specific\n"
"details about the number of function calls executed. The return value is\n"
"a 11-tuple where the entries in the tuple are counts of:\n"
"0. all function calls\n"
"1. calls to PyFunction_Type objects\n"
"2. PyFunction calls that do not create an argument tuple\n"
"3. PyFunction calls that do not create an argument tuple\n"
"   and bypass PyEval_EvalCodeEx()\n"
"4. PyMethod calls\n"
"5. PyMethod calls on bound methods\n"
"6. PyType calls\n"
"7. PyCFunction calls\n"
"8. generator calls\n"
"9. All other calls\n"
"10. Number of stack pops performed by call_function()");

#define SYS_CALLSTATS_METHODDEF    \
    {"callstats", (PyCFunction)sys_callstats, METH_NOARGS, sys_callstats__doc__},

static PyObject *
sys_callstats_impl(PyObject *module);

static PyObject *
sys_callstats(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_callstats_impl(module);
}

PyDoc_STRVAR(sys__debugmallocstats__doc__,
"_debugmallocstats($module, /)\n"
"--\n"
"\n"
"Print summary info to stderr about the state of pymalloc\'s structures.\n"
"\n"
"In Py_DEBUG mode, also perform some expensive internal consistency checks.");

#define SYS__DEBUGMALLOCSTATS_METHODDEF    \
    {"_debugmallocstats", (PyCFunction)sys__debugmallocstats, METH_NOARGS, sys__debugmallocstats__doc__},

static PyObject *
sys__debugmallocstats_impl(PyObject *module);

static PyObject *
sys__debugmallocstats(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys__debugmallocstats_impl(module);
}

PyDoc_STRVAR(sys__clear_type_cache__doc__,
"_clear_type_cache($module, /)\n"
"--\n"
"\n"
"Clear the internal type lookup cache.");

#define SYS__CLEAR_TYPE_CACHE_METHODDEF    \
    {"_clear_type_cache", (PyCFunction)sys__clear_type_cache, METH_NOARGS, sys__clear_type_cache__doc__},

static PyObject *
sys__clear_type_cache_impl(PyObject *module);

static PyObject *
sys__clear_type_cache(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys__clear_type_cache_impl(module);
}

PyDoc_STRVAR(sys_is_finalizing__doc__,
"is_finalizing($module, /)\n"
"--\n"
"\n"
"Return True if Python is exiting.");

#define SYS_IS_FINALIZING_METHODDEF    \
    {"is_finalizing", (PyCFunction)sys_is_finalizing, METH_NOARGS, sys_is_finalizing__doc__},

static PyObject *
sys_is_finalizing_impl(PyObject *module);

static PyObject *
sys_is_finalizing(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_is_finalizing_impl(module);
}

#ifndef SYS_GETWINDOWSVERSION_METHODDEF
    #define SYS_GETWINDOWSVERSION_METHODDEF
#endif /* !defined(SYS_GETWINDOWSVERSION_METHODDEF) */

#ifndef SYS_SETDLOPENFLAGS_METHODDEF
    #define SYS_SETDLOPENFLAGS_METHODDEF
#endif /* !defined(SYS_SETDLOPENFLAGS_METHODDEF) */

#ifndef SYS_GETDLOPENFLAGS_METHODDEF
    #define SYS_GETDLOPENFLAGS_METHODDEF
#endif /* !defined(SYS_GETDLOPENFLAGS_METHODDEF) */

#ifndef SYS_MDEBUG_METHODDEF
    #define SYS_MDEBUG_METHODDEF
#endif /* !defined(SYS_MDEBUG_METHODDEF) */

#ifndef SYS_GETTOTALREFCOUNT_METHODDEF
    #define SYS_GETTOTALREFCOUNT_METHODDEF
#endif /* !defined(SYS_GETTOTALREFCOUNT_METHODDEF) */

#ifndef SYS_GETCOUNTS_METHODDEF
    #define SYS_GETCOUNTS_METHODDEF
#endif /* !defined(SYS_GETCOUNTS_METHODDEF) */
/*[clinic end generated code: output=d89987671f408b07 input=a9049054013a1b77]*/
