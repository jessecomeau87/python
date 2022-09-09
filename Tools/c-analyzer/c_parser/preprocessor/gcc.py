import os.path
import re

from . import common as _common


TOOL = 'gcc'

# https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
# flags:
#  1  start of a new file
#  2  returning to a file (after including another)
#  3  following text comes from a system header file
#  4  following text treated wrapped in implicit extern "C" block
LINE_MARKER_RE = re.compile(r'^# (\d+) "([^"]+)"((?: [1234])*)$')
PREPROC_DIRECTIVE_RE = re.compile(r'^\s*#\s*(\w+)\b.*')
COMPILER_DIRECTIVE_RE = re.compile(r'''
    ^
    (.*?)  # <before>
    (__\w+__)  # <directive>
    \s*
    [(] [(]
    (
        [^()]*
        (?:
            [(]
            [^()]*
            [)]
            [^()]*
         )*
     )  # <args>
    ( [)] [)] )?  # <closed>
''', re.VERBOSE)

POST_ARGS = (
    '-pthread',
    '-std=c99',
    #'-g',
    #'-Og',
    #'-Wno-unused-result',
    #'-Wsign-compare',
    #'-Wall',
    #'-Wextra',
    '-E',
)


def preprocess(filename,
               incldirs=None,
               includes=None,
               macros=None,
               samefiles=None,
               ):
    text = _common.preprocess(
        TOOL,
        filename,
        incldirs=incldirs,
        includes=includes,
        macros=macros,
        #preargs=PRE_ARGS,
        postargs=POST_ARGS,
        executable=['gcc'],
        compiler='unix',
    )
    return _iter_lines(text, filename, samefiles)


def _iter_lines(text, filename, samefiles, *, raw=False):
    lines = iter(text.splitlines())

    # The first line is special.
    # The next two lines are consistent.
    for expected in [
        f'# 1 "{filename}"',
        '# 1 "<built-in>"',
        '# 1 "<command-line>"',
    ]:
        line = next(lines)
        if line != expected:
            raise NotImplementedError((line, expected))

    # Do all the CLI-provided includes.
    filter_reqfile = (lambda f: _filter_orig_file(f, filename, samefiles))
    make_info = (lambda lno: _common.FileInfo(filename, lno))
    last = None
    for line in lines:
        assert last != filename, (last,)
        lno, included, flags = _parse_marker_line(line, filename)
        if not included:
            raise NotImplementedError((line,))
        if included == filename:
            # This will be the last one.
            assert not flags, (line, flags)
        else:
            assert 1 in flags, (line, flags)
        yield from _iter_top_include_lines(
            lines,
            included,
            filter_reqfile,
            make_info,
            raw,
        )
        last = included
    # The last one is always the requested file.
    assert included == filename, (line,)


def _iter_top_include_lines(lines, topfile, filter_reqfile, make_info, raw):
    partial = 0  # depth
    files = [topfile]
    # We start at 1 in case there are source lines (including blank onces)
    # before the first marker line.  Also, we already verified in
    # _parse_marker_line() that the preprocessor reported lno as 1.
    lno = 1
    for line in lines:
        if line == '# 1 "<command-line>" 2':
            # We're done with this top-level include.
            return

        _lno, included, flags = _parse_marker_line(line)
        if included:
            lno = _lno
            # We hit a marker line.
            if 1 in flags:
                # We're entering a file.
                # XXX Cycles are unexpected?
                #assert included not in files, (line, files)
                files.append(included)
            elif 2 in flags:
                # We're returning to a file.
                assert files and included in files, (line, files)
                assert included != files[-1], (line, files)
                while files[-1] != included:
                    files.pop()
                # XXX How can a file return to line 1?
                #assert lno > 1, (line, lno)
            else:
                # It's the next line from the file.
                assert included == files[-1], (line, files)
                assert lno > 1, (line, lno)
        elif not files:
            raise NotImplementedError((line,))
        elif filter_reqfile(files[-1]):
            assert lno is not None, (line, files[-1])
            if (m := PREPROC_DIRECTIVE_RE.match(line)):
                name, = m.groups()
                if name != 'pragma':
                    raise Exception(line)
            else:
                if not raw:
                    line, partial = _strip_directives(line, partial=partial)
                yield _common.SourceLine(
                    make_info(lno),
                    'source',
                    line or '',
                    None,
                )
            lno += 1


def _parse_marker_line(line, reqfile=None):
    m = LINE_MARKER_RE.match(line)
    if not m:
        return None, None, None
    lno, origfile, flags = m.groups()
    lno = int(lno)
    assert lno > 0, (line, lno)
    assert origfile not in ('<built-in>', '<command-line>'), (line,)
    flags = set(int(f) for f in flags.split()) if flags else ()

    if 1 in flags:
        # We're entering a file.
        assert lno == 1, (line, lno)
        assert 2 not in flags, (line,)
    elif 2 in flags:
        # We're returning to a file.
        #assert lno > 1, (line, lno)
        pass
    elif reqfile and origfile == reqfile:
        # We're starting the requested file.
        assert lno == 1, (line, lno)
        assert not flags, (line, flags)
    else:
        # It's the next line from the file.
        assert lno > 1, (line, lno)
    return lno, origfile, flags


def _strip_directives(line, partial=0):
    # We assume there are no string literals with parens in directive bodies.
    while partial > 0:
        if not (m := re.match(r'[^{}]*([()])', line)):
            return None, partial
        delim, = m.groups()
        partial += 1 if delim == '(' else -1  # opened/closed
        line = line[m.end():]

    line = re.sub(r'__extension__', '', line)

    while (m := COMPILER_DIRECTIVE_RE.match(line)):
        before, _, _, closed = m.groups()
        if closed:
            line = f'{before} {line[m.end():]}'
        else:
            after, partial = _strip_directives(line[m.end():], 2)
            line = f'{before} {after or ""}'
            if partial:
                break

    return line, partial


def _filter_orig_file(origfile, current, samefiles):
    if origfile == current:
        return True
    if origfile == '<stdin>':
        return True
    if os.path.isabs(origfile):
        return False

    for filename in samefiles or ():
        if filename.endswith(os.path.sep):
            filename += os.path.basename(current)
        if origfile == filename:
            return True

    return False
