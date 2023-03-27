#pragma once

#include "AvatarBox.h"

class CLoginDialog : public CDialogEx
{
  DECLARE_DYNAMIC(CLoginDialog)

public:
  CLoginDialog(CWnd* parent = nullptr);
  virtual ~CLoginDialog();

  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedChangeImageButton();
  afx_msg void OnEnChangeUsernameEdit();

  DECLARE_MESSAGE_MAP()

private:
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_LOGIN_DIALOG };
#endif

  CIPAddressCtrl m_ipAddress;
  CEdit m_portEdit;
  int m_portNumber = 10000;
  CSpinButtonCtrl m_portSpin;
  CString m_userName;
  CAvatarBox m_avatarImage;
};
