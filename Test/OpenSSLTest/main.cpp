#include "pch.h"
#include "SyncSSLSocket.h"

namespace core
{
	class CTestConnection : public CSyncConnection
	{
		CSyncSSLSocket m_SSLSocket;
	public:
		CTestConnection(void)
			: CSyncConnection(&m_SSLSocket)
		{}

		void OnConnect(void)
		{

		}

		void OnClose(void)
		{

		}

		void OnRecv(void)
		{
			char szBuffer[1024] = { 0, };
			m_SSLSocket.Recv(szBuffer, 1024, 6);
			printf("server-recved: %s\n", szBuffer);

			std::string strSend = "world!!";
			m_SSLSocket.Send(strSend.c_str(), strSend.size(), 60000);
		}
	};

}

int main(void)
{
	ECODE nRet = EC_SUCCESS;

	core::ST_SYNCSERVER_INIT stInit;
	stInit.wPort = 10500;
	stInit.dwRecvTimeOut = 60000;
	stInit.Connections.push_back(new core::CTestConnection());
	stInit.Connections.push_back(new core::CTestConnection());
	stInit.Connections.push_back(new core::CTestConnection());

	core::CSyncServer test;
	nRet = test.StartUp(stInit);
	core::Sleep(300);

	{
		std::string strSend = "hello?";

		core::CSyncSSLSocket client;
		nRet = client.Connect("127.0.0.1", 10500, 60000);
		nRet = client.Send(strSend.c_str(), strSend.length(), 60000);

		char szBuff[1024] = { 0, };
		nRet = client.Recv(szBuff, 7, 60000);
		printf("client-recv: %s\n", szBuff);

		client.Close();
	}
	test.ShutDown();
	return 0;
}