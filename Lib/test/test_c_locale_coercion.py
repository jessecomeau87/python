# Tests the attempted automatic coercion of the C locale to a UTF-8 locale

import unittest
import os
import sys
import sysconfig
import shutil
import subprocess
from collections import namedtuple

import test.support
from test.support.script_helper import (
    run_python_until_end,
    interpreter_requires_environment,
)

# Set our expectation for the default encoding used in the C locale
# for the filesystem encoding and the standard streams
if sys.platform == "darwin":
    EXPECTED_C_LOCALE_FSENCODING = "utf-8"
else:
    EXPECTED_C_LOCALE_FSENCODING = "ascii"

# XXX (ncoghlan): The above is probably still wrong for:
# * Windows when PYTHONLEGACYWINDOWSFSENCODING is set
# * AIX and any other platforms that use latin-1 in the C locale

# Use the target locale order of Python/pylifecycle.c
_C_UTF8_LOCALES = ("C.UTF-8", "C.utf8", "UTF-8")

# There's no reliable cross-platform way of checking locale alias
# lists, so the only way of knowing which of these locales will work
# is to try them with locale.setlocale(). We do that in a subprocess
# to avoid altering the locale of the test runner.
def _set_locale_in_subprocess(locale_name):
    cmd_fmt = "import locale; print(locale.setlocale(locale.LC_CTYPE, '{}'))"
    cmd = cmd_fmt.format(locale_name)
    result, py_cmd = run_python_until_end("-c", cmd, __isolated=True)
    return result.rc == 0

_EncodingDetails = namedtuple("EncodingDetails",
                              "fsencoding stdin_info stdout_info stderr_info")

class EncodingDetails(_EncodingDetails):
    CHILD_PROCESS_SCRIPT = ";".join([
        "import sys",
        "print(sys.getfilesystemencoding())",
        "print(sys.stdin.encoding + ':' + sys.stdin.errors)",
        "print(sys.stdout.encoding + ':' + sys.stdout.errors)",
        "print(sys.stderr.encoding + ':' + sys.stderr.errors)",
    ])

    @classmethod
    def get_expected_details(cls, expected_fsencoding):
        """Returns expected child process details for a given encoding"""
        _stream = expected_fsencoding + ":{}"
        # stdin and stdout should use surrogateescape either because the
        # coercion triggered, or because the C locale was detected
        stream_info = 2*[_stream.format("surrogateescape")]
        # stderr should always use backslashreplace
        stream_info.append(_stream.format("backslashreplace"))
        return dict(cls(expected_fsencoding, *stream_info)._asdict())

    @staticmethod
    def _handle_output_variations(data):
        """Adjust the output to handle platform specific idiosyncrasies

        * Some platforms report ASCII as ANSI_X3.4-1968
        * Some platforms report ASCII as US-ASCII
        * Some platforms report UTF-8 instead of utf-8
        """
        data = data.replace(b"ANSI_X3.4-1968", b"ascii")
        data = data.replace(b"US-ASCII", b"ascii")
        data = data.lower()
        return data

    @classmethod
    def get_child_details(cls, env_vars):
        """Retrieves fsencoding and standard stream details from a child process

        Returns (encoding_details, stderr_lines):

        - encoding_details: EncodingDetails for eager decoding
        - stderr_lines: result of calling splitlines() on the stderr output

        The child is run in isolated mode if the current interpreter supports
        that.
        """
        result, py_cmd = run_python_until_end(
            "-c", cls.CHILD_PROCESS_SCRIPT,
            __isolated=True,
            **env_vars
        )
        if not result.rc == 0:
            result.fail(py_cmd)
        # All subprocess outputs in this test case should be pure ASCII
        adjusted_output = cls._handle_output_variations(result.out)
        stdout_lines = adjusted_output.decode("ascii").rstrip().splitlines()
        child_encoding_details = dict(cls(*stdout_lines)._asdict())
        stderr_lines = result.err.decode("ascii").rstrip().splitlines()
        return child_encoding_details, stderr_lines


class _ChildProcessEncodingTestCase(unittest.TestCase):
    # Base class to check for expected encoding details in a child process

    def _check_child_encoding_details(self,
                                      env_vars,
                                      expected_fsencoding):
        """Check the C locale handling for the given process environment

        Parameters:
            expected_fsencoding: the encoding the child is expected to report
            allow_c_locale: setting to use for PYTHONALLOWCLOCALE
              None: don't set the variable at all
              str: the value set in the child's environment
        """
        result = EncodingDetails.get_child_details(env_vars)
        encoding_details, stderr_lines = result
        self.assertEqual(encoding_details,
                         EncodingDetails.get_expected_details(
                             expected_fsencoding))
        self.assertEqual(stderr_lines, [])


AVAILABLE_TARGETS = None

def setUpModule():
    global AVAILABLE_TARGETS

    if AVAILABLE_TARGETS is not None:
        # initialization already done
        return
    AVAILABLE_TARGETS = []

    # Find the target locales available in the current system
    for target_locale in _C_UTF8_LOCALES:
        if _set_locale_in_subprocess(target_locale):
            AVAILABLE_TARGETS.append(target_locale)



class _LocaleCoercionTargetsTestCase(_ChildProcessEncodingTestCase):
    # Base class for test cases that rely on coercion targets being defined

    @classmethod
    def setUpClass(cls):
        if not AVAILABLE_TARGETS:
            raise unittest.SkipTest("No C-with-UTF-8 locale available")


class LocaleConfigurationTests(_LocaleCoercionTargetsTestCase):
    # Test explicit external configuration via the process environment

    def test_external_target_locale_configuration(self):
        # Explicitly setting a target locale should give the same behaviour as
        # is seen when implicitly coercing to that target locale
        self.maxDiff = None

        expected_fsencoding = "utf-8"

        base_var_dict = {
            "LANG": "",
            "LC_CTYPE": "",
            "LC_ALL": "",
        }
        for env_var in ("LANG", "LC_CTYPE"):
            for locale_to_set in AVAILABLE_TARGETS:
                # XXX (ncoghlan): LANG=UTF-8 doesn't appear to work as
                #                 expected, so skip that combination for now
                if env_var == "LANG" and locale_to_set == "UTF-8":
                    continue

                with self.subTest(env_var=env_var,
                                  configured_locale=locale_to_set):
                    var_dict = base_var_dict.copy()
                    var_dict[env_var] = locale_to_set
                    self._check_child_encoding_details(var_dict,
                                                       expected_fsencoding)



@test.support.cpython_only
@unittest.skipUnless(sysconfig.get_config_var("PY_COERCE_C_LOCALE"),
                     "C locale coercion disabled at build time")
class LocaleCoercionTests(_LocaleCoercionTargetsTestCase):
    # Test implicit reconfiguration of the environment during CLI startup

    def _check_c_locale_coercion(self, expected_fsencoding, coerce_c_locale):
        """Check the C locale handling for various configurations

        Parameters:
            expected_fsencoding: the encoding the child is expected to report
            allow_c_locale: setting to use for PYTHONALLOWCLOCALE
              None: don't set the variable at all
              str: the value set in the child's environment
        """

        base_var_dict = {
            "LANG": "",
            "LC_CTYPE": "",
            "LC_ALL": "",
        }
        for env_var in ("LANG", "LC_CTYPE"):
            for locale_to_set in ("", "C", "POSIX", "invalid.ascii"):
                with self.subTest(env_var=env_var,
                                  nominal_locale=locale_to_set,
                                  PYTHONCOERCECLOCALE=coerce_c_locale):
                    var_dict = base_var_dict.copy()
                    var_dict[env_var] = locale_to_set
                    if coerce_c_locale is not None:
                        var_dict["PYTHONCOERCECLOCALE"] = coerce_c_locale
                    self._check_child_encoding_details(var_dict,
                                                       expected_fsencoding)

    def test_test_PYTHONCOERCECLOCALE_not_set(self):
        # This should coerce to the first available target locale by default
        self._check_c_locale_coercion("utf-8", coerce_c_locale=None)

    def test_PYTHONCOERCECLOCALE_not_zero(self):
        # *Any* string other that "0" is considered "set" for our purposes
        # and hence should result in the locale coercion being enabled
        for setting in ("", "1", "true", "false"):
            self._check_c_locale_coercion("utf-8", coerce_c_locale=setting)

    def test_PYTHONCOERCECLOCALE_set_to_zero(self):
        # The setting "0" should result in the locale coercion being disabled
        self._check_c_locale_coercion(EXPECTED_C_LOCALE_FSENCODING,
                                      coerce_c_locale="0")


def test_main():
    test.support.run_unittest(
        LocaleConfigurationTests,
        LocaleCoercionTests
    )
    test.support.reap_children()

if __name__ == "__main__":
    test_main()
