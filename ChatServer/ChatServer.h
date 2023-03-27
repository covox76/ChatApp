#pragma once

#ifndef __AFXWIN_H__
  #error "include 'pch.h' before including this file for PCH"
#endif

#include "AppDefs.h"
#include "Resource.h"

class CServerSocket;

class CChatServerApp : public CWinApp
{
public:
  CChatServerApp();

  BOOL InitInstance() override;

  bool InitListenSocket(UINT portNumber);
  void CloseListenSocket();

  void OnReceive(int dataBytesNumber, const ByteVector& data, const CString& clientIpAddress, UINT clientPort);

protected:
  DECLARE_MESSAGE_MAP()

private:
  ByteVector CreateServerMessage(MessageType type, const ByteVector& body);
  bool SendServerMessage(const ByteVector& buffer, const CString& hostIpAddress, UINT hostPortNumber);
  void Shutdown();

  std::vector<ByteVector> m_chatHistory;
  std::vector<std::pair<CString, UINT>> m_clientList;
  std::vector<std::pair<CString, HBITMAP>> m_chatUserInfos;
  CServerSocket* m_listenSocket = nullptr;
};

extern CChatServerApp theApp;
