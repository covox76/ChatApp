#pragma once

#ifndef __AFXWIN_H__
  #error "include 'pch.h' before including this file for PCH"
#endif

#include "AppDefs.h"
#include "Resource.h"

class CClientSocket;

class CChatClientApp : public CWinApp
{
public:
  CChatClientApp();

  BOOL InitInstance() override;

  bool InitListenSocket();
  void CloseListenSocket();

  bool Connect(const CString& ipAddress, int portNumber, const CString& userName, HBITMAP userAvatar);
  bool SendText(const CString& text);
  bool Disconnect();
  void OnReceive(int dataBytesNumber, const ByteVector& data, const CString& clientIpAddress, UINT clientPort);

protected:
  DECLARE_MESSAGE_MAP()

private:
  ByteVector CreateClientMessage(MessageType type, const ByteVector& body);
  bool SendClientMessage(const ByteVector& buffer, const CString& hostIpAddress, UINT hostPortNumber);
  bool Shutdown();

  std::vector<std::pair<CString, HBITMAP>> m_chatUserInfos;
  CString m_serverIpAddress;
  UINT m_serverPortNumber = 0;
  ByteVector m_userInfoData;
  CClientSocket* m_listenSocket = nullptr;
};

extern CChatClientApp theApp;
