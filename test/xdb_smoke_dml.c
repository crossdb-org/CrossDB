UTEST_I(XdbTest, insert_one, 2)
{
	xdb_conn_t *pConn = utest_fixture->pConn;
	xdb_res_t *pRes = xdb_bexec (pConn, "INSERT INTO student (id,name,age,height,weight,class,score) VALUES ("STU2_1000")");
	CHECK_AFFECT (pRes, 1);
}

UTEST_I(XdbTestRows, insert_dup, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_bexec (pConn, "INSERT INTO student (id,name,age,height,weight,class,score) VALUES ("STU2_1000_DUP")");
	CHECK_AFFECT (pRes, 0);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1000");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),1000));	
}

UTEST_I(XdbTest, insert_many, 2)
{
	xdb_conn_t *pConn = utest_fixture->pConn;
	xdb_res_t *pRes = xdb_bexec (pConn, "INSERT INTO student (id,name,age,height,weight,class,score) VALUES ("STU2_1000"),("STU2_1001"),("STU2_1002"),("STU2_1003"),("STU2_1004"),("STU2_1005"),("STU2_1006")");
	CHECK_AFFECT (pRes, 7);
}

UTEST_I(XdbTestRows, query_one, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1000");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),1000));
}

UTEST_I(XdbTestRows, query_all, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY (pRes, 7);

	pRes = xdb_exec (pConn, "SELECT id,name,age,height,weight,class,score FROM student");
	CHECK_QUERY (pRes, 7);
}

UTEST_I(XdbTestRows, query_each, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	for (int i = 0; i < 7; ++i) {
		pRes = xdb_bexec (pConn, "SELECT * FROM student WHERE id=?", 1000 + i);
		CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),1000+i));
	}
}

UTEST_I(XdbTestRows, query_exp, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT age+5,age FROM student");
	CHECK_EXP (pRes, 7, ASSERT_EQ(xdb_column_int(pRes, pRow, 0), xdb_column_int(pRes, pRow, 1) + 5));

	pRes = xdb_exec (pConn, "SELECT age+score,age,score FROM student");
	CHECK_EXP (pRes, 7, ASSERT_EQ(xdb_column_int(pRes, pRow, 0), xdb_column_int(pRes, pRow, 1) + xdb_column_int(pRes, pRow, 2)));
}

UTEST_I(XdbTestRows, query_many_id, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1004");
	CHECK_QUERY (pRes, 2, ASSERT_GT(STU_ID(pRow),1004));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>=1004");
	CHECK_QUERY (pRes, 3, ASSERT_GE(STU_ID(pRow),1004));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id<1004");
	CHECK_QUERY (pRes, 4, ASSERT_LT(STU_ID(pRow),1004));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id<=1004");
	CHECK_QUERY (pRes, 5, ASSERT_LE(STU_ID(pRow),1004));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id!=1004");
	CHECK_QUERY (pRes, 6, ASSERT_NE(STU_ID(pRow),1004));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id<>1004");
	CHECK_QUERY (pRes, 6, ASSERT_NE(STU_ID(pRow),1004));
}

UTEST_I(XdbTestRows, query_or, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1004 OR name='jack'");
	CHECK_QUERY (pRes, 4, ASSERT_TRUE ((STU_ID(pRow)>1004) || !strcmp(STU_NAME(pRow),"jack")));
}

UTEST_I(XdbTestRows, query_limit, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student LIMIT 2");
	CHECK_QUERY (pRes, 2);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GT(STU_ID(pRow),1003));
}

UTEST_I(XdbTestRows, query_order, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age");
	CHECK_QUERY (pRes, 7, ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age");
	CHECK_QUERY (pRes, 3, ASSERT_GT(STU_ID(pRow),1003); ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age ASC");
	CHECK_QUERY (pRes, 7, ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age ASC");
	CHECK_QUERY (pRes, 3, ASSERT_GT(STU_ID(pRow),1003); ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 100;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age DESC");
	CHECK_QUERY (pRes, 7, ASSERT_LE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 100;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age DESC");
	CHECK_QUERY (pRes, 3, ASSERT_GT(STU_ID(pRow),1003); ASSERT_LE(STU_AGE(pRow),age); age=STU_AGE(pRow));
}

UTEST_I(XdbTestRows, query_order_limit, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GT(STU_ID(pRow),1003); ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age ASC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 0;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age ASC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GT(STU_ID(pRow),1003); ASSERT_GE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 100;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY age DESC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_LE(STU_AGE(pRow),age); age=STU_AGE(pRow));

	age = 100;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id>1003 ORDER BY age DESC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_GT(STU_ID(pRow),1003); ASSERT_LE(STU_AGE(pRow),age); age=STU_AGE(pRow));
}

UTEST_I(XdbTestRows, query_page, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int id = 1000;
	pRes = xdb_exec (pConn, "SELECT * FROM student LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id++));

	pRes = xdb_exec (pConn, "SELECT * FROM student LIMIT 2 OFFSET 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id++));

	pRes = xdb_exec (pConn, "SELECT * FROM student LIMIT 4,2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id++));

	pRes = xdb_exec (pConn, "SELECT * FROM student LIMIT 2 OFFSET 6");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),id++));
}

UTEST_I(XdbTestRows, query_where_page, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int id = 1002;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id++));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 LIMIT 2 OFFSET 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id++));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 LIMIT 4,2");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),id++));
}

UTEST_I(XdbTestRows, query_where_order_page, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int id = 1006;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 ORDER BY id DESC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id--));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 ORDER BY id DESC LIMIT 2 OFFSET 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id--));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id > 1001 ORDER BY id DESC LIMIT 4,2");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),id--));
}

UTEST_I(XdbTestRows, query_order_page, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	int id = 1006;
	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY id DESC LIMIT 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id--));

	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY id DESC LIMIT 2 OFFSET 2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id--));

	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY id DESC LIMIT 4,2");
	CHECK_QUERY (pRes, 2, ASSERT_EQ(STU_ID(pRow),id--));

	pRes = xdb_exec (pConn, "SELECT * FROM student ORDER BY id DESC LIMIT 2 OFFSET 6");
	CHECK_QUERY (pRes, 1, ASSERT_EQ(STU_ID(pRow),id--));
}

UTEST_I(XdbTestRows, query_many_name, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name!='jack'");
	CHECK_QUERY (pRes, 4, ASSERT_STRNE(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name<>'jack'");
	CHECK_QUERY (pRes, 4, ASSERT_STRNE(STU_NAME(pRow),"jack"));
}

UTEST_I(XdbTestRows, query_many_composite, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack' AND age>10");
	CHECK_QUERY (pRes, 2, ASSERT_STREQ(STU_NAME(pRow),"jack"); ASSERT_GT(STU_AGE(pRow),10));
}

UTEST_I(XdbTestRows, query_like, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE 'jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE '%ck'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE 'j_ck'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE 'ja%'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(STU_NAME(pRow),"jack"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE 'w%d_'");
	CHECK_QUERY (pRes, 1, ASSERT_STREQ(STU_NAME(pRow),"wendy"));

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name LIKE 'abc'");
	CHECK_QUERY (pRes, 0);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE class LIKE '6-_'");
	CHECK_QUERY (pRes, 7);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE class LIKE '_-2'");
	CHECK_QUERY (pRes, 2);
}

UTEST_I(XdbTestRows, query_miss, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_bexec (pConn, "SELECT * FROM student WHERE id=?", 1100);
	CHECK_QUERY (pRes, 0);

	pRes = xdb_pexec (pConn, "SELECT * FROM student WHERE id=%d", 1100);
	CHECK_QUERY (pRes, 0);
}

UTEST_I(XdbTestRows, update_one, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET age=age+1 WHERE id=1001");
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1001");
	CHECK_QUERY(pRes, 1, stu.age += 1);

	// other rows are not affected
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id!=1001");
	CHECK_QUERY(pRes, 6);
}

UTEST_I(XdbTestRows, update_exp, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET score=score+age WHERE id=1001");
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1001");
	CHECK_QUERY(pRes, 1, stu.score += stu.age);

	// other rows are not affected
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id!=1001");
	CHECK_QUERY(pRes, 6);
}

UTEST_I(XdbTestRows, update_one_pstmt, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_bexec (pConn, "UPDATE student SET age=age+? WHERE id=?", 1, 1001);
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_bexec (pConn, "SELECT * FROM student WHERE id=?", 1001);
	CHECK_QUERY(pRes, 1, stu.age += 1);

	pRes = xdb_bexec (pConn, "SELECT * FROM student WHERE id!=?", 1001);
	CHECK_QUERY(pRes, 6);
}

UTEST_I(XdbTestRows, update_one_fmt, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_pexec (pConn, "UPDATE student SET age=age+%d WHERE id=%d", 1, 1001);
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_pexec (pConn, "SELECT * FROM student WHERE id=%d", 1001);
	CHECK_QUERY(pRes, 1, stu.age += 1);

	pRes = xdb_pexec (pConn, "SELECT * FROM student WHERE id!=%d", 1001);
	CHECK_QUERY(pRes, 6);
}

UTEST_I(XdbTestRows, update_char, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_pexec (pConn, "UPDATE student SET name='%s', class='%s' WHERE name='%s'", "jackson", "6-7", "jack");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_pexec (pConn, "SELECT * FROM student WHERE name='%s'", "jackson");
	CHECK_QUERY(pRes, 3, stu.name="jackson"; stu.cls="6-7";);

	pRes = xdb_bexec (pConn, "UPDATE student SET name=?, class=? WHERE name=?", "jack", "6-2", "jackson");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_bexec (pConn, "SELECT * FROM student WHERE name=?", "jack");
	CHECK_QUERY(pRes, 3, stu.cls="6-2";);
}

UTEST_I(XdbTestRows, update_pk, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET id=1101 WHERE id=1001");
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1001");
	CHECK_QUERY (pRes, 0);	

	student_t stu = stu_info[1];
	stu.id = 1101;
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id=1101");
	CHECK_QUERY_ONE(pRes, stu);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE id!=1101");
	CHECK_QUERY (pRes, 6);	
}

UTEST_I(XdbTestRows, update_pk_dup, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET id=1002 WHERE id=1001");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY (pRes, 7);	
}

UTEST_I(XdbTestRows, update_all, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET age=age+1");
	CHECK_AFFECT (pRes, 7);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY(pRes, 7, stu.age += 1);
}

UTEST_I(XdbTestRows, update_many, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET age=age+1 WHERE name='jack'");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY(pRes, 3, stu.age += 1);
	// others are not affected
	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name!='jack'");
	CHECK_QUERY(pRes, 4);
}

UTEST_I(XdbTestRows, update_miss, 2)
{
	int len;
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "UPDATE student SET age=age+1 WHERE id=1101");
	CHECK_AFFECT (pRes, 0);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY (pRes, 7);	
}

UTEST_I(XdbTestRows, delete_one, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "DELETE FROM student WHERE id=1001");
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY(pRes, 6);
}

UTEST_I(XdbTestRows, delete_many, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_bexec (pConn, "DELETE FROM student WHERE name=?", "jack");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY(pRes, 4);
}

UTEST_I(XdbTestRows, delete_all, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "DELETE FROM student");
	CHECK_AFFECT (pRes, 7);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY(pRes, 0);
}

UTEST_I(XdbTestRows, delete_miss, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_pexec (pConn, "DELETE FROM student WHERE id=%d", 1101);
	CHECK_AFFECT (pRes, 0);

	pRes = xdb_exec (pConn, "SELECT * FROM student");
	CHECK_QUERY(pRes, 7);
}

UTEST_I(XdbTestRows, idx_query, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(stu.name, "jack"));

	pRes = xdb_exec (pConn, "CREATE INDEX idx_name ON student (name)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(stu.name, "jack"));

	pRes = xdb_exec (pConn, "DROP INDEX idx_name ON student");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(stu.name, "jack"));
}

UTEST_I(XdbTestRows, idx_query_update, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(stu.name, "jack"));

	pRes = xdb_exec (pConn, "CREATE INDEX idx_name ON student (name)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 3, ASSERT_STREQ(stu.name, "jack"));

	pRes = xdb_exec (pConn, "UPDATE student SET name='jackson' WHERE name='jack'");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack'");
	CHECK_QUERY (pRes, 0);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jackson'");
	CHECK_QUERY (pRes, 3, stu.name="jackson");

	pRes = xdb_exec (pConn, "DELETE FROM student WHERE name='jackson' LIMIT 1");
	CHECK_AFFECT (pRes, 1);

	pRes = xdb_exec (pConn, "DROP INDEX idx_name ON student");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jackson'");
	CHECK_QUERY (pRes, 2, stu.name="jackson");
}

UTEST_I(XdbTestRows, idx_composite_query, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack' AND age=11");
	CHECK_QUERY (pRes, 2, ASSERT_STREQ(stu.name, "jack"); ASSERT_EQ(stu.age, 11));

	pRes = xdb_exec (pConn, "CREATE INDEX idx_nameage ON student (name,age)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack' AND age=11");
	CHECK_QUERY (pRes, 2, ASSERT_STREQ(stu.name, "jack"); ASSERT_EQ(stu.age, 11));

	pRes = xdb_exec (pConn, "DROP INDEX idx_nameage ON student");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM student WHERE name='jack' AND age=11");
	CHECK_QUERY (pRes, 2, ASSERT_STREQ(stu.name, "jack"); ASSERT_EQ(stu.age, 11));
}

UTEST_I(XdbTest, join_two_tables, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE dept (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE emp (id INT PRIMARY KEY, dept_id INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO dept VALUES (1,100),(2,200)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO emp VALUES (1,1,11),(2,1,22),(3,2,33)");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_exec (pConn, "SELECT * FROM dept JOIN emp ON dept.id = emp.dept_id");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 3);
	int sum_dept_val = 0, sum_emp_val = 0;
	int count = 0;
	while ((pRow = xdb_fetch_row (pRes)) != NULL) {
		sum_dept_val += xdb_column_int (pRes, pRow, 1);
		sum_emp_val  += xdb_column_int (pRes, pRow, 4);
		count++;
	}
	ASSERT_EQ (count, 3);
	ASSERT_EQ (sum_dept_val, 100+100+200); // dept 1 joined twice, dept 2 once
	ASSERT_EQ (sum_emp_val, 11+22+33);
	xdb_free_result (pRes);

	// no matching rows
	pRes = xdb_exec (pConn, "DELETE FROM emp");
	CHECK_AFFECT (pRes, 3);
	pRes = xdb_exec (pConn, "SELECT * FROM dept JOIN emp ON dept.id = emp.dept_id");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 0);
	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE emp");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE dept");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, join_three_tables, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE ta (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE tb (id INT PRIMARY KEY, aid INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE tc (id INT PRIMARY KEY, bid INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO ta VALUES (1,100),(2,200)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO tb VALUES (1,1,11),(2,2,22)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO tc VALUES (1,1,111),(2,1,222),(3,2,333)");
	CHECK_AFFECT (pRes, 3);

	pRes = xdb_exec (pConn, "SELECT * FROM ta JOIN tb ON ta.id = tb.aid JOIN tc ON tb.id = tc.bid");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 3);
	int count = 0;
	while ((pRow = xdb_fetch_row (pRes)) != NULL) {
		int a_val = xdb_column_int (pRes, pRow, 1);
		int b_id  = xdb_column_int (pRes, pRow, 2);
		int b_aid = xdb_column_int (pRes, pRow, 3);
		int c_bid = xdb_column_int (pRes, pRow, 6);
		ASSERT_EQ (b_aid, (a_val == 100) ? 1 : 2);
		ASSERT_EQ (c_bid, b_id);
		count++;
	}
	ASSERT_EQ (count, 3);
	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE tc");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE tb");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE ta");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, join_varchar_unsupported, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE jt1 (id INT PRIMARY KEY, name VARCHAR(32))");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE jt2 (id INT PRIMARY KEY, t1id INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	// JOIN with a VARCHAR column must fail cleanly (not crash) until supported
	pRes = xdb_exec (pConn, "SELECT * FROM jt1 JOIN jt2 ON jt1.id = jt2.t1id");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "DROP TABLE jt2");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE jt1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, join_table_alias, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE dept (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE emp (id INT PRIMARY KEY, dept_id INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO dept VALUES (1,100),(2,200)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO emp VALUES (1,1,11),(2,1,22),(3,2,33)");
	CHECK_AFFECT (pRes, 3);

	// JOIN using aliases
	pRes = xdb_exec (pConn, "SELECT * FROM dept AS d JOIN emp AS e ON d.id = e.dept_id");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 3);
	xdb_free_result (pRes);

	// WHERE filtering on an aliased table
	pRes = xdb_exec (pConn, "SELECT * FROM dept AS d JOIN emp AS e ON d.id = e.dept_id WHERE e.val > 20");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 2);
	while ((pRow = xdb_fetch_row (pRes)) != NULL) {
		ASSERT_TRUE (xdb_column_int (pRes, pRow, 4) > 20);
	}
	xdb_free_result (pRes);

	// once aliased, the real table name must no longer resolve
	pRes = xdb_exec (pConn, "SELECT * FROM dept AS d JOIN emp AS e ON dept.id = e.dept_id");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "DROP TABLE emp");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE dept");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

UTEST_I(XdbTest, join_self, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE emp (id INT PRIMARY KEY, mgr_id INT, salary INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	// id=1 is the top manager (mgr_id=0, no manager row matches it)
	pRes = xdb_exec (pConn, "INSERT INTO emp VALUES (1,0,1000),(2,1,800),(3,1,850),(4,2,600)");
	CHECK_AFFECT (pRes, 4);

	// self-join: each employee with their manager's row
	pRes = xdb_exec (pConn, "SELECT * FROM emp AS e JOIN emp AS m ON e.mgr_id = m.id");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 3); // id=1 has no matching manager row, excluded
	int count = 0;
	while ((pRow = xdb_fetch_row (pRes)) != NULL) {
		int e_mgr_id = xdb_column_int (pRes, pRow, 1);
		int m_id     = xdb_column_int (pRes, pRow, 3);
		ASSERT_EQ (e_mgr_id, m_id);
		count++;
	}
	ASSERT_EQ (count, 3);
	xdb_free_result (pRes);

	// self-join combined with a WHERE filter on one side
	pRes = xdb_exec (pConn, "SELECT * FROM emp AS e JOIN emp AS m ON e.mgr_id = m.id WHERE e.salary > 700");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 2); // ids 2 and 3, both salary > 700
	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE emp");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

// T2: same JOIN as join_two_tables, but with an index on the join column,
// to confirm an available index doesn't change correctness (it just isn't
// used for the join scan itself yet)
UTEST_I(XdbTest, join_indexed_column, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE dept (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE emp (id INT PRIMARY KEY, dept_id INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE INDEX idx_dept_id ON emp (dept_id)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO dept VALUES (1,100),(2,200)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO emp VALUES (1,1,11),(2,1,22),(3,2,33),(4,9,999)");
	CHECK_AFFECT (pRes, 4);

	pRes = xdb_exec (pConn, "SELECT * FROM dept JOIN emp ON dept.id = emp.dept_id");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 3); // dept_id=9 has no matching dept, dept id=2 has no matching emp beyond id=3
	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE emp");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE dept");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

// T3: a 3rd table's ON clause referencing the 1st table (not the immediately
// preceding one) is rejected with a clean error, not a wrong/silent join
UTEST_I(XdbTest, join_non_chain_rejected, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE a (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE b (id INT PRIMARY KEY, aid INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE c (id INT PRIMARY KEY, aid INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	// c joins against a (the 1st table), not b (the immediately preceding one)
	pRes = xdb_exec (pConn, "SELECT * FROM a JOIN b ON a.id = b.aid JOIN c ON a.id = c.aid");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);

	// the equivalent proper chain (c joins against b) must still work
	pRes = xdb_exec (pConn, "CREATE TABLE d (id INT PRIMARY KEY, bid INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "SELECT * FROM a JOIN b ON a.id = b.aid JOIN d ON b.id = d.bid");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE d");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE c");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE b");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE a");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

// T4: WHERE on a joined (non-base) column combined with ORDER BY and LIMIT.
// This is the row/metadata coherence check: xdb_sql_orderby/xdb_sql_limit
// must treat each JOIN result as a whole batch of reftbl_count row pointers,
// not a single pointer, or rows get scrambled/corrupted.
UTEST_I(XdbTest, join_where_order_limit, 2)
{
	xdb_res_t *pRes;
	xdb_row_t *pRow;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE t1 (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE t2 (id INT PRIMARY KEY, t1id INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO t1 VALUES (1,100),(2,200)");
	CHECK_AFFECT (pRes, 2);
	pRes = xdb_exec (pConn, "INSERT INTO t2 VALUES (1,1,50),(2,1,10),(3,1,90),(4,2,30),(5,2,70)");
	CHECK_AFFECT (pRes, 5);

	// val>20 excludes t2.id=2 (val=10); remaining sorted asc by t2.val: 30,50,70,90
	// LIMIT 2 -> t2.id=4 (val=30,t1id=2) then t2.id=1 (val=50,t1id=1)
	pRes = xdb_exec (pConn, "SELECT * FROM t1 JOIN t2 ON t1.id = t2.t1id WHERE t2.val > 20 ORDER BY t2.val LIMIT 2");
	ASSERT_EQ_MSG (xdb_errcode(pRes), XDB_OK, xdb_errmsg(pRes));
	ASSERT_EQ (xdb_row_count(pRes), 2);

	pRow = xdb_fetch_row (pRes);
	ASSERT_TRUE (pRow != NULL);
	ASSERT_EQ (xdb_column_int (pRes, pRow, 0), 2);   // t1.id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 1), 200); // t1.val
	ASSERT_EQ (xdb_column_int (pRes, pRow, 2), 4);   // t2.id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 3), 2);   // t2.t1id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 4), 30);  // t2.val

	pRow = xdb_fetch_row (pRes);
	ASSERT_TRUE (pRow != NULL);
	ASSERT_EQ (xdb_column_int (pRes, pRow, 0), 1);   // t1.id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 1), 100); // t1.val
	ASSERT_EQ (xdb_column_int (pRes, pRow, 2), 1);   // t2.id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 3), 1);   // t2.t1id
	ASSERT_EQ (xdb_column_int (pRes, pRow, 4), 50);  // t2.val

	xdb_free_result (pRes);

	pRes = xdb_exec (pConn, "DROP TABLE t2");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE t1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}

// T5: an unqualified column name that exists in more than one joined table
// must be rejected as ambiguous, not silently resolved to the wrong one
UTEST_I(XdbTest, join_ambiguous_column, 2)
{
	xdb_res_t *pRes;
	xdb_conn_t *pConn = utest_fixture->pConn;

	pRes = xdb_exec (pConn, "CREATE TABLE t1 (id INT PRIMARY KEY, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "CREATE TABLE t2 (id INT PRIMARY KEY, t1id INT, val INT)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "INSERT INTO t1 VALUES (1,100)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "INSERT INTO t2 VALUES (1,1,50)");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "SELECT * FROM t1 JOIN t2 ON t1.id = t2.t1id WHERE val > 20");
	ASSERT_NE (xdb_errcode(pRes), XDB_OK);

	pRes = xdb_exec (pConn, "DROP TABLE t2");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
	pRes = xdb_exec (pConn, "DROP TABLE t1");
	ASSERT_EQ (xdb_errcode(pRes), XDB_OK);
}
