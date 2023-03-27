#pragma once

class CAvatarBox : public CStatic
{
  DECLARE_DYNAMIC(CAvatarBox)

public:
  CAvatarBox();
  virtual ~CAvatarBox();

  void SetCurrentBitmap(const CString& path);
  HBITMAP GetCurrentBitmap();
  void ChangeCurrentBitmap(HBITMAP bitmap);
  
  afx_msg void OnPaint();

protected:
  DECLARE_MESSAGE_MAP()

private:
  void DrawCurrentBitmap(CPaintDC* dc);

  bool m_isBitmapLoaded = false;
  CBitmap m_bitmap;
  BITMAP m_bitmapData = { 0 };
};


