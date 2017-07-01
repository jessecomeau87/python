import unittest
import sys

from test.support import import_fresh_module, run_unittest

TESTS = 'test.datetimetester'

try:
    pure_tests = import_fresh_module(TESTS, fresh=['datetime', '_strptime'],
                                     blocked=['_datetime'])
    fast_tests = import_fresh_module(TESTS, fresh=['datetime',
                                                   '_datetime', '_strptime'])
finally:
    # XXX: import_fresh_module() is supposed to leave sys.module cache untouched,
    # XXX: but it does not, so we have to cleanup ourselves.
    for modname in ['datetime', '_datetime', '_strptime']:
        sys.modules.pop(modname, None)
test_modules = [pure_tests, fast_tests]
test_suffixes = ["_Pure", "_Fast"]
# XXX(gb) First run all the _Pure tests, then all the _Fast tests.  You might
# not believe this, but in spite of all the sys.modules trickery running a _Pure
# test last will leave a mix of pure and native datetime stuff lying around.
all_test_classes = []

for module, suffix in zip(test_modules, test_suffixes):
    test_classes = []
    for name, cls in module.__dict__.items():
        if not isinstance(cls, type):
            continue
        if issubclass(cls, unittest.TestCase):
            test_classes.append(cls)
        elif issubclass(cls, unittest.TestSuite):
            suit = cls()
            test_classes.extend(type(test) for test in suit)
    for cls in test_classes:
        cls.__qualname__ += suffix
        cls._test_type = suffix[1:]
        @classmethod
        def setUpClass(cls_, module=module):
            cls_._save_sys_modules = sys.modules.copy()
            sys.modules[TESTS] = module
            sys.modules['datetime'] = module.datetime_module
            sys.modules['_strptime'] = module._strptime
        @classmethod
        def tearDownClass(cls_):
            sys.modules.clear()
            sys.modules.update(cls_._save_sys_modules)
        cls.setUpClass = setUpClass
        cls.tearDownClass = tearDownClass
    all_test_classes.extend(test_classes)

def test_main():
    run_unittest(*all_test_classes)

if __name__ == "__main__":
    test_main()
