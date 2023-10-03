import os
import re
import time
import unittest


class StructSeqTest(unittest.TestCase):

    def test_tuple(self):
        t = time.gmtime()
        self.assertIsInstance(t, tuple)
        astuple = tuple(t)
        self.assertEqual(len(t), len(astuple))
        self.assertEqual(t, astuple)

        # Check that slicing works the same way; at one point, slicing t[i:j] with
        # 0 < i < j could produce NULLs in the result.
        for i in range(-len(t), len(t)):
            self.assertEqual(t[i:], astuple[i:])
            for j in range(-len(t), len(t)):
                self.assertEqual(t[i:j], astuple[i:j])

        for j in range(-len(t), len(t)):
            self.assertEqual(t[:j], astuple[:j])

        self.assertRaises(IndexError, t.__getitem__, -len(t)-1)
        self.assertRaises(IndexError, t.__getitem__, len(t))
        for i in range(-len(t), len(t)-1):
            self.assertEqual(t[i], astuple[i])

    def test_repr(self):
        t = time.gmtime()
        self.assertTrue(repr(t))
        t = time.gmtime(0)
        self.assertEqual(repr(t),
            "time.struct_time(tm_year=1970, tm_mon=1, tm_mday=1, tm_hour=0, "
            "tm_min=0, tm_sec=0, tm_wday=3, tm_yday=1, tm_isdst=0)")
        # os.stat() gives a complicated struct sequence.
        st = os.stat(__file__)
        rep = repr(st)
        self.assertTrue(rep.startswith("os.stat_result"))
        self.assertIn("st_mode=", rep)
        self.assertIn("st_ino=", rep)
        self.assertIn("st_dev=", rep)

    def test_concat(self):
        t1 = time.gmtime()
        t2 = t1 + tuple(t1)
        for i in range(len(t1)):
            self.assertEqual(t2[i], t2[i+len(t1)])

    def test_repeat(self):
        t1 = time.gmtime()
        t2 = 3 * t1
        for i in range(len(t1)):
            self.assertEqual(t2[i], t2[i+len(t1)])
            self.assertEqual(t2[i], t2[i+2*len(t1)])

    def test_contains(self):
        t1 = time.gmtime()
        for item in t1:
            self.assertIn(item, t1)
        self.assertNotIn(-42, t1)

    def test_hash(self):
        t1 = time.gmtime()
        self.assertEqual(hash(t1), hash(tuple(t1)))

    def test_cmp(self):
        t1 = time.gmtime()
        t2 = type(t1)(t1)
        self.assertEqual(t1, t2)
        self.assertTrue(not (t1 < t2))
        self.assertTrue(t1 <= t2)
        self.assertTrue(not (t1 > t2))
        self.assertTrue(t1 >= t2)
        self.assertTrue(not (t1 != t2))

    def test_fields(self):
        t = time.gmtime()
        self.assertEqual(len(t), t.n_sequence_fields)
        self.assertEqual(t.n_unnamed_fields, 0)
        self.assertEqual(t.n_fields, time._STRUCT_TM_ITEMS)

    def test_constructor(self):
        t = time.struct_time

        self.assertRaises(TypeError, t)
        self.assertRaises(TypeError, t, None)
        self.assertRaises(TypeError, t, "123")
        self.assertRaises(TypeError, t, "123", dict={})
        self.assertRaises(TypeError, t, "123456789", dict=None)

        s = "123456789"
        self.assertEqual("".join(t(s)), s)

    def test_constructor_with_duplicate_fields(self):
        t = time.struct_time
        self.assertEqual(t("123456789"), tuple("123456789"))
        self.assertEqual(t("123456789", dict={"tm_zone": "some zone"}), tuple("123456789"))
        with self.assertRaisesRegex(TypeError, "got multiple values for field 'tm_year'"):
            t("123456789", dict={"tm_year": 0})
        with self.assertRaisesRegex(TypeError, "got multiple values for field 'tm_year'"):
            t("123456789", dict={"tm_year": 0, "tm_mon": 1})
        with self.assertRaisesRegex(TypeError, "got multiple values for field 'tm_mon'"):
            t("123456789", dict={"tm_zone": "some zone", "tm_mon": 1})
        with self.assertRaisesRegex(TypeError, "got multiple values for field 'tm_mon'"):
            t("123456789", dict={"tm_zone": "some zone", "tm_mon": 1, "error": 0})
        with self.assertRaisesRegex(TypeError, "got multiple values for field 'tm_mon'"):
            t("123456789", dict={"error": 0, "tm_zone": "some zone", "tm_mon": 1})

    def test_constructor_with_duplicate_unnamed_fields(self):
        assert os.stat_result.n_unnamed_fields > 0
        n_visible_fields = os.stat_result.n_sequence_fields

        r = os.stat_result(range(n_visible_fields), {'st_atime': -1.0})
        self.assertEqual(r.st_atime, -1.0)
        self.assertEqual(r, tuple(range(n_visible_fields)))

        r = os.stat_result((*range(n_visible_fields), -1.0))
        self.assertEqual(r.st_atime, -1.0)
        self.assertEqual(r, tuple(range(n_visible_fields)))

        with self.assertRaisesRegex(TypeError, "got multiple values for field 'st_atime'"):
            os.stat_result((*range(n_visible_fields), -1.0), {'st_atime': -1.0})

    def test_constructor_with_unknown_fields(self):
        t = time.struct_time
        with self.assertRaisesRegex(TypeError,
                                    re.escape("got unexpected field name(s): {'error'}")):
            t("123456789", dict={"error": 0})
        with self.assertRaisesRegex(TypeError,
                                    re.escape("got unexpected field name(s): {'error'}")):
            t("123456789", dict={"tm_zone": "some zone", "error": 0})

    def test_eviltuple(self):
        class Exc(Exception):
            pass

        # Devious code could crash structseqs' constructors
        class C:
            def __getitem__(self, i):
                raise Exc
            def __len__(self):
                return 9

        self.assertRaises(Exc, time.struct_time, C())

    def test_reduce(self):
        t = time.gmtime()
        cls, (tup, dct) = t.__reduce__()
        self.assertIs(cls, time.struct_time)
        self.assertEqual(tup, tuple(t))
        cls(tup, dct)

    def test_reduce_with_unnamed_fields(self):
        r = os.stat_result(range(os.stat_result.n_sequence_fields), {'st_atime': 1.0})
        self.assertEqual(r.st_atime, 1.0)
        cls, (tup, dct) = r.__reduce__()
        self.assertIs(cls, os.stat_result)
        self.assertEqual(tup, tuple(r))
        self.assertIn('st_atime', dct)
        reconstructed = cls(tup, dct)
        self.assertEqual(reconstructed, r)
        self.assertAlmostEqual(reconstructed.st_atime, r.st_atime)

    def test_extended_getslice(self):
        # Test extended slicing by comparing with list slicing.
        t = time.gmtime()
        L = list(t)
        indices = (0, None, 1, 3, 19, 300, -1, -2, -31, -300)
        for start in indices:
            for stop in indices:
                # Skip step 0 (invalid)
                for step in indices[1:]:
                    self.assertEqual(list(t[start:stop:step]),
                                     L[start:stop:step])

    def test_match_args(self):
        expected_args = ('tm_year', 'tm_mon', 'tm_mday', 'tm_hour', 'tm_min',
                         'tm_sec', 'tm_wday', 'tm_yday', 'tm_isdst')
        self.assertEqual(time.struct_time.__match_args__, expected_args)

    def test_match_args_with_unnamed_fields(self):
        expected_args = ('st_mode', 'st_ino', 'st_dev', 'st_nlink', 'st_uid',
                         'st_gid', 'st_size')
        self.assertEqual(os.stat_result.n_unnamed_fields, 3)
        self.assertEqual(os.stat_result.__match_args__, expected_args)


if __name__ == "__main__":
    unittest.main()
