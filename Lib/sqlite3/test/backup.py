import os
import sqlite3 as sqlite
import unittest

from test import support


@unittest.skipIf(sqlite.sqlite_version_info < (3, 6, 11), "Backup API not supported")
class BackupTests(unittest.TestCase):
    def setUp(self):
        cx = self.cx = sqlite.connect(":memory:")
        cx.execute('CREATE TABLE foo (key INTEGER)')
        cx.executemany('INSERT INTO foo (key) VALUES (?)', [(3,), (4,)])
        cx.commit()
        os.mkdir(support.TESTFN)
        self.addCleanup(support.rmtree, support.TESTFN)
        self.temp_counter = 0

    @property
    def temp_file_name(self):
        self.temp_counter += 1
        return os.path.join(support.TESTFN, 'bcktest_%d' % self.temp_counter)

    def tearDown(self):
        self.cx.close()

    def verify_backup(self, bckfn):
        cx = sqlite.connect(bckfn)
        result = cx.execute("SELECT key FROM foo ORDER BY key").fetchall()
        self.assertEqual(result[0][0], 3)
        self.assertEqual(result[1][0], 4)

    def test_bad_target(self):
        with self.assertRaises(TypeError):
            self.cx.backup(None)

    def test_keyword_only_args(self):
        with self.assertRaises(TypeError):
            self.cx.backup('foo', 1)

    def test_simple(self):
        bckfn = self.temp_file_name
        self.cx.backup(bckfn)
        self.verify_backup(bckfn)

    def test_progress(self):
        journal = []

        def progress(status, remaining, total):
            journal.append(status)

        bckfn = self.temp_file_name
        self.cx.backup(bckfn, pages=1, progress=progress)
        self.verify_backup(bckfn)

        self.assertEqual(len(journal), 2)
        self.assertEqual(journal[0], sqlite.SQLITE_OK)
        self.assertEqual(journal[1], sqlite.SQLITE_DONE)

    def test_progress_all_pages_at_once_1(self):
        journal = []

        def progress(status, remaining, total):
            journal.append(remaining)

        bckfn = self.temp_file_name
        self.cx.backup(bckfn, progress=progress)
        self.verify_backup(bckfn)

        self.assertEqual(len(journal), 1)
        self.assertEqual(journal[0], 0)

    def test_progress_all_pages_at_once_2(self):
        journal = []

        def progress(status, remaining, total):
            journal.append(remaining)

        bckfn = self.temp_file_name
        self.cx.backup(bckfn, pages=-1, progress=progress)
        self.verify_backup(bckfn)

        self.assertEqual(len(journal), 1)
        self.assertEqual(journal[0], 0)

    def test_non_callable_progress(self):
        bckfn = self.temp_file_name
        with self.assertRaises(TypeError) as err:
            self.cx.backup(bckfn, pages=1, progress='bar')
        self.assertEqual(str(err.exception), 'progress argument must be a callable')

    def test_modifying_progress(self):
        journal = []

        def progress(status, remaining, total):
            if not journal:
                self.cx.execute('INSERT INTO foo (key) VALUES (?)', (remaining+1000,))
                self.cx.commit()
            journal.append(remaining)

        bckfn = self.temp_file_name
        self.cx.backup(bckfn, pages=1, progress=progress)
        self.verify_backup(bckfn)

        cx = sqlite.connect(bckfn)
        result = cx.execute("SELECT key FROM foo"
                            " WHERE key >= 1000"
                            " ORDER BY key").fetchall()
        self.assertEqual(result[0][0], 1001)

        self.assertEqual(len(journal), 3)
        self.assertEqual(journal[0], 1)
        self.assertEqual(journal[1], 1)
        self.assertEqual(journal[2], 0)

    def test_failing_progress(self):
        def progress(status, remaining, total):
            raise SystemError('nearly out of space')

        bckfn = self.temp_file_name
        with self.assertRaises(SystemError) as err:
            self.cx.backup(bckfn, progress=progress)
        self.assertEqual(str(err.exception), 'nearly out of space')
        self.assertFalse(os.path.exists(bckfn))

    def test_database_source_name(self):
        bckfn = self.temp_file_name
        self.cx.backup(bckfn, name='main')
        self.cx.backup(bckfn, name='temp')
        with self.assertRaises(sqlite.OperationalError):
            self.cx.backup(bckfn, name='non-existing')
        self.assertFalse(os.path.exists(bckfn))
        self.cx.execute("ATTACH DATABASE ':memory:' AS attached_db")
        self.cx.execute('CREATE TABLE attached_db.foo (key INTEGER)')
        self.cx.executemany('INSERT INTO attached_db.foo (key) VALUES (?)', [(3,), (4,)])
        self.cx.commit()
        bckfn = self.temp_file_name
        self.cx.backup(bckfn, name='attached_db')
        self.verify_backup(bckfn)

    def test_backup_to_other_connection(self):
        dx = sqlite.connect(':memory:')
        self.cx.backup(dx)
        result = dx.execute("SELECT key FROM foo ORDER BY key").fetchall()
        self.assertEqual(result[0][0], 3)
        self.assertEqual(result[1][0], 4)

    def test_backup_to_other_connection_in_transaction(self):
        dx = sqlite.connect(':memory:')
        dx.execute('CREATE TABLE bar (key INTEGER)')
        dx.executemany('INSERT INTO bar (key) VALUES (?)', [(3,), (4,)])
        try:
            with self.assertRaises(sqlite.OperationalError):
                self.cx.backup(dx)
        finally:
            dx.rollback()

def suite():
    return unittest.TestSuite(unittest.makeSuite(BackupTests, "test_"))

def test():
    runner = unittest.TextTestRunner()
    runner.run(suite())

if __name__ == "__main__":
    test()
