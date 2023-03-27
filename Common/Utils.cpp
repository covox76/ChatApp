#include "pch.h"
#include "Utils.h"

#include <lmerr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::optional<ByteVector> BitmapToPictureData(HBITMAP bitmapHandle)
{
  // Result contains bytes sequence of width, height, number bits per pixel and bitmap pixels
  std::optional<ByteVector> result;

  if (auto dc = CreateCompatibleDC(nullptr))
  {
    BITMAP bitmap = { 0 };
    BITMAPINFO bitmapInfo = { 0 };
    auto oldBitmap = reinterpret_cast<HBITMAP>(SelectObject(dc, bitmapHandle));

    GetObject(bitmapHandle, sizeof(bitmap), &bitmap);

    FourBytesInteger width = { 0 };
    FourBytesInteger height = { 0 };
    TwoBytesInteger bitsPerPixel = { 0 };
    ByteVector pixels;

    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width.value = bitmap.bmWidth;
    bitmapInfo.bmiHeader.biHeight = height.value = bitmap.bmHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = bitsPerPixel.value = bitmap.bmBitsPixel;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage = ((width.value * bitmap.bmBitsPixel + 31) / 32) * 4 * height.value;

    pixels.resize(bitmapInfo.bmiHeader.biSizeImage);

    if (pixels.size() > 0 && GetDIBits(dc, bitmapHandle, 0, height.value, &pixels[0], &bitmapInfo, DIB_RGB_COLORS) > 0)
    {
      height.value = height.value < 0 ? height.value * -1 : height.value;

      result = std::make_optional<ByteVector>();

      if (result.has_value())
      {
        result->reserve(PictureDataFixedSize);

        result->push_back(width.bytes[0]);
        result->push_back(width.bytes[1]);
        result->push_back(width.bytes[2]);
        result->push_back(width.bytes[3]);

        result->push_back(height.bytes[0]);
        result->push_back(height.bytes[1]);
        result->push_back(height.bytes[2]);
        result->push_back(height.bytes[3]);

        result->push_back(bitsPerPixel.bytes[0]);
        result->push_back(bitsPerPixel.bytes[1]);

        result->insert(result->end(), pixels.begin(), pixels.end());

        if (result->size() != PictureDataFixedSize)
          result.reset();
      }

      SelectObject(dc, oldBitmap);
    }

    DeleteDC(dc);
  }

  return result;
}

std::unique_ptr<std::remove_pointer<HBITMAP>::type, std::function<void(HBITMAP)>> PictureDataToBitmap(const ByteVector& data)
{
  if (data.size() != PictureDataFixedSize)
    return {};

  BITMAPINFO bitmapInfo = { 0 };
  FourBytesInteger width = { 0 };
  FourBytesInteger height = { 0 };
  TwoBytesInteger bitsPerPixel = { 0 };

  width.bytes[0] = data[0];
  width.bytes[1] = data[1];
  width.bytes[2] = data[2];
  width.bytes[3] = data[3];

  height.bytes[0] = data[4];
  height.bytes[1] = data[5];
  height.bytes[2] = data[6];
  height.bytes[3] = data[7];

  bitsPerPixel.bytes[0] = data[8];
  bitsPerPixel.bytes[1] = data[9];

  ByteVector pixels(data.begin() + 10, data.end());

  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmapInfo.bmiHeader.biWidth = width.value;
  bitmapInfo.bmiHeader.biHeight = height.value < 0 ? height.value * -1 : height.value;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = bitsPerPixel.value;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;
  bitmapInfo.bmiHeader.biSizeImage = ((width.value * bitsPerPixel.value + 31) / 32) * 4 * height.value;

  if (auto result = CreateDIBitmap(GetDC(nullptr), &bitmapInfo.bmiHeader, CBM_INIT, &pixels[0], &bitmapInfo, DIB_RGB_COLORS))
    return std::unique_ptr<std::remove_pointer<HBITMAP>::type, std::function<void(HBITMAP)>>(result, [&](HBITMAP hBmp)
                                                                                             {
                                                                                               DeleteObject(hBmp);
                                                                                             });
  else
    return {};
}

void WideCharToUTF8(ByteVector& data)
{
  ByteVector multiByteData;
  auto dataSize = static_cast<int>(data.size() / sizeof(WCHAR));
  auto bytesNum = ::WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWSTR>(data.data()), dataSize, NULL, 0, 0, 0);

  multiByteData.resize(bytesNum);
  bytesNum = ::WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWSTR>(data.data()), dataSize,
                                   reinterpret_cast<LPSTR>(multiByteData.data()), bytesNum, 0, 0);

  SecureZeroMemory(data.data(), data.size());

  data.resize(bytesNum + 1);

  memcpy_s(data.data(), data.size(), multiByteData.data(), multiByteData.size());

  data[bytesNum] = 0;
  data.resize(bytesNum);

  SecureZeroMemory(multiByteData.data(), multiByteData.size());
}

void StringToUTF8(const CString& text, ByteVector& data)
{
  auto dataSize = static_cast<size_t>(text.GetLength() * sizeof(WCHAR));
  data.resize(dataSize);

  memcpy_s(data.data(), dataSize, text.GetString(), dataSize);

  WideCharToUTF8(data);
}

void UTF8ToWideChar(ByteVector& data)
{
  ByteVector wideCharData;
  auto dataSize = static_cast<int>(data.size());
  auto bytesNum = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCSTR>(data.data()), dataSize, NULL, 0) * sizeof(WCHAR);

  wideCharData.resize(bytesNum);
  bytesNum = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCSTR>(data.data()), dataSize,
                                   reinterpret_cast<LPWSTR>(wideCharData.data()), static_cast<int>(bytesNum)) * sizeof(WCHAR);

  SecureZeroMemory(data.data(), data.size());
  data.resize(bytesNum + 2);

  memcpy_s(data.data(), data.size(), wideCharData.data(), wideCharData.size());

  data[bytesNum] = 0;
  data[bytesNum + 1] = 0;
  data.resize(bytesNum);

  SecureZeroMemory(wideCharData.data(), wideCharData.size());
}

void UTF8ToString(ByteVector& data, CString& text)
{
  UTF8ToWideChar(data);

  auto textSize = static_cast<int>(data.size() / sizeof(WCHAR));

  memcpy_s(text.GetBuffer(textSize), data.size(), data.data(), data.size());

  text.ReleaseBuffer(textSize);
}

CString GetLastErrorMessage(DWORD lastErrorCode /*= 0*/)
{
  CString errorMessage;

  if (lastErrorCode == 0)
    lastErrorCode = GetLastError();

  errorMessage.Format(L"Błąd nr %d. %s", lastErrorCode, GetErrorMessage(lastErrorCode).GetString());
  errorMessage.Remove(L'\n');
  errorMessage.Remove(L'\r');

  return errorMessage;
}

CString GetErrorMessage(DWORD errorCode)
{
  LPVOID errorBuffer = nullptr;
  DWORD maxErrorBufferSize = 256;
  DWORD resultCode = 0;
  WORD facility = HRESULT_FACILITY(errorCode);

  if (facility == FACILITY_MSMQ)
  {
    // MSMQ errors only (see winerror.h for facility info). Load the MSMQ library containing the error message strings.
    if (auto instance = LoadLibrary(L"MQUTIL.DLL"))
      resultCode = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 instance, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&errorBuffer),
                                 maxErrorBufferSize, NULL);
  }
  else if (errorCode >= NERR_BASE && errorCode <= MAX_NERR)
  {
    // Could be a network error. Load the library containing network messages.
    if (auto instance = LoadLibrary(L"NETMSG.DLL"))
      resultCode = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 instance, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&errorBuffer),
                                 maxErrorBufferSize, NULL);
  }

  if (resultCode == 0)
  {
    if (facility == FACILITY_WINDOWS)
      errorCode = HRESULT_CODE(errorCode);

    resultCode = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&errorBuffer),
                               maxErrorBufferSize, NULL);
  }

  CString resultMessage;

  if (resultCode > 0)
    resultMessage.Format(L"%s", reinterpret_cast<LPTSTR>(errorBuffer));
  else
    resultMessage.Format(L"Pobranie informacji o błędzie zakończyło się niepowodzeniem. (%d)", GetLastError());

  if (errorBuffer != nullptr)
    LocalFree(errorBuffer);

  return resultMessage;
}
