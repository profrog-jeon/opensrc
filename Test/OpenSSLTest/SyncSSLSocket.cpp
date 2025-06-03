#include "pch.h"
#include "SyncSSLSocket.h"

namespace core
{
	struct ST_SSL_LIBRARY_INITIALIZER
	{
		const SSL_METHOD* pServerMethod;
		const SSL_METHOD* pClientMethod;

		ST_SSL_LIBRARY_INITIALIZER(void)
		{
			SSL_library_init();
			SSL_load_error_strings();
			OpenSSL_add_all_algorithms();
			pServerMethod = TLS_server_method();
			pClientMethod = TLS_client_method();
		}
	};
	static ST_SSL_LIBRARY_INITIALIZER g_stSSLInitializer;

	CSyncSSLSocket::CSyncSSLSocket(void)
	{
	}

	CSyncSSLSocket::~CSyncSSLSocket(void)
	{
	}

	void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
	{
		/* set the local certificate from CertFile */
		if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			abort();
		}
		/* set the private key from KeyFile (may be the same as CertFile) */
		if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			abort();
		}
		/* verify private key */
		if (!SSL_CTX_check_private_key(ctx))
		{
			fprintf(stderr, "Private key does not match the public certificate\n");
			abort();
		}
	}

	ECODE CSyncSSLSocket::Assign(SOCKET hAcceptedSocket)
	{
		ECODE nRet = EC_SUCCESS;
		try
		{
			m_pCTX = SSL_CTX_new(g_stSSLInitializer.pServerMethod);
			if (NULL == m_pCTX)
				throw exception_format("SSL_CTX_new(g_stSSLInitializer.pServerMethod) failure");

			std::string strModuleDir = ExtractDirectory(GetFileNameA());
			std::string strCertFile = strModuleDir + "/../../Build/Data/server.crt";
			std::string strKeyFile = strModuleDir + "/../../Build/Data/server.key";
			LoadCertificates(m_pCTX, (char*)strCertFile.c_str(), (char*)strKeyFile.c_str());

			m_pSSL = SSL_new(m_pCTX);
			if (NULL == m_pCTX)
				throw exception_format("SSL_new(m_pCTX) failure");

			SSL_set_fd(m_pSSL, hAcceptedSocket);

			nRet = SSL_accept(m_pSSL);
			if (nRet <= 0)
			{
				nRet = SSL_get_error(m_pSSL, nRet);
				nRet = GetLastError();
				throw exception_format("SSL_accept(m_pSSL) failure, %s", GetErrorMsg(nRet).c_str());
			}

			return CSyncTCPSocket::Assign(hAcceptedSocket);
		}
		catch (const std::exception& e)
		{
			Log_Error("%s", e.what());
			ERR_print_errors_fp(stderr);
			Close();
			return EC_INVALID_DATA;
		}
	}

	ECODE CSyncSSLSocket::Connect(const char* pszIP, WORD wPort, DWORD dwTimeOut)
	{
		ECODE nRet = EC_SUCCESS;
		try
		{
			nRet = EC_INTERNAL_ERROR;
			m_pCTX = SSL_CTX_new(g_stSSLInitializer.pClientMethod);
			if (NULL == m_pCTX)
				throw exception_format("SSL_CTX_new(g_stSSLInitializer.pClientMethod) failure");

			nRet = CSyncTCPSocket::Connect(pszIP, wPort, dwTimeOut);
			if (EC_SUCCESS != nRet)
				throw exception_format("CSyncTCPSocket::Connect(%s, %u, %u) failure, %d", pszIP, wPort, dwTimeOut, nRet);

			nRet = EC_INTERNAL_ERROR;
			m_pSSL = SSL_new(m_pCTX);
			if (NULL == m_pSSL)
				throw exception_format("SSL_new(m_pCTX) failure");

			SSL_set_fd(m_pSSL, m_hSocket);

			nRet = SSL_connect(m_pSSL);
			if (nRet <= 0)
			{
				nRet = SSL_get_error(m_pSSL, nRet);
				throw exception_format("SSL_connect(%s, %u, %u) failure", pszIP, wPort, dwTimeOut);
			}
		}
		catch (const std::exception& e)
		{
			Log_Error("%s", e.what());
			ERR_print_errors_fp(stderr);
			Close();
			return nRet;
		}

		return EC_SUCCESS;
	}

	void CSyncSSLSocket::Close(void)
	{
		if (m_pSSL)
			SSL_free(m_pSSL);
		if (m_pCTX)
			SSL_CTX_free(m_pCTX);

		m_pSSL = nullptr;
		m_pCTX = nullptr;
		CSyncTCPSocket::Close();
	}

	ECODE CSyncSSLSocket::Send(const void* pBuff, size_t tBufSize, DWORD dwTimeOut)
	{
		if (nullptr == m_pSSL)
			return EC_NOT_CREATED;

		return CSyncTCPSocket::Send(pBuff, tBufSize, dwTimeOut);
	}

	ECODE CSyncSSLSocket::Recv(void* pBuff, size_t tBufSize, DWORD dwTimeOut)
	{
		if (nullptr == m_pSSL)
			return EC_NOT_CREATED;

		return CSyncTCPSocket::Recv(pBuff, tBufSize, dwTimeOut);
	}

	ECODE CSyncSSLSocket::Peek(void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead)
	{
		if (nullptr == m_pSSL)
			return EC_NOT_CREATED;

		return CSyncTCPSocket::Peek(pBuff, tBufSize, dwTimeOut, ptRead);
	}

	ECODE CSyncSSLSocket::SendWorker(SOCKET hSocket, const void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptSent)
	{
		if (INVALID_SOCKET_ == hSocket)
			return EC_INVALID_DATA;

		int nRet = core::setsockopt(hSocket, SOL_SOCKET_, SO_SNDTIMEO_, (char*)&dwTimeOut, sizeof(dwTimeOut));
		if (SOCKET_ERROR_ == nRet)
			Log_Error("setsockopt(m_hSocket, SOL_SOCKET_, SO_SNDTIMEO, %u) failure, %s", dwTimeOut, GetErrorMsg(nRet).c_str());

		nRet = SSL_write(m_pSSL, pBuff, (int)tBufSize);
		if (nRet <= 0)
		{
			nRet = SSL_get_error(m_pSSL, nRet);
			if (SSL_ERROR_SYSCALL == nRet)
			{
#ifdef _MSC_VER
				if (10060 == core::GetLastError())	// WSAETIMEDOUT
#else
				if (11 == core::GetLastError())		// EWOULDBLOCK or EAGAN
#endif
					return EC_TIMEOUT;
			}

			Log_Error("send(0x%08X, size:%u, timeout:%u) failure, %s", hSocket, tBufSize, dwTimeOut, GetErrorMsg(nRet).c_str());
			return nRet;
		}

		if (ptSent)
			*ptSent = (size_t)nRet;
		return EC_SUCCESS;
	}

	ECODE CSyncSSLSocket::RecvWorker(SOCKET hSocket, void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead)
	{
		if (INVALID_SOCKET_ == hSocket)
			return EC_INVALID_DATA;

		int nRet = core::setsockopt(hSocket, SOL_SOCKET_, SO_RCVTIMEO_, (char*)&dwTimeOut, sizeof(dwTimeOut));
		if (SOCKET_ERROR_ == nRet)
			Log_Error("setsockopt(m_hSocket, SOL_SOCKET_, SO_RCVTIMEO_, %u) failure, %s", dwTimeOut, GetErrorMsg(nRet).c_str());

		nRet = SSL_read(m_pSSL, pBuff, (int)tBufSize);
		if (nRet <= 0)
		{
			nRet = SSL_get_error(m_pSSL, nRet);;
			if (SSL_ERROR_SYSCALL == nRet)
			{
#ifdef _MSC_VER
				if (10060 == core::GetLastError())	// WSAETIMEDOUT
#else
				if (11 == core::GetLastError())		// EWOULDBLOCK or EAGAN
#endif
					return EC_TIMEOUT;
			}

			if (SSL_ERROR_ZERO_RETURN == nRet)
				Log_Info("Connection(0x%08X) reset by remote host.", hSocket);
			else
				Log_Error("recv(0x%08X, size:%u, timeout:%u, MSG_PEEK_) failure, %d(%s)"
					, hSocket, tBufSize, dwTimeOut, GetLastError(), GetErrorMsg(nRet).c_str());
			return nRet;
		}

		if (0 == nRet)
		{
			nRet = GetLastError();
			Log_Info("Connection(0x%08X) closed by remote host.", hSocket);
			return nRet;
		}

		if (ptRead)
			*ptRead = (size_t)nRet;
		return EC_SUCCESS;
	}

	ECODE CSyncSSLSocket::PeekWorker(SOCKET hSocket, void* pBuff, size_t tBufSize, DWORD dwTimeOut, size_t* ptRead)
	{
		if (INVALID_SOCKET_ == hSocket)
			return EC_INVALID_DATA;

		int nRet = core::setsockopt(hSocket, SOL_SOCKET_, SO_RCVTIMEO_, (char*)&dwTimeOut, sizeof(dwTimeOut));
		if (SOCKET_ERROR_ == nRet)
			Log_Error("setsockopt(m_hSocket, SOL_SOCKET_, SO_RCVTIMEO_, %u) failure, %s", dwTimeOut, GetErrorMsg(nRet).c_str());

		nRet = SSL_peek(m_pSSL, pBuff, (int)tBufSize);
		if (nRet <= 0)
		{
			nRet = SSL_get_error(m_pSSL, nRet);;
			if (SSL_ERROR_SYSCALL == nRet)
			{
#ifdef _MSC_VER
				if (10060 == core::GetLastError())	// WSAETIMEDOUT
#else
				if (11 == core::GetLastError())		// EWOULDBLOCK or EAGAN
#endif
					return EC_TIMEOUT;
			}

			if (SSL_ERROR_ZERO_RETURN == nRet)
				Log_Info("Connection(0x%08X) reset by remote host.", hSocket);
			else
				Log_Error("SSL_peek(0x%08X, size:%u, timeout:%u) failure, %d(%s)"
					, hSocket, tBufSize, dwTimeOut, nRet, GetErrorMsg(nRet).c_str());
			return nRet;
		}

		if (0 == nRet)
		{
			nRet = GetLastError();
			Log_Info("Connection(0x%08X) closed by remote host.", hSocket);
			return nRet;
		}

		if (ptRead)
			*ptRead = (size_t)nRet;
		return EC_SUCCESS;
	}

	std::string CSyncSSLSocket::GetErrorMsg(int nSSLErrCode)
	{
		char szBuffer[1024] = { 0, };
		return ERR_error_string(nSSLErrCode, szBuffer);
	}
}
