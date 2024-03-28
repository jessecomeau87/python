import contextlib
import json
import io
import os
import os.path
import pickle
import queue
#import select
import subprocess
import sys
import tempfile
from textwrap import dedent, indent
import threading
import types
import unittest
import warnings

from test import support
from test.support import os_helper
from test.support import import_helper

_interpreters = import_helper.import_module('_xxsubinterpreters')
from test.support import interpreters


try:
    import _testinternalcapi
except ImportError:
    _testinternalcapi = None
else:
    run_in_interpreter = _testinternalcapi.run_in_subinterp_with_config

def requires__testinternalcapi(func):
    return unittest.skipIf(_testinternalcapi is None, "test requires _testinternalcapi module")(func)


def _dump_script(text):
    lines = text.splitlines()
    print('-' * 20)
    for i, line in enumerate(lines, 1):
        print(f' {i:>{len(str(len(lines)))}}  {line}')
    print('-' * 20)


def _close_file(file):
    try:
        if hasattr(file, 'close'):
            file.close()
        else:
            os.close(file)
    except OSError as exc:
        if exc.errno != 9:
            raise  # re-raise
        # It was closed already.


class CapturedResults:

    STDIO = dedent("""\
        import contextlib, io
        with open({w_pipe}, 'wb', buffering=0) as _spipe_{stream}:
            _captured_std{stream} = io.StringIO()
            with contextlib.redirect_std{stream}(_captured_std{stream}):
                #########################
                # begin wrapped script

                {indented}

                # end wrapped script
                #########################
            text = _captured_std{stream}.getvalue()
            _spipe_{stream}.write(text.encode('utf-8'))
        """)[:-1]
    EXC = dedent("""\
        import json, traceback
        with open({w_pipe}, 'wb', buffering=0) as _spipe_exc:
            try:
                #########################
                # begin wrapped script

                {indented}

                # end wrapped script
                #########################
            except Exception as exc:
                # This matches what _interpreters.exec() returns.
                text = json.dumps(dict(
                    type=dict(
                        __name__=type(exc).__name__,
                        __qualname__=type(exc).__qualname__,
                        __module__=type(exc).__module__,
                    ),
                    msg=str(exc),
                    formatted=traceback.format_exception_only(exc),
                    errdisplay=traceback.format_exception(exc),
                ))
                _spipe_exc.write(text.encode('utf-8'))
        """)[:-1]

    @classmethod
    def wrap_script(cls, script, *, stdout=True, stderr=False, exc=False):
        script = dedent(script).strip(os.linesep)
        wrapped = script

        # Handle exc.
        if exc:
            exc = os.pipe()
            r_exc, w_exc = exc
            indented = wrapped.replace('\n', '\n        ')
            wrapped = cls.EXC.format(
                w_pipe=w_exc,
                indented=indented,
            )
        else:
            exc = None

        # Handle stdout.
        if stdout:
            stdout = os.pipe()
            r_out, w_out = stdout
            indented = wrapped.replace('\n', '\n        ')
            wrapped = cls.STDIO.format(
                w_pipe=w_out,
                indented=indented,
                stream='out',
            )
        else:
            stdout = None

        # Handle stderr.
        if stderr == 'stdout':
            stderr = None
        elif stderr:
            stderr = os.pipe()
            r_err, w_err = stderr
            indented = wrapped.replace('\n', '\n        ')
            wrapped = cls.STDIO.format(
                w_pipe=w_err,
                indented=indented,
                stream='err',
            )
        else:
            stderr = None

        if wrapped == script:
            raise NotImplementedError
        results = cls(stdout, stderr, exc)
        return wrapped, results

    def __init__(self, out, err, exc):
        self._rf_out = None
        self._rf_err = None
        self._rf_exc = None
        self._w_out = None
        self._w_err = None
        self._w_exc = None

        if out is not None:
            r_out, w_out = out
            self._rf_out = open(r_out, 'rb', buffering=0)
            self._w_out = w_out

        if err is not None:
            r_err, w_err = err
            self._rf_err = open(r_err, 'rb', buffering=0)
            self._w_err = w_err

        if exc is not None:
            r_exc, w_exc = exc
            self._rf_exc = open(r_exc, 'rb', buffering=0)
            self._w_exc = w_exc

        self._buf_out = b''
        self._buf_err = b''
        self._buf_exc = b''
        self._exc = None

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def close(self):
        if self._w_out is not None:
            _close_file(self._w_out)
            self._w_out = None
        if self._w_err is not None:
            _close_file(self._w_err)
            self._w_err = None
        if self._w_exc is not None:
            _close_file(self._w_exc)
            self._w_exc = None

        self._capture()

        if self._rf_out is not None:
            _close_file(self._rf_out)
            self._rf_out = None
        if self._rf_err is not None:
            _close_file(self._rf_err)
            self._rf_err = None
        if self._rf_exc is not None:
            _close_file(self._rf_exc)
            self._rf_exc = None

    def _capture(self):
        # Ideally this is called only after the script finishes
        # (and thus has closed the write end of the pipe.
        if self._rf_out is not None:
            chunk = self._rf_out.read(100)
            while chunk:
                self._buf_out += chunk
                chunk = self._rf_out.read(100)
        if self._rf_err is not None:
            chunk = self._rf_err.read(100)
            while chunk:
                self._buf_err += chunk
                chunk = self._rf_err.read(100)
        if self._rf_exc is not None:
            chunk = self._rf_exc.read(100)
            while chunk:
                self._buf_exc += chunk
                chunk = self._rf_exc.read(100)

    def stdout(self):
        self._capture()
        return self._buf_out.decode('utf-8')

    def stderr(self):
        self._capture()
        return self._buf_err.decode('utf-8')

    def exc(self):
        if self._exc is not None:
            return self._exc
        self._capture()
        if not self._buf_exc:
            return None
        try:
            data = json.loads(self._buf_exc)
        except json.decoder.JSONDecodeError:
            warnings.warn('incomplete exception data', RuntimeWarning)
            print(self._buf_exc.decode('utf-8'))
            return None
        self._exc = exc = types.SimpleNamespace(**data)
        exc.type = types.SimpleNamespace(**exc.type)
        return self._exc


def _captured_script(script, *, stdout=True, stderr=False, exc=False):
    return CapturedResults.wrap_script(
        script,
        stdout=stdout,
        stderr=stderr,
        exc=exc,
    )


def clean_up_interpreters():
    for interp in interpreters.list_all():
        if interp.id == 0:  # main
            continue
        try:
            interp.close()
        except RuntimeError:
            pass  # already destroyed


def _run_output(interp, request, init=None):
    script, (stdout, _, _) = _captured_script(request)
    with stdout:
        if init:
            interp.prepare_main(init)
        interp.exec(script)
        return stdout.read()


@contextlib.contextmanager
def _running(interp):
    r, w = os.pipe()
    def run():
        interp.exec(dedent(f"""
            # wait for "signal"
            with open({r}) as rpipe:
                rpipe.read()
            """))

    t = threading.Thread(target=run)
    t.start()

    yield

    with open(w, 'w') as spipe:
        spipe.write('done')
    t.join()


class TestBase(unittest.TestCase):

    def tearDown(self):
        clean_up_interpreters()

    def pipe(self):
        def ensure_closed(fd):
            try:
                os.close(fd)
            except OSError:
                pass
        r, w = os.pipe()
        self.addCleanup(lambda: ensure_closed(r))
        self.addCleanup(lambda: ensure_closed(w))
        return r, w

    def temp_dir(self):
        tempdir = tempfile.mkdtemp()
        tempdir = os.path.realpath(tempdir)
        self.addCleanup(lambda: os_helper.rmtree(tempdir))
        return tempdir

    @contextlib.contextmanager
    def captured_thread_exception(self):
        ctx = types.SimpleNamespace(caught=None)
        def excepthook(args):
            ctx.caught = args
        orig_excepthook = threading.excepthook
        threading.excepthook = excepthook
        try:
            yield ctx
        finally:
            threading.excepthook = orig_excepthook

    def make_script(self, filename, dirname=None, text=None):
        if text:
            text = dedent(text)
        if dirname is None:
            dirname = self.temp_dir()
        filename = os.path.join(dirname, filename)

        os.makedirs(os.path.dirname(filename), exist_ok=True)
        with open(filename, 'w', encoding='utf-8') as outfile:
            outfile.write(text or '')
        return filename

    def make_module(self, name, pathentry=None, text=None):
        if text:
            text = dedent(text)
        if pathentry is None:
            pathentry = self.temp_dir()
        else:
            os.makedirs(pathentry, exist_ok=True)
        *subnames, basename = name.split('.')

        dirname = pathentry
        for subname in subnames:
            dirname = os.path.join(dirname, subname)
            if os.path.isdir(dirname):
                pass
            elif os.path.exists(dirname):
                raise Exception(dirname)
            else:
                os.mkdir(dirname)
            initfile = os.path.join(dirname, '__init__.py')
            if not os.path.exists(initfile):
                with open(initfile, 'w'):
                    pass
        filename = os.path.join(dirname, basename + '.py')

        with open(filename, 'w', encoding='utf-8') as outfile:
            outfile.write(text or '')
        return filename

    @support.requires_subprocess()
    def run_python(self, *argv):
        proc = subprocess.run(
            [sys.executable, *argv],
            capture_output=True,
            text=True,
        )
        return proc.returncode, proc.stdout, proc.stderr

    def assert_python_ok(self, *argv):
        exitcode, stdout, stderr = self.run_python(*argv)
        self.assertNotEqual(exitcode, 1)
        return stdout, stderr

    def assert_python_failure(self, *argv):
        exitcode, stdout, stderr = self.run_python(*argv)
        self.assertNotEqual(exitcode, 0)
        return stdout, stderr

    def assert_ns_equal(self, ns1, ns2, msg=None):
        # This is mostly copied from TestCase.assertDictEqual.
        self.assertEqual(type(ns1), type(ns2))
        if ns1 == ns2:
            return

        import difflib
        import pprint
        from unittest.util import _common_shorten_repr
        standardMsg = '%s != %s' % _common_shorten_repr(ns1, ns2)
        diff = ('\n' + '\n'.join(difflib.ndiff(
                       pprint.pformat(vars(ns1)).splitlines(),
                       pprint.pformat(vars(ns2)).splitlines())))
        diff = f'namespace({diff})'
        standardMsg = self._truncateMessage(standardMsg, diff)
        self.fail(self._formatMessage(msg, standardMsg))

    def _run_string(self, interp, script):
        wrapped, results = _captured_script(script, exc=False)
        #_dump_script(wrapped)
        with results:
            if isinstance(interp, interpreters.Interpreter):
                interp.exec(script)
            else:
                err = _interpreters.run_string(interp, wrapped)
                if err is not None:
                    return None, err
        return results.stdout(), None

    def run_and_capture(self, interp, script):
        text, err = self._run_string(interp, script)
        if err is not None:
            print()
            if not err.errdisplay.startswith('Traceback '):
                print('Traceback (most recent call last):')
            print(err.errdisplay, file=sys.stderr)
            raise Exception(f'subinterpreter failed: {err.formatted}')
        else:
            return text

    @requires__testinternalcapi
    def run_external(self, script, config='legacy'):
        if isinstance(config, str):
            config = _interpreters.new_config(config)
        wrapped, results = _captured_script(script, exc=True)
        #_dump_script(wrapped)
        with results:
            rc = run_in_interpreter(wrapped, config)
            assert rc == 0, rc
        text = results.stdout()
        err = results.exc()
        return err, text
