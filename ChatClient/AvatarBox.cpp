#include "pch.h"
#include "AvatarBox.h"

#include "ChatClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CAvatarBox, CStatic)

CAvatarBox::CAvatarBox()
{
}

CAvatarBox::~CAvatarBox()
{
}

BEGIN_MESSAGE_MAP(CAvatarBox, CStatic)
  ON_WM_PAINT()
END_MESSAGE_MAP()

void CAvatarBox::OnPaint()
{
  CPaintDC dc(this);

  DrawCurrentBitmap(&dc);
}

void CAvatarBox::SetCurrentBitmap(const CString& path)
{
  CImage image;

  if (image.Load(path) == S_OK)
  {
    CRect rect;
    GetClientRect(rect);

    // Create an avatar image by picture frame size
    CImage avatar;
    avatar.Create(rect.Width(), rect.Height(), 32);

    auto avatarDC = avatar.GetDC();

    SetStretchBltMode(avatarDC, COLORONCOLOR);

    image.StretchBlt(avatarDC, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
    avatar.ReleaseDC();

    if (auto imageBitmapHandle = avatar.Detach())
    {
      // Delete the current bitmap
      if (m_bitmap.DeleteObject())
        m_bitmap.Detach();	// If there was a bitmap, detach it

      // Attach the currently loaded bitmap to the bitmap object
      m_bitmap.Attach(imageBitmapHandle);
    }

    m_bitmap.GetBitmap(&m_bitmapData);  // Get bitmap data

    Invalidate();

    m_isBitmapLoaded = true;
  }
  else
    m_isBitmapLoaded = false;
}

HBITMAP CAvatarBox::GetCurrentBitmap()
{
  if (m_isBitmapLoaded)
    return m_bitmap;
  else
    return GetBitmap();
}

void CAvatarBox::ChangeCurrentBitmap(HBITMAP bitmap)
{
  // Delete the current bitmap
  if (m_bitmap.DeleteObject())
    m_bitmap.Detach();	// If there was a bitmap, detach it

  if (bitmap)
  {
    m_bitmap.Attach(bitmap);
    m_bitmap.GetBitmap(&m_bitmapData);  // Get bitmap data

    Invalidate();

    m_isBitmapLoaded = true;
  }
  else
    m_isBitmapLoaded = false;
}

void CAvatarBox::DrawCurrentBitmap(CPaintDC* dc)
{
  if (m_isBitmapLoaded)
  {
    // Create a device context to load the bitmap into
    CDC dcMem;
    dcMem.CreateCompatibleDC(dc);

    // Get the picture area available
    CRect rect;
    GetClientRect(rect);

    // Select the bitmap into compatible device context
    auto oldBitmap = dcMem.SelectObject(&m_bitmap);

    dc->BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

    dcMem.SelectObject(oldBitmap);
    dcMem.DeleteDC();
  }
}
