#pragma once

class CServerSocket : public CSocket
{
public:
  CServerSocket();
  virtual ~CServerSocket();

  void OnAccept(int nErrorCode) override;
  void OnClose(int nErrorCode) override;
  void OnConnect(int nErrorCode) override;
  BOOL OnMessagePending() override;
  void OnOutOfBandData(int nErrorCode) override;
  void OnReceive(int nErrorCode) override;
  void OnSend(int nErrorCode) override;
  int Receive(void* lpBuf, int nBufLen, int nFlags = 0) override;
  int Send(const void* lpBuf, int nBufLen, int nFlags = 0) override;
};
