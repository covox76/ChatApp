#pragma once

#include "pch.h"

static constexpr auto MinUdpPortNumber = 4096;
static constexpr auto MaxUdpPortNumber = 65535;
// bytes sequence of width (4 bytes), height (4 bytes), number bits per pixel (2 bytes) and bitmap pixels (4096 bytes)
static constexpr auto PictureDataFixedSize = 4106ull;

static constexpr auto MessageHeaderSize = 3;
static constexpr auto SocketBufferSize = 8192;

using ByteVector = std::vector<BYTE>;

union FourBytesInteger
{
  std::uint32_t value;
  BYTE bytes[4];
};

union TwoBytesInteger
{
  std::uint16_t value;
  BYTE bytes[2];
};

enum MessageType : BYTE
{
  Disconnect,
  DisconnectError,
  SendText,
  SendTextError,
  Connect,
  ConnectError,
  ConnectWrongUserName,
  ServerShutdown,
  ClientShutdown
};

namespace DialogText
{
  constexpr auto UserColumnName = L"U�ytkownik";
  constexpr auto MessageColumnName = L"Wiadomo��";
  constexpr auto StartButton = L"Uruchom";
  constexpr auto StopButton = L"Zatrzymaj";
  constexpr auto LoginButton = L"Po��cz";
  constexpr auto LogoutButton = L"Roz��cz";
}

namespace InfoText
{
  constexpr auto UserHasJoinedChat = L"U�ytkownik do��czy� do chatu.";
  constexpr auto UserHasLeftChat = L"U�ytkownik opu�ci� chat.";
}

namespace WarningText
{
  constexpr auto UserNameLengthExceeded = L"Przekroczono maksymaln� liczb� znak�w w nazwie u�ytkownika.\nDopuszczlna liczba znak�w wynosi 15.";
  constexpr auto FullIpAddressNeeded = L"Prosz� poda� pe�en adres IP serwera.";
  constexpr auto IpAddressNotSupported = L"Podany adres IP serwera nie jest wspierany.";
  constexpr auto IpAddressWrongAddressSpace = L"Podany adres IP serwera nie nale�y do przestrzeni adresowej u�ytkownika.";
  constexpr auto ChatServerShutdown = L"Serwer czatu zosta� wy��czony.";
  constexpr auto ChatClientShutdown = L"Klient czatu zosta� wy��czony.";
}

namespace ErrorText
{
  constexpr auto UserNameExists = L"Podana nazwa u�ytkownika ju� istnieje.";
  constexpr auto UnexpectedError = L"Nieoczekiwany b��d.";
}
