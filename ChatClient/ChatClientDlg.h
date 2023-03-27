#pragma once

class CChatClientDlg : public CDialogEx
{
public:
  CChatClientDlg(CWnd* parent = nullptr);

  void OnConnect(const CString& userName, HBITMAP userAvatar);
  void OnSendText(const CString& userName, const CString& text, int imageIndex);
  void OnDisconnect(const CString& userName, int imageIndex);
  void OnError();

protected:
  HICON m_hIcon;

  BOOL OnInitDialog() override;
  void DoDataExchange(CDataExchange* pDX) override;

  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnBnClickedSendButton();
  afx_msg void OnBnClickedLoginButton();

  DECLARE_MESSAGE_MAP()

private:
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_CHATCLIENT_DIALOG };
#endif

  bool m_clientIsConnected = false;
  int m_minWindowWidth = -1;
  int m_minWindowHeight = -1;

  CImageList m_imageList;
  CMFCListCtrl m_chatList;
  CString m_message;
  CEdit m_messageEdit;
  CButton m_sendButton;
  CButton m_loginButton;
};
