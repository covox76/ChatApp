#include "pch.h"
#include "LoginDlg.h"

#include "ChatClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLoginDialog, CDialogEx)

CLoginDialog::CLoginDialog(CWnd* parent /*= nullptr*/)
  : CDialogEx(IDD_LOGIN_DIALOG, parent)
{

}

CLoginDialog::~CLoginDialog()
{
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_SERVER_IPADDRESS, m_ipAddress);
  DDX_Text(pDX, IDC_PORT_EDIT, m_portNumber);
  DDX_Control(pDX, IDC_PORT_EDIT, m_portEdit);
  DDX_Control(pDX, IDC_PORT_SPIN, m_portSpin);
  DDX_Text(pDX, IDC_USERNAME_EDIT, m_userName);
  DDX_Control(pDX, IDC_AVATAR_STATIC, m_avatarImage);
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
  ON_BN_CLICKED(IDOK, &CLoginDialog::OnBnClickedOk)
  ON_BN_CLICKED(IDC_CHANGE_IMAGE_BUTTON, &CLoginDialog::OnBnClickedChangeImageButton)
  ON_EN_CHANGE(IDC_USERNAME_EDIT, &CLoginDialog::OnEnChangeUsernameEdit)
END_MESSAGE_MAP()

BOOL CLoginDialog::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  m_ipAddress.SetAddress(127, 0, 0, 1);
  m_portEdit.SetLimitText(5);
  m_portSpin.SetRange32(MinUdpPortNumber, MaxUdpPortNumber);

  return TRUE;
}

void CLoginDialog::OnEnChangeUsernameEdit()
{
  UpdateData();

  if (m_userName.IsEmpty())
    GetDlgItem(IDOK)->EnableWindow(FALSE);
  else
    GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CLoginDialog::OnBnClickedChangeImageButton()
{
  CFileDialog fileDlg(TRUE, L".bmp", L"", 0, L"Pliki BMP (*.bmp)|*.bmp|Pliki JPG (*.jpg)|*.jpg|Pliki PNG (*.png)|*.png|Wszystkie pliki (*.*)|*.*||");

  if (fileDlg.DoModal() == IDOK)
    m_avatarImage.SetCurrentBitmap(fileDlg.GetPathName());
}

void CLoginDialog::OnBnClickedOk()
{
  UpdateData();

  if (m_userName.GetLength() > 15)
  {
    AfxMessageBox(WarningText::UserNameLengthExceeded, MB_OK | MB_ICONEXCLAMATION);

    return;
  }

  DWORD ipAddress;

  if (m_ipAddress.GetAddress(ipAddress) != 4)
  {
    AfxMessageBox(WarningText::FullIpAddressNeeded, MB_OK | MB_ICONEXCLAMATION);

    return;
  }

  CString ipAddressText;

  m_ipAddress.GetWindowText(ipAddressText);

  struct in_addr addr {};

  if (inet_pton(AF_INET, CT2CA(ipAddressText), &addr) != 1)
  {
    switch (WSAGetLastError())
    {
      case WSAEAFNOSUPPORT:
        AfxMessageBox(WarningText::IpAddressNotSupported, MB_OK | MB_ICONEXCLAMATION);
        break;

      case WSAEFAULT:
        AfxMessageBox(WarningText::IpAddressWrongAddressSpace, MB_OK | MB_ICONEXCLAMATION);
        break;
    }

    return;
  }

  if (theApp.Connect(ipAddressText, m_portNumber, m_userName, m_avatarImage.GetCurrentBitmap()))
    CDialogEx::OnOK();
  else
    CDialogEx::OnCancel();
}
