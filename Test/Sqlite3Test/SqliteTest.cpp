#include "pch.h"

TEST(SqliteTest, OpenAndCloseTest)
{
	std::string strSqliteFile = "test.sql";
	printf("%s\n", sqlite3_libversion());

	sqlite3* pHandle = nullptr;
	ASSERT_EQ(SQLITE_OK, sqlite3_open(strSqliteFile.c_str(), &pHandle));
	sqlite3_close(pHandle);
}
