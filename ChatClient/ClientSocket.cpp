#include "pch.h"
#include "ClientSocket.h"

#include "ChatClient.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CClientSocket::CClientSocket()
{
}

CClientSocket::~CClientSocket()
{
}

void CClientSocket::OnAccept(int nErrorCode)
{

  CSocket::OnAccept(nErrorCode);
}

void CClientSocket::OnClose(int nErrorCode)
{

  CSocket::OnClose(nErrorCode);
}

void CClientSocket::OnConnect(int nErrorCode)
{

  CSocket::OnConnect(nErrorCode);
}

BOOL CClientSocket::OnMessagePending()
{

  return CSocket::OnMessagePending();
}

void CClientSocket::OnOutOfBandData(int nErrorCode)
{

  CSocket::OnOutOfBandData(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode)
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

void CClientSocket::OnSend(int nErrorCode)
{

  CSocket::OnSend(nErrorCode);
}

int CClientSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{

  return CSocket::Receive(lpBuf, nBufLen, nFlags);
}

int CClientSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{

  return CSocket::Send(lpBuf, nBufLen, nFlags);
}
