"""Provide access to Python's configuration information.  The specific
configuration variables available depend heavily on the platform and
configuration.  The values may be retrieved using
get_config_var(name), and the list of variables is available via
get_config_vars().keys().  Additional convenience functions are also
available.

Written by:   Fred L. Drake, Jr.
Email:        <fdrake@acm.org>
"""

import _imp
import os
import re
import sys
import warnings

from .errors import DistutilsPlatformError

from sysconfig import _PREFIX as PREFIX
from sysconfig import _BASE_PREFIX as BASE_PREFIX
from sysconfig import _EXEC_PREFIX as EXEC_PREFIX
from sysconfig import _BASE_EXEC_PREFIX as BASE_EXEC_PREFIX
from sysconfig import parse_config_h as sysconfig_parse_config_h
from sysconfig import _parse_makefile as sysconfig_parse_makefile

from sysconfig import _is_python_source_dir
from sysconfig import _sys_home

from sysconfig import expand_makefile_vars
from sysconfig import get_config_var
from sysconfig import get_python_version
from sysconfig import get_python_lib


warnings.warn(
    'the distutils.sysconfig module is deprecated, use sysconfig instead',
    DeprecationWarning,
    stacklevel=2
)


# Following functions are the same as in sysconfig but with different API
def parse_config_h(fp, g=None):
    return sysconfig_parse_config_h(fp, vars=g)


def parse_makefile(fn, g=None):
    return sysconfig_parse_makefile(fn, vars=g)


# Path to the base directory of the project. On Windows the binary may
# live in project/PCbuild/win32 or project/PCbuild/amd64.
# set for cross builds
if "_PYTHON_PROJECT_BASE" in os.environ:
    project_base = os.path.abspath(os.environ["_PYTHON_PROJECT_BASE"])
else:
    if sys.executable:
        project_base = os.path.dirname(os.path.abspath(sys.executable))
    else:
        # sys.executable can be empty if argv[0] has been changed and Python is
        # unable to retrieve the real program name
        project_base = os.getcwd()


if os.name == 'nt':
    def _fix_pcbuild(d):
        if d and os.path.normcase(d).startswith(
                os.path.normcase(os.path.join(PREFIX, "PCbuild"))):
            return PREFIX
        return d
    project_base = _fix_pcbuild(project_base)
    _sys_home = _fix_pcbuild(_sys_home)

def _python_build():
    if _sys_home:
        return _is_python_source_dir(_sys_home)
    return _is_python_source_dir(project_base)

python_build = _python_build()


# Calculate the build qualifier flags if they are defined.  Adding the flags
# to the include and lib directories only makes sense for an installation, not
# an in-source build.
build_flags = ''
try:
    if not python_build:
        build_flags = sys.abiflags
except AttributeError:
    # It's not a configure-based build, so the sys module doesn't have
    # this attribute, which is fine.
    pass


def get_python_inc(plat_specific=0, prefix=None):
    """Return the directory containing installed Python header files.

    If 'plat_specific' is false (the default), this is the path to the
    non-platform-specific header files, i.e. Python.h and so on;
    otherwise, this is the path to platform-specific header files
    (namely pyconfig.h).

    If 'prefix' is supplied, use it instead of sys.base_prefix or
    sys.base_exec_prefix -- i.e., ignore 'plat_specific'.
    """
    if prefix is None:
        prefix = plat_specific and BASE_EXEC_PREFIX or BASE_PREFIX
    if os.name == "posix":
        if python_build:
            # Assume the executable is in the build directory.  The
            # pyconfig.h file should be in the same directory.  Since
            # the build directory may not be the source directory, we
            # must use "srcdir" from the makefile to find the "Include"
            # directory.
            if plat_specific:
                return _sys_home or project_base
            else:
                incdir = os.path.join(get_config_var('srcdir'), 'Include')
                return os.path.normpath(incdir)
        python_dir = 'python' + get_python_version() + build_flags
        return os.path.join(prefix, "include", python_dir)
    elif os.name == "nt":
        if python_build:
            # Include both the include and PC dir to ensure we can find
            # pyconfig.h
            return (os.path.join(prefix, "include") + os.path.pathsep +
                    os.path.join(prefix, "PC"))
        return os.path.join(prefix, "include")
    else:
        raise DistutilsPlatformError(
            "I don't know where Python installs its C header files "
            "on platform '%s'" % os.name)


def customize_compiler(compiler):
    """Do any platform-specific customization of a CCompiler instance.

    Mainly needed on Unix, so we can plug in the information that
    varies across Unices and is stored in Python's Makefile.
    """
    if compiler.compiler_type == "unix":
        if sys.platform == "darwin":
            # Perform first-time customization of compiler-related
            # config vars on OS X now that we know we need a compiler.
            # This is primarily to support Pythons from binary
            # installers.  The kind and paths to build tools on
            # the user system may vary significantly from the system
            # that Python itself was built on.  Also the user OS
            # version and build tools may not support the same set
            # of CPU architectures for universal builds.
            global _config_vars
            # Use get_config_var() to ensure _config_vars is initialized.
            if not get_config_var('CUSTOMIZED_OSX_COMPILER'):
                import _osx_support
                _osx_support.customize_compiler(_config_vars)
                _config_vars['CUSTOMIZED_OSX_COMPILER'] = 'True'

        (cc, cxx, cflags, ccshared, ldshared, shlib_suffix, ar, ar_flags) = \
            get_config_vars('CC', 'CXX', 'CFLAGS',
                            'CCSHARED', 'LDSHARED', 'SHLIB_SUFFIX', 'AR', 'ARFLAGS')

        if 'CC' in os.environ:
            newcc = os.environ['CC']
            if (sys.platform == 'darwin'
                    and 'LDSHARED' not in os.environ
                    and ldshared.startswith(cc)):
                # On OS X, if CC is overridden, use that as the default
                #       command for LDSHARED as well
                ldshared = newcc + ldshared[len(cc):]
            cc = newcc
        if 'CXX' in os.environ:
            cxx = os.environ['CXX']
        if 'LDSHARED' in os.environ:
            ldshared = os.environ['LDSHARED']
        if 'CPP' in os.environ:
            cpp = os.environ['CPP']
        else:
            cpp = cc + " -E"           # not always
        if 'LDFLAGS' in os.environ:
            ldshared = ldshared + ' ' + os.environ['LDFLAGS']
        if 'CFLAGS' in os.environ:
            cflags = cflags + ' ' + os.environ['CFLAGS']
            ldshared = ldshared + ' ' + os.environ['CFLAGS']
        if 'CPPFLAGS' in os.environ:
            cpp = cpp + ' ' + os.environ['CPPFLAGS']
            cflags = cflags + ' ' + os.environ['CPPFLAGS']
            ldshared = ldshared + ' ' + os.environ['CPPFLAGS']
        if 'AR' in os.environ:
            ar = os.environ['AR']
        if 'ARFLAGS' in os.environ:
            archiver = ar + ' ' + os.environ['ARFLAGS']
        else:
            archiver = ar + ' ' + ar_flags

        cc_cmd = cc + ' ' + cflags
        compiler.set_executables(
            preprocessor=cpp,
            compiler=cc_cmd,
            compiler_so=cc_cmd + ' ' + ccshared,
            compiler_cxx=cxx,
            linker_so=ldshared,
            linker_exe=cc,
            archiver=archiver)

        compiler.shared_lib_extension = shlib_suffix


def get_config_h_filename():
    """Return full pathname of installed pyconfig.h file."""
    if python_build:
        if os.name == "nt":
            inc_dir = os.path.join(_sys_home or project_base, "PC")
        else:
            inc_dir = _sys_home or project_base
    else:
        inc_dir = get_python_inc(plat_specific=1)

    return os.path.join(inc_dir, 'pyconfig.h')


def get_makefile_filename():
    """Return full pathname of installed Makefile from the Python build."""
    if python_build:
        return os.path.join(_sys_home or project_base, "Makefile")
    lib_dir = get_python_lib(plat_specific=0, standard_lib=1)
    config_file = 'config-{}{}'.format(get_python_version(), build_flags)
    if hasattr(sys.implementation, '_multiarch'):
        config_file += '-%s' % sys.implementation._multiarch
    return os.path.join(lib_dir, config_file, 'Makefile')


_config_vars = None

def _init_posix():
    """Initialize the module as appropriate for POSIX systems."""
    # _sysconfigdata is generated at build time, see the sysconfig module
    name = os.environ.get('_PYTHON_SYSCONFIGDATA_NAME',
        '_sysconfigdata_{abi}_{platform}_{multiarch}'.format(
        abi=sys.abiflags,
        platform=sys.platform,
        multiarch=getattr(sys.implementation, '_multiarch', ''),
    ))
    _temp = __import__(name, globals(), locals(), ['build_time_vars'], 0)
    build_time_vars = _temp.build_time_vars
    global _config_vars
    _config_vars = {}
    _config_vars.update(build_time_vars)


def _init_nt():
    """Initialize the module as appropriate for NT"""
    g = {}
    # set basic install directories
    g['LIBDEST'] = get_python_lib(plat_specific=0, standard_lib=1)
    g['BINLIBDEST'] = get_python_lib(plat_specific=1, standard_lib=1)

    # XXX hmmm.. a normal install puts include files here
    g['INCLUDEPY'] = get_python_inc(plat_specific=0)

    g['EXT_SUFFIX'] = _imp.extension_suffixes()[0]
    g['EXE'] = ".exe"
    g['VERSION'] = get_python_version().replace(".", "")
    g['BINDIR'] = os.path.dirname(os.path.abspath(sys.executable))

    global _config_vars
    _config_vars = g


def get_config_vars(*args):
    """With no arguments, return a dictionary of all configuration
    variables relevant for the current platform.  Generally this includes
    everything needed to build extensions and install both pure modules and
    extensions.  On Unix, this means every variable defined in Python's
    installed Makefile; on Windows it's a much smaller set.

    With arguments, return a list of values that result from looking up
    each argument in the configuration variable dictionary.
    """
    global _config_vars
    if _config_vars is None:
        func = globals().get("_init_" + os.name)
        if func:
            func()
        else:
            _config_vars = {}

        # Normalized versions of prefix and exec_prefix are handy to have;
        # in fact, these are the standard versions used most places in the
        # Distutils.
        _config_vars['prefix'] = PREFIX
        _config_vars['exec_prefix'] = EXEC_PREFIX

        # For backward compatibility, see issue19555
        SO = _config_vars.get('EXT_SUFFIX')
        if SO is not None:
            _config_vars['SO'] = SO

        # Always convert srcdir to an absolute path
        srcdir = _config_vars.get('srcdir', project_base)
        if os.name == 'posix':
            if python_build:
                # If srcdir is a relative path (typically '.' or '..')
                # then it should be interpreted relative to the directory
                # containing Makefile.
                base = os.path.dirname(get_makefile_filename())
                srcdir = os.path.join(base, srcdir)
            else:
                # srcdir is not meaningful since the installation is
                # spread about the filesystem.  We choose the
                # directory containing the Makefile since we know it
                # exists.
                srcdir = os.path.dirname(get_makefile_filename())
        _config_vars['srcdir'] = os.path.abspath(os.path.normpath(srcdir))

        # Convert srcdir into an absolute path if it appears necessary.
        # Normally it is relative to the build directory.  However, during
        # testing, for example, we might be running a non-installed python
        # from a different directory.
        if python_build and os.name == "posix":
            base = project_base
            if (not os.path.isabs(_config_vars['srcdir']) and
                base != os.getcwd()):
                # srcdir is relative and we are not in the same directory
                # as the executable. Assume executable is in the build
                # directory and make srcdir absolute.
                srcdir = os.path.join(base, _config_vars['srcdir'])
                _config_vars['srcdir'] = os.path.normpath(srcdir)

        # OS X platforms require special customization to handle
        # multi-architecture, multi-os-version installers
        if sys.platform == 'darwin':
            import _osx_support
            _osx_support.customize_config_vars(_config_vars)

    if args:
        vals = []
        for name in args:
            vals.append(_config_vars.get(name))
        return vals
    else:
        return _config_vars

