#include "pch.h"
#include "ChatClientDlg.h"

#include "ChatClient.h"
#include "LoginDlg.h"
#include "ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChatClientDlg::CChatClientDlg(CWnd* parent /*=nullptr*/)
  : CDialogEx(IDD_CHATCLIENT_DIALOG, parent)
{
  m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_CHAT_LIST, m_chatList);
  DDX_Text(pDX, IDC_MESSAGE_EDIT, m_message);
  DDX_Control(pDX, IDC_MESSAGE_EDIT, m_messageEdit);
  DDX_Control(pDX, IDC_SEND_BUTTON, m_sendButton);
  DDX_Control(pDX, IDC_LOGIN_BUTTON, m_loginButton);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialogEx)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_GETMINMAXINFO()
  ON_BN_CLICKED(IDC_SEND_BUTTON, &CChatClientDlg::OnBnClickedSendButton)
  ON_BN_CLICKED(IDC_LOGIN_BUTTON, &CChatClientDlg::OnBnClickedLoginButton)
END_MESSAGE_MAP()

BOOL CChatClientDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hIcon, FALSE);

  CRect rect;

  GetWindowRect(&rect);

  m_minWindowWidth = rect.Width();
  m_minWindowHeight = rect.Height();

  auto exStyle = m_chatList.GetExtendedStyle();
  exStyle |= LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;

  m_chatList.SetExtendedStyle(exStyle);

  m_chatList.InsertColumn(0, DialogText::UserColumnName, LVCFMT_LEFT, 100);
  m_chatList.InsertColumn(1, DialogText::MessageColumnName, LVCFMT_LEFT, 200);

  if (m_imageList.Create(32, 32, ILC_COLOR32, 0, 0))
    m_chatList.SetImageList(&m_imageList, LVSIL_SMALL);

  return TRUE;
}

void CChatClientDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this);

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    CRect rect;
    GetClientRect(&rect);

    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    dc.DrawIcon(x, y, m_hIcon);
  }
  else
    CDialogEx::OnPaint();
}

HCURSOR CChatClientDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CChatClientDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  if (lpMMI)
  {
    lpMMI->ptMinTrackSize.x = m_minWindowWidth;
    lpMMI->ptMinTrackSize.y = m_minWindowHeight;
  }

  CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CChatClientDlg::OnBnClickedSendButton()
{
  UpdateData();

  if (m_clientIsConnected)
    theApp.SendText(m_message);
}

void CChatClientDlg::OnBnClickedLoginButton()
{
  if (m_clientIsConnected)
  {
    theApp.Disconnect();

    m_messageEdit.EnableWindow(FALSE);
    m_sendButton.EnableWindow(FALSE);
    m_loginButton.SetWindowText(DialogText::LoginButton);
    m_chatList.DeleteAllItems();

    m_clientIsConnected = false;
  }
  else
  {
    CLoginDialog loginDlg;

    if (loginDlg.DoModal() == IDOK)
    {
      m_messageEdit.EnableWindow(TRUE);
      m_sendButton.EnableWindow(TRUE);
      m_loginButton.SetWindowText(DialogText::LogoutButton);

      m_clientIsConnected = true;
    }
  }
}

void CChatClientDlg::OnConnect(const CString& userName, HBITMAP userAvatar)
{
  CBitmap bitmap;
  auto imageIndex = -1;

  if (bitmap.Attach(userAvatar))
    imageIndex = m_imageList.Add(&bitmap, RGB(255, 255, 255));

  m_chatList.SetRedraw(FALSE);

  auto itemPos = m_chatList.InsertItem(m_chatList.GetItemCount(), userName, imageIndex);
  m_chatList.SetItemText(itemPos, 1, InfoText::UserHasJoinedChat);

  m_chatList.EnsureVisible(itemPos, FALSE);
  m_chatList.SetRedraw(TRUE);
  m_chatList.InvalidateRect(NULL);
  m_chatList.UpdateWindow();
}

void CChatClientDlg::OnSendText(const CString& userName, const CString& text, int imageIndex)
{
  m_chatList.SetRedraw(FALSE);

  auto itemPos = m_chatList.InsertItem(m_chatList.GetItemCount(), userName, imageIndex);
  m_chatList.SetItemText(itemPos, 1, text);

  m_chatList.EnsureVisible(itemPos, FALSE);
  m_chatList.SetRedraw(TRUE);
  m_chatList.InvalidateRect(NULL);
  m_chatList.UpdateWindow();
}

void CChatClientDlg::OnDisconnect(const CString& userName, int imageIndex)
{
  m_chatList.SetRedraw(FALSE);

  auto itemPos = m_chatList.InsertItem(m_chatList.GetItemCount(), userName, imageIndex);
  m_chatList.SetItemText(itemPos, 1, InfoText::UserHasLeftChat);

  m_imageList.Remove(imageIndex);

  m_chatList.EnsureVisible(itemPos, FALSE);
  m_chatList.SetRedraw(TRUE);
  m_chatList.InvalidateRect(NULL);
  m_chatList.UpdateWindow();
}

void CChatClientDlg::OnError()
{
  if (m_clientIsConnected)
  {
    m_messageEdit.EnableWindow(FALSE);
    m_sendButton.EnableWindow(FALSE);
    m_loginButton.SetWindowText(DialogText::LoginButton);
    m_chatList.DeleteAllItems();

    m_clientIsConnected = false;
  }
}
