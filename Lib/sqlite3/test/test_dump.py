# Author: Paul Kippes <kippesp@gmail.com>

import unittest
import sqlite3 as sqlite

class DumpTests(unittest.TestCase):
    def setUp(self):
        self.cx = sqlite.connect(":memory:")
        self.cu = self.cx.cursor()

    def tearDown(self):
        self.cx.close()

    def test_table_dump(self):
        expected_sqls = [
                """CREATE TABLE "index"("index" blob);"""
                ,
                """INSERT INTO "index" VALUES(X'01');"""
                ,
                """CREATE TABLE "quoted""table"("quoted""field" text);"""
                ,
                """INSERT INTO "quoted""table" VALUES('quoted''value');"""
                ,
                "CREATE TABLE t1(id integer primary key, s1 text, " \
                "t1_i1 integer not null, i2 integer, unique (s1), " \
                "constraint t1_idx1 unique (i2));"
                ,
                "INSERT INTO \"t1\" VALUES(1,'foo',10,20);"
                ,
                "INSERT INTO \"t1\" VALUES(2,'foo2',30,30);"
                ,
                "CREATE TABLE t2(id integer, t2_i1 integer, " \
                "t2_i2 integer, primary key (id)," \
                "foreign key(t2_i1) references t1(t1_i1));"
                ,
                "CREATE TRIGGER trigger_1 update of t1_i1 on t1 " \
                "begin " \
                "update t2 set t2_i1 = new.t1_i1 where t2_i1 = old.t1_i1; " \
                "end;"
                ,
                "CREATE VIEW v1 as select * from t1 left join t2 " \
                "using (id);"
                ]
        [self.cu.execute(s) for s in expected_sqls]
        i = self.cx.iterdump()
        actual_sqls = [s for s in i]
        expected_sqls = ['BEGIN TRANSACTION;'] + expected_sqls + \
            ['COMMIT;']
        [self.assertEqual(expected_sqls[i], actual_sqls[i])
            for i in range(len(expected_sqls))]

    def test_dump_autoincrement(self):
        expected_sqls = [
            """CREATE TABLE "posts" (id int primary key);""",
            """INSERT INTO "posts" VALUES(0);""",
            """CREATE TABLE "tags" ( """
            """id integer primary key autoincrement,"""
            """tag varchar(256),"""
            """post int references posts);""",
            """INSERT INTO "tags" VALUES(NULL,'test',0);""",
            """CREATE TABLE "tags2" ( """
            """id integer primary key autoincrement,"""
            """tag varchar(256),"""
            """post int references posts);"""
        ]

        for sql_statement in expected_sqls:
            self.cu.execute(sql_statement)
        iterdump = self.cx.iterdump()
        actual_sqls = [sql_statement for sql_statement in iterdump]

        # the NULL value should now be automatically be set to 1
        expected_sqls[3] = expected_sqls[3].replace("NULL", "1")
        expected_sqls = ['BEGIN TRANSACTION;'] + expected_sqls + \
                        ['DELETE FROM "sqlite_sequence";'] + \
                        ["""INSERT INTO "sqlite_sequence" VALUES('tags',1);"""] + \
                        ['COMMIT;']

        for i in range(len(expected_sqls)):
            self.assertEqual(expected_sqls[i], actual_sqls[i])

    def test_dump_autoincrement_create_new_db(self):
        old_db = [
            """BEGIN TRANSACTION ;""",
            """CREATE TABLE "posts" (id int primary key);""",
            """INSERT INTO "posts" VALUES(0);""",
            """CREATE TABLE "tags" ( """
            """id integer primary key autoincrement,"""
            """tag varchar(256),"""
            """post int references posts);""",
            """CREATE TABLE "tags2" ( """
            """id integer primary key autoincrement,"""
            """tag varchar(256),"""
            """post int references posts);"""
        ]
        for i in range(1, 10):
            old_db.append("""INSERT INTO "tags"
                                VALUES(NULL,'test{0}',0);""".format(i))
        for i in range(1, 5):
            old_db.append("""INSERT INTO "tags2"
                                VALUES(NULL,'test{0}',0);""".format(i))
        old_db.append("COMMIT;")

        for sql_statement in old_db:
            self.cu.execute(sql_statement)

        cx2 = sqlite.connect(":memory:")
        query = "".join(line for line in self.cx.iterdump())
        cx2.executescript(query)
        cu2 = cx2.cursor()

        self.assertEqual(cu2.execute('SELECT "seq"'
                                     'FROM "sqlite_sequence"'
                                     'WHERE "name" == "tags"').fetchall()[0][0], 9)
        self.assertEqual(cu2.execute('SELECT "seq" FROM'
                                     '"sqlite_sequence"'
                                     'WHERE "name" == "tags2"').fetchall()[0][0], 4)

        cu2.close()
        cx2.close()

    def test_unorderable_row(self):
        # iterdump() should be able to cope with unorderable row types (issue #15545)
        class UnorderableRow:
            def __init__(self, cursor, row):
                self.row = row
            def __getitem__(self, index):
                return self.row[index]
        self.cx.row_factory = UnorderableRow
        CREATE_ALPHA = """CREATE TABLE "alpha" ("one");"""
        CREATE_BETA = """CREATE TABLE "beta" ("two");"""
        expected = [
            "BEGIN TRANSACTION;",
            CREATE_ALPHA,
            CREATE_BETA,
            "COMMIT;"
            ]
        self.cu.execute(CREATE_BETA)
        self.cu.execute(CREATE_ALPHA)
        got = list(self.cx.iterdump())
        self.assertEqual(expected, got)


if __name__ == "__main__":
    unittest.main()
