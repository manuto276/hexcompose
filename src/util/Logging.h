#pragma once
#include <windows.h>
#include <string>
#include <string_view>

namespace hexcompose::log
{

  // Log minimale: OutputDebugStringW con prefix.
  inline void debug(std::wstring_view msg)
  {
    std::wstring line = L"[HexCompose] " + std::wstring(msg) + L"\r\n";
    ::OutputDebugStringW(line.c_str());
  }

  // Helper per UTF-8 literal verso wide
  inline std::wstring w(std::string_view s)
  {
    if (s.empty())
      return L"";
    int len = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out(len, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), len);
    return out;
  }

} // namespace
