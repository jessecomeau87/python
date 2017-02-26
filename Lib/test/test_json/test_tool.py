import os
import sys
import textwrap
import unittest
from subprocess import Popen, PIPE
from test import support
from test.support.script_helper import assert_python_ok


class TestTool(unittest.TestCase):
    data = """

        [["blorpie"],[ "whoops" ] , [
                                 ],\t"d-shtaeou",\r"d-nthiouh",
        "i-vhbjkhnth", {"nifty":87}, {"morefield" :\tfalse,"field"
            :"yes"}  ]
           """

    expect_without_sort_keys = textwrap.dedent("""\
    [
        [
            "blorpie"
        ],
        [
            "whoops"
        ],
        [],
        "d-shtaeou",
        "d-nthiouh",
        "i-vhbjkhnth",
        {
            "nifty": 87
        },
        {
            "field": "yes",
            "morefield": false
        }
    ]
    """)

    expect = textwrap.dedent("""\
    [
        [
            "blorpie"
        ],
        [
            "whoops"
        ],
        [],
        "d-shtaeou",
        "d-nthiouh",
        "i-vhbjkhnth",
        {
            "nifty": 87
        },
        {
            "morefield": false,
            "field": "yes"
        }
    ]
    """)

    def test_stdin_stdout(self):
        args = sys.executable, '-m', 'json.tool'
        with Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE) as proc:
            out, err = proc.communicate(self.data.encode())
        self.assertEqual(out.splitlines(), self.expect.encode().splitlines())
        self.assertEqual(err, b'')

    def _create_infile(self):
        infile = support.TESTFN
        with open(infile, "w") as fp:
            self.addCleanup(os.remove, infile)
            fp.write(self.data)
        return infile

    def test_infile_stdout(self):
        infile = self._create_infile()
        rc, out, err = assert_python_ok('-m', 'json.tool', infile)
        self.assertEqual(rc, 0)
        self.assertEqual(out.splitlines(), self.expect.encode().splitlines())
        self.assertEqual(err, b'')

    def test_infile_outfile(self):
        infile = self._create_infile()
        outfile = support.TESTFN + '.out'
        rc, out, err = assert_python_ok('-m', 'json.tool', infile, outfile)
        self.addCleanup(os.remove, outfile)
        with open(outfile, "r") as fp:
            self.assertEqual(fp.read(), self.expect)
        self.assertEqual(rc, 0)
        self.assertEqual(out, b'')
        self.assertEqual(err, b'')

    def test_help_flag(self):
        rc, out, err = assert_python_ok('-m', 'json.tool', '-h')
        self.assertEqual(rc, 0)
        self.assertTrue(out.startswith(b'usage: '))
        self.assertEqual(err, b'')

    def test_sort_keys_flag(self):
        infile = self._create_infile()
        rc, out, err = assert_python_ok('-m', 'json.tool', '--sort-keys', infile)
        self.assertEqual(rc, 0)
        self.assertEqual(out.splitlines(),
                         self.expect_without_sort_keys.encode().splitlines())
        self.assertEqual(err, b'')

    def test_indent(self):
        json_stdin = b'[1, 2]'
        expect = textwrap.dedent('''\
        [
          1,
          2
        ]
        ''').encode()
        args = sys.executable, '-m', 'json.tool', '--indent', '2'
        with Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE) as proc:
            json_stdout, err = proc.communicate(json_stdin)
        self.assertEqual(expect.splitlines(), json_stdout.splitlines())
        self.assertEqual(err, b'')

    def test_no_indent(self):
        json_stdin = b'[1, 2]'
        args = sys.executable, '-m', 'json.tool', '--no-indent'
        with Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE) as proc:
            json_stdout, err = proc.communicate(json_stdin)
        self.assertEqual(json_stdin.splitlines(), json_stdout.splitlines())
        self.assertEqual(err, b'')

    def test_ensure_ascii(self):
        json_stdin = '"\xA7 \N{snake} \u03B4 \U0001D037"'.encode()
        expect = b'"\\u00a7 \\ud83d\\udc0d \\u03b4 \\ud834\\udc37"\n'
        args = sys.executable, '-m', 'json.tool'
        with Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE) as proc:
            json_stdout, err = proc.communicate(json_stdin)
        self.assertEqual(expect.splitlines(), json_stdout.splitlines())
        self.assertEqual(err, b'')

    def test_no_ensure_ascii(self):
        json_stdin = '"\xA7 \N{snake} \u03B4 \U0001D037"'.encode()
        args = sys.executable, '-m', 'json.tool', '--no-ensure-ascii'
        with Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE) as proc:
            json_stdout, err = proc.communicate(json_stdin)
        self.assertEqual(json_stdin.splitlines(), json_stdout.splitlines())
        self.assertEqual(err, b'')
