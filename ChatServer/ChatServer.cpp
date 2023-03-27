#include "pch.h"
#include "ChatServer.h"

#include "ChatServerDlg.h"
#include "ServerSocket.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CChatServerApp, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CChatServerApp::CChatServerApp()
{
}

CChatServerApp theApp;

BOOL CChatServerApp::InitInstance()
{
  INITCOMMONCONTROLSEX InitCtrls = { 0 };
  InitCtrls.dwSize = sizeof(InitCtrls);
  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  CWinApp::InitInstance();

  if (!AfxSocketInit())
  {
    AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
    return FALSE;
  }

  auto shellManager = new CShellManager;

  CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

  SetRegistryKey(_T("Local AppWizard-Generated Applications"));

  CChatServerDlg dlg;

  m_pMainWnd = &dlg;

  auto response = dlg.DoModal();

  if (response == IDOK)
  {
  }
  else if (response == IDCANCEL)
  {
  }
  else if (response == -1)
  {
    TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
    TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
  }

  CloseListenSocket();

  if (shellManager != nullptr)
    delete shellManager;

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
  ControlBarCleanUp();
#endif

  return FALSE;
}

// Initialize server socket for listening on specific port (only the first instance of the program will succeed)
bool CChatServerApp::InitListenSocket(UINT portNumber)
{
  CloseListenSocket();

  m_listenSocket = new CServerSocket();

  auto result = m_listenSocket->Create(portNumber, SOCK_DGRAM, NULL) ? true : false;

  if (result == false)
  {
    CloseListenSocket();

    AfxMessageBox(GetLastErrorMessage().GetString(), MB_OK | MB_ICONSTOP);
  }

  return result;
}

void CChatServerApp::CloseListenSocket()
{
  if (m_listenSocket != nullptr)
  {
    Shutdown();

    delete m_listenSocket;

    m_listenSocket = nullptr;
  }
}

ByteVector CChatServerApp::CreateServerMessage(MessageType type, const ByteVector& body)
{
  ByteVector data;
  TwoBytesInteger dataLength = { 0 };

  dataLength.value = static_cast<std::uint16_t>(body.size());

  // Message header contains message type and message body size
  data.push_back(type);
  data.push_back(dataLength.bytes[0]);
  data.push_back(dataLength.bytes[1]);

  // Append message body if exists
  if (body.empty() == false)
    data.insert(data.end(), body.begin(), body.end());

  return data;
}

bool CChatServerApp::SendServerMessage(const ByteVector& buffer, const CString& hostIpAddress, UINT hostPortNumber)
{
  auto result = false;

  if (m_listenSocket && m_listenSocket->m_hSocket != INVALID_SOCKET)
  {
    auto sendRes = m_listenSocket->SendTo(buffer.data(), static_cast<int>(buffer.size()), hostPortNumber, hostIpAddress.GetString());

    if (sendRes == SOCKET_ERROR)
      AfxMessageBox(GetLastErrorMessage().GetString(), MB_OK | MB_ICONSTOP);
    else
      result = true;
  }

  return result;
}

void CChatServerApp::Shutdown()
{
  ByteVector dummyData;

  // sending to all clients
  for (auto& [ipAddress, port] : m_clientList)
    SendServerMessage(CreateServerMessage(MessageType::ServerShutdown, dummyData), ipAddress, port);

  m_chatHistory.clear();
  m_clientList.clear();
  m_chatUserInfos.clear();
}

void CChatServerApp::OnReceive(int dataBytesNumber, const ByteVector& data, const CString& clientIpAddress, UINT clientPort)
{
  if (dataBytesNumber < MessageHeaderSize)
    return;

  // Parse header data (message type and body size) 
  int bytePos = 0;
  auto type = data[bytePos++];
  TwoBytesInteger dataLength = { 0 };

  dataLength.bytes[0] = data[bytePos++];
  dataLength.bytes[1] = data[bytePos++];

  if (dataBytesNumber < dataLength.value + MessageHeaderSize)
    return;

  switch (type)
  {
    case MessageType::Connect:
    {
      dataLength.bytes[0] = data[bytePos++];
      dataLength.bytes[1] = data[bytePos++];

      ByteVector userNameData;
      userNameData.reserve(dataLength.value);
      userNameData.insert(userNameData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

      bytePos += dataLength.value;

      CString userName;
      UTF8ToString(userNameData, userName);

      auto findResult = std::find_if(m_chatUserInfos.cbegin(), m_chatUserInfos.cend(), [userName](auto& info)
                                                                                       { return info.first == userName; });

      if (findResult == m_chatUserInfos.end())
      {
        ByteVector userAvatarData;

        dataLength.bytes[0] = data[bytePos++];
        dataLength.bytes[1] = data[bytePos++];

        userAvatarData.reserve(dataLength.value);
        userAvatarData.insert(userAvatarData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

        if (auto newBitmap = PictureDataToBitmap(userAvatarData))
        {
          auto userAvatar = newBitmap.get();

          m_chatUserInfos.emplace_back(userName, userAvatar);
          m_clientList.emplace_back(clientIpAddress, clientPort);
          reinterpret_cast<CChatServerDlg*>(m_pMainWnd)->OnConnect(userName, userAvatar);

          m_chatHistory.push_back(data);

          // Sending answer to all clients
          for (auto& [ipAddress, port] : m_clientList)
          {
            if (ipAddress == clientIpAddress && port == clientPort)
            {
              for (auto& historyData : m_chatHistory)
                SendServerMessage(historyData, ipAddress, port);
            }
            else
              SendServerMessage(data, ipAddress, port);
          }
        }
        else
        {
          ByteVector dummyData;

          SendServerMessage(CreateServerMessage(MessageType::ConnectError, dummyData), clientIpAddress, clientPort);
        }
      }
      else
      {
        ByteVector dummyData;

        SendServerMessage(CreateServerMessage(MessageType::ConnectWrongUserName, dummyData), clientIpAddress, clientPort);
      }
      break;
    }

    case MessageType::SendText:
    {
      dataLength.bytes[0] = data[bytePos++];
      dataLength.bytes[1] = data[bytePos++];

      ByteVector userNameData;

      userNameData.reserve(dataLength.value);
      userNameData.insert(userNameData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

      bytePos += dataLength.value;

      CString userName;
      UTF8ToString(userNameData, userName);

      auto findResult = std::find_if(m_chatUserInfos.cbegin(), m_chatUserInfos.cend(), [userName](auto& info)
                                                                                       { return info.first == userName; });

      if (findResult != m_chatUserInfos.end())
      {
        auto index = static_cast<int>(std::distance(m_chatUserInfos.cbegin(), findResult));
        ByteVector userAvatarData;

        dataLength.bytes[0] = data[bytePos++];
        dataLength.bytes[1] = data[bytePos++];

        userAvatarData.reserve(dataLength.value);
        userAvatarData.insert(userAvatarData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

        if (auto newBitmap = PictureDataToBitmap(userAvatarData))
        {
          auto userAvatar = newBitmap.get();

          bytePos += dataLength.value;

          ByteVector textData;

          dataLength.bytes[0] = data[bytePos++];
          dataLength.bytes[1] = data[bytePos++];

          textData.reserve(dataLength.value);
          textData.insert(textData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

          CString text;
          UTF8ToString(textData, text);

          reinterpret_cast<CChatServerDlg*>(m_pMainWnd)->OnSendText(userName, text, index);

          m_chatHistory.push_back(data);

          // Sending answer to all clients
          for (auto& [ipAddress, port] : m_clientList)
            SendServerMessage(data, ipAddress, port);
        }
        else
        {
          ByteVector dummyData;

          SendServerMessage(CreateServerMessage(MessageType::SendTextError, dummyData), clientIpAddress, clientPort);
        }
      }
      else
      {
        ByteVector dummyData;

        SendServerMessage(CreateServerMessage(MessageType::SendTextError, dummyData), clientIpAddress, clientPort);
      }

      break;
    }

    case MessageType::Disconnect:
    {
      dataLength.bytes[0] = data[bytePos++];
      dataLength.bytes[1] = data[bytePos++];

      ByteVector userNameData;

      userNameData.reserve(dataLength.value);
      userNameData.insert(userNameData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

      bytePos += dataLength.value;

      CString userName;
      UTF8ToString(userNameData, userName);

      auto findResult = std::find_if(m_chatUserInfos.cbegin(), m_chatUserInfos.cend(), [userName](auto& info)
                                                                                       { return info.first == userName; });

      if (findResult != m_chatUserInfos.end())
      {
        auto index = static_cast<int>(std::distance(m_chatUserInfos.cbegin(), findResult));
        ByteVector userAvatarData;

        dataLength.bytes[0] = data[bytePos++];
        dataLength.bytes[1] = data[bytePos++];

        userAvatarData.reserve(dataLength.value);
        userAvatarData.insert(userAvatarData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

        if (auto newBitmap = PictureDataToBitmap(userAvatarData))
        {
          auto userAvatar = newBitmap.get();

          reinterpret_cast<CChatServerDlg*>(m_pMainWnd)->OnDisconnect(userName, index);

          m_chatHistory.push_back(data);
          m_chatUserInfos.erase(findResult);

          // Send answer to all clients (sender including)
          for (auto& [ipAddress, port] : m_clientList)
            SendServerMessage(data, ipAddress, port);

          // Remove sender from active client list 
          auto findClient = std::find_if(m_clientList.cbegin(), m_clientList.cend(), [clientIpAddress, clientPort](auto& client)
                                                                                     { return (client.first == clientIpAddress && client.second == clientPort); });

          if (findClient != m_clientList.end())
            m_clientList.erase(findClient);
        }
        else
        {
          ByteVector dummyData;

          SendServerMessage(CreateServerMessage(MessageType::DisconnectError, dummyData), clientIpAddress, clientPort);
        }
      }
      else
      {
        ByteVector dummyData;

        SendServerMessage(CreateServerMessage(MessageType::DisconnectError, dummyData), clientIpAddress, clientPort);
      }

      break;
    }

    case MessageType::ClientShutdown:
    {
      dataLength.bytes[0] = data[bytePos++];
      dataLength.bytes[1] = data[bytePos++];

      ByteVector userNameData;

      userNameData.reserve(dataLength.value);
      userNameData.insert(userNameData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

      bytePos += dataLength.value;

      CString userName;
      UTF8ToString(userNameData, userName);

      auto findResult = std::find_if(m_chatUserInfos.cbegin(), m_chatUserInfos.cend(), [userName](auto& info)
                                                                                       { return info.first == userName; });

      if (findResult != m_chatUserInfos.end())
      {
        auto index = static_cast<int>(std::distance(m_chatUserInfos.cbegin(), findResult));
        ByteVector userAvatarData;

        dataLength.bytes[0] = data[bytePos++];
        dataLength.bytes[1] = data[bytePos++];

        userAvatarData.reserve(dataLength.value);
        userAvatarData.insert(userAvatarData.begin(), data.begin() + bytePos, data.begin() + bytePos + dataLength.value);

        if (auto newBitmap = PictureDataToBitmap(userAvatarData))
        {
          auto userAvatar = newBitmap.get();

          reinterpret_cast<CChatServerDlg*>(m_pMainWnd)->OnDisconnect(userName, index);

          m_chatHistory.push_back(data);
          m_chatUserInfos.erase(findResult);

          // Remove sender from active client list (before sending answer to all active clients) 
          auto findClient = std::find_if(m_clientList.cbegin(), m_clientList.cend(), [clientIpAddress, clientPort](auto& client)
                                                                                     { return (client.first == clientIpAddress && client.second == clientPort); });

          if (findClient != m_clientList.end())
            m_clientList.erase(findClient);

          // Send answer to all clients (except sender)
          for (auto& [ipAddress, port] : m_clientList)
            SendServerMessage(data, ipAddress, port);
        }
      }

      break;
    }
  }
}
