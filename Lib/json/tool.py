r"""Command-line tool to validate and pretty-print JSON

Usage::

    $ echo '{"json":"obj"}' | python -m json.tool
    {
        "json": "obj"
    }
    $ echo '{ 1.2:3.4}' | python -m json.tool
    Expecting property name enclosed in double quotes: line 1 column 3 (char 2)

"""
import argparse
import json
import sys


def main():
    prog = 'python -m json.tool'
    description = ('A simple command line interface for json module '
                   'to validate and pretty-print JSON objects.')
    parser = argparse.ArgumentParser(prog=prog, description=description)
    parser.add_argument('infile', nargs='?', type=argparse.FileType(),
                        help='a JSON file to be validated or pretty-printed')
    parser.add_argument('outfile', nargs='?', default='-',
                        help='write the output of infile to outfile')
    parser.add_argument('--sort-keys', action='store_true', default=False,
                        help='sort the output of dictionaries alphabetically by key')
    parser.add_argument('--json-lines', action='store_true', default=False,
                        help='parse input using the jsonlines format')
    options = parser.parse_args()

    infile = options.infile or sys.stdin
    sort_keys = options.sort_keys
    json_lines = options.json_lines
    with infile:
        try:
            if json_lines:
                objs = tuple(json.loads(line) for line in infile)
            else:
                objs = (json.load(infile), )
        except ValueError as e:
            raise SystemExit(e)

    try:
        outfile = sys.stdout if options.outfile == '-' else open(options.outfile, 'w')
    except IOError as e:
        parser.error(f"can't open '{options.outfile}': {str(e)}")
    with outfile:
        for obj in objs:
            json.dump(obj, outfile, sort_keys=sort_keys, indent=4)
            outfile.write('\n')


if __name__ == '__main__':
    main()
