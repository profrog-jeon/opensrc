#pragma once

namespace core
{
	class CSyncSSLSocket : public CSyncTCPSocket
	{
		SSL_CTX* m_pCTX = nullptr;
		SSL* m_pSSL = nullptr;

	public:
		CSyncSSLSocket(void);
		~CSyncSSLSocket(void);

		ECODE Assign(SOCKET hAcceptedSocket);
		ECODE Connect(const char* pszIP, WORD wPort, DWORD dwTimeOut);
		void Close(void);

		ECODE Send(const void* pBuff, size_t tBufSize, DWORD dwTimeOut);
		ECODE Recv(void* pBuff, size_t tBufSize, DWORD dwTimeOut);
		ECODE Peek(void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead = NULL);

	private:
		ECODE SendWorker(SOCKET hSocket, const void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptSent);
		ECODE RecvWorker(SOCKET hSocket, void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead);
		ECODE PeekWorker(SOCKET hSocket, void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead);

		std::string GetErrorMsg(int nSSLErrCode);
	};
}

