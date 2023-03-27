#include "pch.h"
#include "ServerSocket.h"

#include "ChatServer.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CServerSocket::CServerSocket()
{
}

CServerSocket::~CServerSocket()
{
}

void CServerSocket::OnAccept(int nErrorCode)
{

  CSocket::OnAccept(nErrorCode);
}

void CServerSocket::OnClose(int nErrorCode)
{

  CSocket::OnClose(nErrorCode);
}

void CServerSocket::OnConnect(int nErrorCode)
{

  CSocket::OnConnect(nErrorCode);
}

BOOL CServerSocket::OnMessagePending()
{

  return CSocket::OnMessagePending();
}

void CServerSocket::OnOutOfBandData(int nErrorCode)
{

  CSocket::OnOutOfBandData(nErrorCode);
}

void CServerSocket::OnReceive(int nErrorCode)
{
  ByteVector buffer(SocketBufferSize);
  CString socketAddress;
  UINT socketPort = 0;

  auto res = ReceiveFrom(buffer.data(), static_cast<int>(buffer.size()), socketAddress, socketPort);

  switch (res)
  {
    case 0:
      Close();
      break;

    case SOCKET_ERROR:
    {
      auto errorCode = GetLastError();

      if (errorCode != WSAEWOULDBLOCK)
      {
        AfxMessageBox(GetLastErrorMessage(errorCode).GetString(), MB_OK | MB_ICONSTOP);
        Close();
      }

      break;
    }

    default:
      buffer.resize(static_cast<size_t>(res));

      theApp.OnReceive(res, buffer, socketAddress, socketPort);

      break;
  }

  CSocket::OnReceive(nErrorCode);
}

void CServerSocket::OnSend(int nErrorCode)
{

  CSocket::OnSend(nErrorCode);
}

int CServerSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{

  return CSocket::Receive(lpBuf, nBufLen, nFlags);
}

int CServerSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{

  return CSocket::Send(lpBuf, nBufLen, nFlags);
}
