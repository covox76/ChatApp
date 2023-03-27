#pragma once

class CChatServerDlg : public CDialogEx
{
public:
  CChatServerDlg(CWnd* parent = nullptr);

  void OnConnect(const CString& userName, HBITMAP userAvatar);
  void OnSendText(const CString& userName, const CString& text, int imageIndex);
  void OnDisconnect(const CString& userName, int imageIndex);

protected:
  HICON m_hIcon;

  BOOL OnInitDialog() override;
  void DoDataExchange(CDataExchange* pDX) override;

  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnBnClickedStartStopButton();

  DECLARE_MESSAGE_MAP()

private:
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_CHATSERVER_DIALOG };
#endif

  bool m_serverIsRunning = false;
  int m_minWindowWidth = -1;
  int m_minWindowHeight = -1;

  CImageList m_imageList;
  CMFCListCtrl m_infoList;
  CEdit m_portEdit;
  int m_portNumber = 10000;
  CSpinButtonCtrl m_portSpin;
  CButton m_startButton;
};
