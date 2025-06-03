#include "pch.h"

void exit_nicely(PGconn* conn)
{
	PQfinish(conn);
	exit(1);
}

int main()
{
	PGconn* conn; /* 데이터베이스 접속 디스크럽트 구조체 */
	PGresult* res; /*질의 결과를 저장할 PGresult 구조체 포인터*/

	char* pghost = "127.0.0.1";
	char* pgport = NULL;    /* 서버 포트 */
	char* pgoptions = NULL;    /* 서버로 전달할 특별한 옵션 */
	char* pgtty = NULL;    /* 서버를 위한 디버깅 tty */

	/* 데이터베이스로 접속을 시도한다. */
	char* dbName = "akdanhall";
	conn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, 
		"akdan", "4E07C92A-E0F9-4CFC-8373-F20ADBBFE1F9");

	/* 서버와의 접속이 성공적으로 이루어졌는지 검사한다.
	* 만일 실패하였다면 에러 메시지를 출력하고 종료한다. */
	if (PQstatus(conn) == CONNECTION_BAD)
	{
		printf("Connection to database `%s` failed.\n", dbName);
		printf("%s", PQerrorMessage(conn));
		exit_nicely(conn);
	}

#ifdef DEBUG
	/* 디버깅 파일 스트림을 열고 추적을 시작한다. */
	debug = fopen("/tmp/trace.out", "w");
	PQtrace(conn, debug);
#endif      /* DEBUG */

	/* 트랜잭션 블록을 시작한다. 모든 작업은 트랜잭션 구문안에서
	* 이루어져야 한다. */
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("BEGIN command failed\n");
		PQclear(res);
		exit_nicely(conn);
	}

	/*
	* 메모리 유출을 막으려면 더 이상 필요하지 않는 PGresult를
	* PQclear 해야 한다.
	*/
	PQclear(res);

	/*
	* 데이터베이스의 시스템 카탈로그인 pg_database 클래스에서 모든
	* 데이터베이스 항목을 얻어서 커서를 선언한다. */

	res = PQexec(conn, "DECLARE myportal CURSOR FOR select *from pg_database");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("DECLARE CURSOR command failed\n");
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	/* 선언한 커서에서 데이터를 모두 불러들인다. */
	res = PQexec(conn, "FETCH ALL in myportal");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		printf("FETCH ALL command didn't return tuples properly＼n");
		PQclear(res);
		exit_nicely(conn);
	}

	/* 먼저 필드 헤더를 출력한다. */
	int nFields = PQnfields(res);
	for (int i = 0; i < nFields; i++)
	{
		printf("%-15s", PQfname(res, i));
	}
	printf("\n\n");

	/* 다음으로 인스턴스 전체를 레코드 수와 필드 수만큼 출력한다. */
	for (int i = 0; i < PQntuples(res); i++)
	{
		for (int j = 0; j < nFields; j++)
		{
			printf("%-15s", PQgetvalue(res, i, j));
		}
		printf("\n");
	}

	PQclear(res);

	/* 커서를 닫는다. 커서가 더 이상 필요없으면 커서를 닫아야 한다. */
	res = PQexec(conn, "CLOSE myportal");
	PQclear(res);

	/* 트랜잭션을 끝낸다. */
	res = PQexec(conn, "END");
	PQclear(res);

	/* 데이터베이스 접속을 종료하고 정리한다. */
	PQfinish(conn);

	exit(0);
}
TEST(PostgreSqlTest, OpenAndClose)
{

}