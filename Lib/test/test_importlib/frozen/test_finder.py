from .. import abc
from .. import util

machinery = util.import_importlib('importlib.machinery')

import _imp
import marshal
import os.path
import unittest
import warnings

from test.support import import_helper, REPO_ROOT, STDLIB_DIR


def resolve_stdlib_file(name, ispkg=False):
    assert name
    if ispkg:
        return os.path.join(STDLIB_DIR, *name.split('.'), '__init__.py')
    else:
        return os.path.join(STDLIB_DIR, *name.split('.')) + '.py'


class FindSpecTests(abc.FinderTests):

    """Test finding frozen modules."""

    def find(self, name, **kwargs):
        finder = self.machinery.FrozenImporter
        with import_helper.frozen_modules():
            return finder.find_spec(name, **kwargs)

    def check_basic(self, spec, name, ispkg=False):
        self.assertEqual(spec.name, name)
        self.assertIs(spec.loader, self.machinery.FrozenImporter)
        self.assertEqual(spec.origin, 'frozen')
        self.assertFalse(spec.has_location)
        if ispkg:
            self.assertIsNotNone(spec.submodule_search_locations)
        else:
            self.assertIsNone(spec.submodule_search_locations)
        self.assertIsNotNone(spec.loader_state)

    def check_loader_state(self, spec, origname=None, filename=None):
        if not filename:
            if not origname:
                origname = spec.name

        actual = dict(vars(spec.loader_state))

        # Check the code object used to import the frozen module.
        # We can't compare the marshaled data directly because
        # marshal.dumps() would mark "expected" (below) as a ref,
        # which slightly changes the output.
        # (See https://bugs.python.org/issue34093.)
        data = actual.pop('data')
        with import_helper.frozen_modules():
            expected = _imp.get_frozen_object(spec.name)
        code = marshal.loads(data)
        self.assertEqual(code, expected)

        # Check the rest of spec.loader_state.
        expected = dict(
            origname=origname,
        )
        self.assertDictEqual(actual, expected)

    def check_search_locations(self, spec):
        # Frozen packages do not have any path entries.
        # (See https://bugs.python.org/issue21736.)
        expected = []
        self.assertListEqual(spec.submodule_search_locations, expected)

    def test_module(self):
        modules = [
            '__hello__',
            '__phello__.spam',
            '__phello__.ham.eggs',
        ]
        for name in modules:
            with self.subTest(f'{name} -> {name}'):
                spec = self.find(name)
                self.check_basic(spec, name)
                self.check_loader_state(spec)
        modules = {
            '__hello_alias__': '__hello__',
            '_frozen_importlib': 'importlib._bootstrap',
        }
        for name, origname in modules.items():
            with self.subTest(f'{name} -> {origname}'):
                spec = self.find(name)
                self.check_basic(spec, name)
                self.check_loader_state(spec, origname)
        modules = [
            '__phello__.__init__',
            '__phello__.ham.__init__',
        ]
        for name in modules:
            origname = name.rpartition('.')[0]
            filename = resolve_stdlib_file(name)
            with self.subTest(f'{name} -> {origname}'):
                spec = self.find(name)
                self.check_basic(spec, name)
                self.check_loader_state(spec, origname, filename)
        modules = {
            '__hello_only__': ('Tools', 'freeze', 'flag.py'),
        }
        for name, path in modules.items():
            origname = None
            filename = os.path.join(REPO_ROOT, *path)
            with self.subTest(f'{name} -> {filename}'):
                spec = self.find(name)
                self.check_basic(spec, name)
                self.check_loader_state(spec, origname, filename)

    def test_package(self):
        packages = [
            '__phello__',
            '__phello__.ham',
        ]
        for name in packages:
            filename = resolve_stdlib_file(name, ispkg=True)
            with self.subTest(f'{name} -> {name}'):
                spec = self.find(name)
                self.check_basic(spec, name, ispkg=True)
                self.check_loader_state(spec, name, filename)
                self.check_search_locations(spec)
        packages = {
            '__phello_alias__': '__hello__',
        }
        for name, origname in packages.items():
            filename = resolve_stdlib_file(origname, ispkg=False)
            with self.subTest(f'{name} -> {origname}'):
                spec = self.find(name)
                self.check_basic(spec, name, ispkg=True)
                self.check_loader_state(spec, origname, filename)
                self.check_search_locations(spec)

    # These are covered by test_module() and test_package().
    test_module_in_package = None
    test_package_in_package = None

    # No easy way to test.
    test_package_over_module = None

    def test_path_ignored(self):
        for name in ('__hello__', '__phello__', '__phello__.spam'):
            actual = self.find(name)
            for path in (None, object(), '', 'eggs', [], [''], ['eggs']):
                with self.subTest((name, path)):
                    spec = self.find(name, path=path)
                    self.assertEqual(spec, actual)

    def test_target_ignored(self):
        imported = ('__hello__', '__phello__')
        with import_helper.CleanImport(*imported, usefrozen=True):
            import __hello__ as match
            import __phello__ as nonmatch
        name = '__hello__'
        actual = self.find(name)
        for target in (None, match, nonmatch, object(), 'not-a-module-object'):
            with self.subTest(target):
                spec = self.find(name, target=target)
                self.assertEqual(spec, actual)

    def test_failure(self):
        spec = self.find('<not real>')
        self.assertIsNone(spec)

    def test_not_using_frozen(self):
        finder = self.machinery.FrozenImporter
        with import_helper.frozen_modules(enabled=False):
            # both frozen and not frozen
            spec1 = finder.find_spec('__hello__')
            # only frozen
            spec2 = finder.find_spec('__hello_only__')
        self.assertIsNone(spec1)
        self.assertIsNone(spec2)


(Frozen_FindSpecTests,
 Source_FindSpecTests
 ) = util.test_both(FindSpecTests, machinery=machinery)


class FinderTests(abc.FinderTests):

    """Test finding frozen modules."""

    def find(self, name, path=None):
        finder = self.machinery.FrozenImporter
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            with import_helper.frozen_modules():
                return finder.find_module(name, path)

    def test_module(self):
        name = '__hello__'
        loader = self.find(name)
        self.assertTrue(hasattr(loader, 'load_module'))

    def test_package(self):
        loader = self.find('__phello__')
        self.assertTrue(hasattr(loader, 'load_module'))

    def test_module_in_package(self):
        loader = self.find('__phello__.spam', ['__phello__'])
        self.assertTrue(hasattr(loader, 'load_module'))

    # No frozen package within another package to test with.
    test_package_in_package = None

    # No easy way to test.
    test_package_over_module = None

    def test_failure(self):
        loader = self.find('<not real>')
        self.assertIsNone(loader)


(Frozen_FinderTests,
 Source_FinderTests
 ) = util.test_both(FinderTests, machinery=machinery)


if __name__ == '__main__':
    unittest.main()
