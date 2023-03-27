#pragma once

#include "AppDefs.h"

std::optional<ByteVector> BitmapToPictureData(HBITMAP bitmapHandle);
std::unique_ptr<std::remove_pointer<HBITMAP>::type, std::function<void(HBITMAP)>> PictureDataToBitmap(const ByteVector& data);

void UTF8ToString(ByteVector& data, CString& text);
void UTF8ToWideChar(ByteVector& data);
void StringToUTF8(const CString& text, ByteVector& data);
void WideCharToUTF8(ByteVector& data);

CString GetLastErrorMessage(DWORD lastErrorCode = 0);
CString GetErrorMessage(DWORD errorCode);
