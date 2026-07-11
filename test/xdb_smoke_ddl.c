UTEST_I(XdbTest, create_db, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;
	pRes = xdb_exec (pConn, "CREATE DATABASE xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE DATABASE xdb");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE DATABASE IF NOT EXISTS xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP DATABASE xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, drop_db, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;
	pRes = xdb_exec (pConn, "DROP DATABASE xdb");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP DATABASE IF EXISTS xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE DATABASE xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP DATABASE xdb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, create_table, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;
	pRes = xdb_exec (pConn, "CREATE TABLE teacher (id INT PRIMARY KEY, name CHAR(16), age TINYINT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE teacher (id INT PRIMARY KEY, name CHAR(16), age TINYINT)");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE IF NOT EXISTS teacher (id INT PRIMARY KEY, name CHAR(16), age TINYINT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE teacher");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, drop_table, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;
	pRes = xdb_exec (pConn, "DROP TABLE teacher");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE IF EXISTS teacher");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE teacher (id INT PRIMARY KEY, name CHAR(16), age TINYINT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE teacher");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, create_fkey, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE parent (id INT PRIMARY KEY)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE child (id INT PRIMARY KEY, pid INT, FOREIGN KEY (pid) REFERENCES parent(id))");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	// regression: xdb_create_fkey used to write through an uninitialized
	// filter pointer and crash here
	pRes = xdb_exec (pConn, "INSERT INTO parent VALUES (1)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "INSERT INTO child VALUES (1,1)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "DROP TABLE child");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE parent");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, fkey_enforcement, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE parent (id INT PRIMARY KEY)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE child (id INT PRIMARY KEY, pid INT, FOREIGN KEY (pid) REFERENCES parent(id))");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO parent VALUES (1),(2)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_affected_rows(pRes), 2);

	// insert with a non-existent parent must be rejected
	pRes = xdb_exec (pConn, "INSERT INTO child VALUES (1,99)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_E_CONSTRAINT);
	pRes = xdb_exec (pConn, "SELECT * FROM child");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_row_count(pRes), 0);
	xdb_free_result (pRes);

	// insert with a valid parent must succeed
	pRes = xdb_exec (pConn, "INSERT INTO child VALUES (1,1)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_affected_rows(pRes), 1);

	// deleting a still-referenced parent must be rejected (RESTRICT)
	pRes = xdb_exec (pConn, "DELETE FROM parent WHERE id=1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_E_CONSTRAINT);
	pRes = xdb_exec (pConn, "SELECT * FROM parent WHERE id=1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_row_count(pRes), 1);
	xdb_free_result (pRes);

	// deleting a parent with no referencing children must succeed
	pRes = xdb_exec (pConn, "DELETE FROM parent WHERE id=2");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_affected_rows(pRes), 1);

	// once the child is removed, the parent can be deleted too
	pRes = xdb_exec (pConn, "DELETE FROM child WHERE id=1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DELETE FROM parent WHERE id=1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	ASSERT_EQ (xdb_affected_rows(pRes), 1);

	pRes = xdb_exec (pConn, "DROP TABLE child");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE parent");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTestRows, create_idx, 2)
{
}

UTEST_I(XdbTestRows, drop_idx, 2)
{
}

