#include "pch.h"

void exit_nicely(PGconn* conn)
{
	PQfinish(conn);
	exit(1);
}

int main()
{
	PGconn* conn; /* �����ͺ��̽� ���� ��ũ��Ʈ ����ü */
	PGresult* res; /*���� ����� ������ PGresult ����ü ������*/

	char* pghost = "127.0.0.1";
	char* pgport = NULL;    /* ���� ��Ʈ */
	char* pgoptions = NULL;    /* ������ ������ Ư���� �ɼ� */
	char* pgtty = NULL;    /* ������ ���� ����� tty */

	/* �����ͺ��̽��� ������ �õ��Ѵ�. */
	char* dbName = "akdanhall";
	conn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, 
		"akdan", "4E07C92A-E0F9-4CFC-8373-F20ADBBFE1F9");

	/* �������� ������ ���������� �̷�������� �˻��Ѵ�.
	* ���� �����Ͽ��ٸ� ���� �޽����� ����ϰ� �����Ѵ�. */
	if (PQstatus(conn) == CONNECTION_BAD)
	{
		printf("Connection to database `%s` failed.\n", dbName);
		printf("%s", PQerrorMessage(conn));
		exit_nicely(conn);
	}

#ifdef DEBUG
	/* ����� ���� ��Ʈ���� ���� ������ �����Ѵ�. */
	debug = fopen("/tmp/trace.out", "w");
	PQtrace(conn, debug);
#endif      /* DEBUG */

	/* Ʈ����� ����� �����Ѵ�. ��� �۾��� Ʈ����� �����ȿ���
	* �̷������ �Ѵ�. */
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("BEGIN command failed\n");
		PQclear(res);
		exit_nicely(conn);
	}

	/*
	* �޸� ������ �������� �� �̻� �ʿ����� �ʴ� PGresult��
	* PQclear �ؾ� �Ѵ�.
	*/
	PQclear(res);

	/*
	* �����ͺ��̽��� �ý��� īŻ�α��� pg_database Ŭ�������� ���
	* �����ͺ��̽� �׸��� �� Ŀ���� �����Ѵ�. */

	res = PQexec(conn, "DECLARE myportal CURSOR FOR select *from pg_database");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("DECLARE CURSOR command failed\n");
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	/* ������ Ŀ������ �����͸� ��� �ҷ����δ�. */
	res = PQexec(conn, "FETCH ALL in myportal");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		printf("FETCH ALL command didn't return tuples properly��n");
		PQclear(res);
		exit_nicely(conn);
	}

	/* ���� �ʵ� ����� ����Ѵ�. */
	int nFields = PQnfields(res);
	for (int i = 0; i < nFields; i++)
	{
		printf("%-15s", PQfname(res, i));
	}
	printf("\n\n");

	/* �������� �ν��Ͻ� ��ü�� ���ڵ� ���� �ʵ� ����ŭ ����Ѵ�. */
	for (int i = 0; i < PQntuples(res); i++)
	{
		for (int j = 0; j < nFields; j++)
		{
			printf("%-15s", PQgetvalue(res, i, j));
		}
		printf("\n");
	}

	PQclear(res);

	/* Ŀ���� �ݴ´�. Ŀ���� �� �̻� �ʿ������ Ŀ���� �ݾƾ� �Ѵ�. */
	res = PQexec(conn, "CLOSE myportal");
	PQclear(res);

	/* Ʈ������� ������. */
	res = PQexec(conn, "END");
	PQclear(res);

	/* �����ͺ��̽� ������ �����ϰ� �����Ѵ�. */
	PQfinish(conn);

	exit(0);
}
TEST(PostgreSqlTest, OpenAndClose)
{

}