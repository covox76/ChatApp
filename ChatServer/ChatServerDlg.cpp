#include "pch.h"
#include "ChatServerDlg.h"

#include "ChatServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChatServerDlg::CChatServerDlg(CWnd* parent /*=nullptr*/)
  : CDialogEx(IDD_CHATSERVER_DIALOG, parent)
{
  m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
}

void CChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_INFO_LIST, m_infoList);
  DDX_Control(pDX, IDC_PORT_EDIT, m_portEdit);
  DDX_Text(pDX, IDC_PORT_EDIT, m_portNumber);
  DDX_Control(pDX, IDC_PORT_SPIN, m_portSpin);
  DDX_Control(pDX, IDC_START_STOP_BUTTON, m_startButton);
}

BEGIN_MESSAGE_MAP(CChatServerDlg, CDialogEx)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_GETMINMAXINFO()
  ON_BN_CLICKED(IDC_START_STOP_BUTTON, &CChatServerDlg::OnBnClickedStartStopButton)
END_MESSAGE_MAP()

BOOL CChatServerDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hIcon, FALSE);

  CRect rect;

  GetWindowRect(&rect);

  m_minWindowWidth = rect.Width();
  m_minWindowHeight = rect.Height();

  m_portEdit.SetLimitText(5);
  m_portSpin.SetRange32(MinUdpPortNumber, MaxUdpPortNumber);

  auto exStyle = m_infoList.GetExtendedStyle();
  exStyle |= LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;

  m_infoList.SetExtendedStyle(exStyle);

  m_infoList.InsertColumn(0, DialogText::UserColumnName, LVCFMT_LEFT, 100);
  m_infoList.InsertColumn(1, DialogText::MessageColumnName, LVCFMT_LEFT, 200);

  if (m_imageList.Create(32, 32, ILC_COLOR32, 0, 0))
    m_infoList.SetImageList(&m_imageList, LVSIL_SMALL);

  return TRUE;
}

void CChatServerDlg::OnPaint()
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

HCURSOR CChatServerDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CChatServerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  if (lpMMI)
  {
    lpMMI->ptMinTrackSize.x = m_minWindowWidth;
    lpMMI->ptMinTrackSize.y = m_minWindowHeight;
  }

  CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CChatServerDlg::OnBnClickedStartStopButton()
{
  UpdateData();

  if (m_serverIsRunning)
  {
    m_portSpin.EnableWindow(TRUE);
    m_portEdit.EnableWindow(TRUE);
    m_startButton.SetWindowText(DialogText::StartButton);
    m_infoList.DeleteAllItems();

    theApp.CloseListenSocket();

    m_serverIsRunning = false;
  }
  else if (theApp.InitListenSocket(m_portNumber))
  {
    m_portSpin.EnableWindow(FALSE);
    m_portEdit.EnableWindow(FALSE);
    m_startButton.SetWindowText(DialogText::StopButton);

    m_serverIsRunning = true;
  }
}

void CChatServerDlg::OnConnect(const CString& userName, HBITMAP userAvatar)
{
  CBitmap bitmap;
  auto imageIndex = -1;

  if (bitmap.Attach(userAvatar))
    imageIndex = m_imageList.Add(&bitmap, RGB(255, 255, 255));

  m_infoList.SetRedraw(FALSE);

  auto itemPos = m_infoList.InsertItem(m_infoList.GetItemCount(), userName, imageIndex);
  m_infoList.SetItemText(itemPos, 1, InfoText::UserHasJoinedChat);

  m_infoList.EnsureVisible(itemPos, FALSE);
  m_infoList.SetRedraw(TRUE);
  m_infoList.InvalidateRect(NULL);
  m_infoList.UpdateWindow();
}

void CChatServerDlg::OnSendText(const CString& userName, const CString& text, int imageIndex)
{
  m_infoList.SetRedraw(FALSE);

  auto itemPos = m_infoList.InsertItem(m_infoList.GetItemCount(), userName, imageIndex);
  m_infoList.SetItemText(itemPos, 1, text);

  m_infoList.EnsureVisible(itemPos, FALSE);
  m_infoList.SetRedraw(TRUE);
  m_infoList.InvalidateRect(NULL);
  m_infoList.UpdateWindow();
}

void CChatServerDlg::OnDisconnect(const CString& userName, int imageIndex)
{
  m_infoList.SetRedraw(FALSE);

  auto itemPos = m_infoList.InsertItem(m_infoList.GetItemCount(), userName, imageIndex);
  m_infoList.SetItemText(itemPos, 1, InfoText::UserHasLeftChat);

  m_imageList.Remove(imageIndex);

  m_infoList.EnsureVisible(itemPos, FALSE);
  m_infoList.SetRedraw(TRUE);
  m_infoList.InvalidateRect(NULL);
  m_infoList.UpdateWindow();
}
