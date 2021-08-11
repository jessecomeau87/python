
/* Frozen modules initializer
 *
 * Frozen modules are written to header files by Programs/_freeze_module.
 * These files are typically put in Python/.  Each holds an array of bytes
 * named "_Py_M__<module>", which is used below.
 *
 * These files must be regenerated any time * the corresponding .pyc
 * file would change (e.g. compiler, bytecode format, marshal format).
 * This can be done with "make regen-frozen-all".
 *
 * Additionally, specific groups of frozen modules can be regenerated:
 *
 *  import-related     make regen-importlib
 *  stdlib (partial)   make regen-frozen-stdlib
 *  test modules       make regen-frozen
 *
 * Those make targets simply run Tools/scripts/freeze_modules.py, which
 * does the following:
 *
 * 1. run Programs/_freeze_module on the target modules
 * 2. update the includes and _PyImport_FrozenModules[] in this file
 * 3. update the FROZEN_FILES variable in Makefile.pre.in
 *
 * (Note that most of the date in this file is auto-generated by the script.)
 *
 * Additional modules can be frozen by adding them to freeze_modules.py
 * and then re-running "make regen-frozen-all".  This can also be done
 * manually by following those steps, though this is not recommended.
 * Expect such manual changes to be removed the next time
 * freeze_modules.py runs.
 * */

/* In order to test the support for frozen modules, by default we
   define some simple frozen modules: __hello__, __phello__ (a package),
   and __phello__.spam.  Loading any will print some famous words... */

#include "Python.h"

/* Includes for frozen modules: */

/* importlib */
#include "importlib.h"
#include "importlib_external.h"
#include "importlib_zipimport.h"
/* stdlib */
/* Test module */
#include "frozen_hello.h"

static const struct _frozen _PyImport_FrozenModules[] = {
    /* importlib */
    {"_frozen_importlib", _Py_M__importlib__bootstrap,
        (int)sizeof(_Py_M__importlib__bootstrap)},
    {"_frozen_importlib_external", _Py_M__importlib__bootstrap_external,
        (int)sizeof(_Py_M__importlib__bootstrap_external)},
    {"zipimport", _Py_M__zipimport, (int)sizeof(_Py_M__zipimport)},

    /* stdlib */
    /* without site (python -S) */
    /* with site */

    /* Test module */
    {"__hello__", _Py_M__hello, (int)sizeof(_Py_M__hello)},
    /* Test package (negative size indicates package-ness) */
    {"__phello__", _Py_M__hello, -(int)sizeof(_Py_M__hello)},
    {"__phello__.spam", _Py_M__hello, (int)sizeof(_Py_M__hello)},

    {0, 0, 0} /* sentinel */
};

/* Embedding apps may change this pointer to point to their favorite
   collection of frozen modules: */

const struct _frozen *PyImport_FrozenModules = _PyImport_FrozenModules;
