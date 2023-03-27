#include "pch.h"
#include "ChatClient.h"

#include "ChatClientDlg.h"
#include "ClientSocket.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CChatClientApp::CChatClientApp()
{
}

CChatClientApp theApp;

BOOL CChatClientApp::InitInstance()
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

  CChatClientDlg dlg;

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

// Initialize client socket for listening on dynamic port
bool CChatClientApp::InitListenSocket()
{
  CloseListenSocket();

  m_listenSocket = new CClientSocket();

  auto result = m_listenSocket->Create(0, SOCK_DGRAM, NULL) ? true : false;

  if (result == false)
  {
    CloseListenSocket();

    AfxMessageBox(GetLastErrorMessage().GetString(), MB_OK | MB_ICONSTOP);
  }

  return result;
}

void CChatClientApp::CloseListenSocket()
{
  if (m_listenSocket != nullptr)
  {
    Shutdown();

    delete m_listenSocket;

    m_listenSocket = nullptr;
  }
}

ByteVector CChatClientApp::CreateClientMessage(MessageType type, const ByteVector& body)
{
  ByteVector data;
  TwoBytesInteger dataLength = { 0 };

  dataLength.value = static_cast<std::uint16_t>(body.size());

  // Message header contains message type and message body size
  data.push_back(type);
  data.push_back(dataLength.bytes[0]);
  data.push_back(dataLength.bytes[1]);

  // Append message body
  if (body.empty() == false)
    data.insert(data.end(), body.begin(), body.end());

  return data;
}

bool CChatClientApp::SendClientMessage(const ByteVector& buffer, const CString& hostIpAddress, UINT hostPortNumber)
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

bool CChatClientApp::Connect(const CString& ipAddress, int portNumber, const CString& userName, HBITMAP userAvatar)
{
  auto result = InitListenSocket();

  if (result == false)
    return result;

  m_serverIpAddress = ipAddress;
  m_serverPortNumber = portNumber;

  m_userInfoData.clear();

  // Convert user name from UNICODE standard to data in UTF8
  ByteVector userNameData;
  StringToUTF8(userName, userNameData);

  // Append user name data to user info data
  TwoBytesInteger dataLength = { 0 };
  dataLength.value = static_cast<std::uint16_t>(userNameData.size());

  m_userInfoData.push_back(dataLength.bytes[0]);
  m_userInfoData.push_back(dataLength.bytes[1]);
  m_userInfoData.insert(m_userInfoData.end(), userNameData.begin(), userNameData.end());

  // Convert user avatar bitmap to data
  auto userAvatarData = BitmapToPictureData(userAvatar);

  if (userAvatarData)
  {
    // Append user avatar data to user info data
    dataLength.value = static_cast<std::uint16_t>(userAvatarData->size());

    m_userInfoData.push_back(dataLength.bytes[0]);
    m_userInfoData.push_back(dataLength.bytes[1]);
    m_userInfoData.insert(m_userInfoData.end(), userAvatarData->begin(), userAvatarData->end());
  }

  return SendClientMessage(CreateClientMessage(MessageType::Connect, m_userInfoData), m_serverIpAddress, m_serverPortNumber);
}

bool CChatClientApp::SendText(const CString& text)
{
  auto result = false;

  if (text.IsEmpty())
    return result;

  ByteVector textData;
  // Convert user text message from UNICODE standard to user text data in UTF8
  StringToUTF8(text, textData);

  TwoBytesInteger dataLength = { 0 };
  dataLength.value = static_cast<std::uint16_t>(textData.size());

  textData.insert(textData.begin(), dataLength.bytes[1]);
  textData.insert(textData.begin(), dataLength.bytes[0]);

  // Insert user info data at very begining text data
  textData.insert(textData.begin(), m_userInfoData.begin(), m_userInfoData.end());

  return SendClientMessage(CreateClientMessage(MessageType::SendText, textData), m_serverIpAddress, m_serverPortNumber);
}

bool CChatClientApp::Disconnect()
{
  return SendClientMessage(CreateClientMessage(MessageType::Disconnect, m_userInfoData), m_serverIpAddress, m_serverPortNumber);
}

bool CChatClientApp::Shutdown()
{
  return SendClientMessage(CreateClientMessage(MessageType::ClientShutdown, m_userInfoData), m_serverIpAddress, m_serverPortNumber);
}

void CChatClientApp::OnReceive(int dataBytesNumber, const ByteVector& data, const CString& clientIpAddress, UINT clientPort)
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
          reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnConnect(userName, userAvatar);
        }
      }

      break;
    }

    case MessageType::ConnectWrongUserName:
      reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnError();
      CloseListenSocket();
      AfxMessageBox(ErrorText::UserNameExists, MB_OK | MB_ICONERROR);
      break;

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

          reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnSendText(userName, text, index);
        }
      }

      break;
    }

    case MessageType::Disconnect:
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

          m_chatUserInfos.erase(findResult);

          reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnDisconnect(userName, index);
        }
      }

      break;
    }

    case MessageType::ServerShutdown:
      CloseListenSocket();
      reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnError();
      AfxMessageBox(WarningText::ChatServerShutdown, MB_OK | MB_ICONEXCLAMATION);
      break;

    case MessageType::ConnectError:
    case MessageType::SendTextError:
    case MessageType::DisconnectError:
      CloseListenSocket();
      reinterpret_cast<CChatClientDlg*>(m_pMainWnd)->OnError();
      AfxMessageBox(ErrorText::UnexpectedError, MB_OK | MB_ICONERROR);
      break;
  }
}
