#include "WinUtils.h"
#include "../util/Logging.h"
#include <cwctype>
#include <thread>
#include <chrono>

namespace hexcompose::win
{

  HKL GetForegroundKeyboardLayout()
  {
    HWND fg = ::GetForegroundWindow();
    if (!fg)
      return ::GetKeyboardLayout(0);
    DWORD tid = ::GetWindowThreadProcessId(fg, nullptr);
    HKL h = ::GetKeyboardLayout(tid);
    if (!h)
      h = ::GetKeyboardLayout(0);
    return h;
  }

  bool IsCapsLockOn()
  {
    SHORT s = ::GetKeyState(VK_CAPITAL);
    return (s & 0x0001) != 0;
  }

  void GetKeyboardStateSnapshot(BYTE out[256], WPARAM wparam, const KBDLLHOOKSTRUCT &k)
  {
    ::GetKeyboardState(out);
    if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
    {
      out[k.vkCode] |= 0x80;
    }
    else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)
    {
      out[k.vkCode] &= ~0x80;
    }
    if (k.flags & LLKHF_ALTDOWN)
      out[VK_MENU] |= 0x80;
  }

  std::wstring ToUpperForLayout(const wchar_t *input, int len, HKL hkl)
  {
    if (len <= 0)
      return L"";
    WCHAR localeName[LOCALE_NAME_MAX_LENGTH] = {0};
    LCID lcid = MAKELCID(LOWORD((UINT_PTR)hkl), SORT_DEFAULT);
    int ok = ::LCIDToLocaleName(lcid, localeName, LOCALE_NAME_MAX_LENGTH, 0);
    if (ok == 0)
      lstrcpyW(localeName, LOCALE_NAME_USER_DEFAULT);
    wchar_t out[8] = {0};
    int r = ::LCMapStringEx(localeName, LCMAP_UPPERCASE, input, len, out, 8, nullptr, nullptr, 0);
    if (r > 0)
      return std::wstring(out, out + r);
    if (len == 1)
    {
      wchar_t c = towupper(*input);
      return std::wstring(1, c);
    }
    return std::wstring(input, input + len);
  }

  int KeyToUnicode(HKL hkl, const BYTE kbdState[256], const KBDLLHOOKSTRUCT &kbd, wchar_t *outBuf, int outBufLen)
  {
    return ::ToUnicodeEx(kbd.vkCode, kbd.scanCode, kbdState, outBuf, outBufLen, 0, hkl);
  }

  void InjectUnicodeString(const std::wstring &s)
  {
    if (s.empty())
      return;
    std::vector<INPUT> inps;
    inps.reserve(s.size() * 2);
    for (wchar_t ch : s)
    {
      INPUT down{};
      down.type = INPUT_KEYBOARD;
      down.ki.wVk = 0;
      down.ki.wScan = ch;
      down.ki.dwFlags = KEYEVENTF_UNICODE;
      down.ki.dwExtraInfo = kHexComposeTag;
      inps.push_back(down);

      INPUT up = down;
      up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
      inps.push_back(up);
    }
    ::SendInput((UINT)inps.size(), inps.data(), sizeof(INPUT));
  }

  void InjectUnicodeChar(wchar_t ch)
  {
    InjectUnicodeString(std::wstring(1, ch));
  }

  void InjectBackspace()
  {
    INPUT inps[2]{};
    for (int i = 0; i < 2; ++i)
    {
      inps[i].type = INPUT_KEYBOARD;
      inps[i].ki.wVk = VK_BACK;
      inps[i].ki.dwFlags = (i == 0) ? 0 : KEYEVENTF_KEYUP;
      inps[i].ki.dwExtraInfo = kHexComposeTag;
    }
    ::SendInput(2, inps, sizeof(INPUT));
  }

  std::wstring CodepointToUtf16(uint32_t cp)
  {
    if (cp <= 0xFFFF)
    {
      if (cp >= 0xD800 && cp <= 0xDFFF)
        return L""; // surrogati non validi
      return std::wstring(1, (wchar_t)cp);
    }
    if (cp > 0x10FFFF)
      return L"";
    cp -= 0x10000;
    wchar_t hi = (wchar_t)(0xD800 + (cp >> 10));
    wchar_t lo = (wchar_t)(0xDC00 + (cp & 0x3FF));
    return std::wstring{hi, lo};
  }

  void InjectCodepoint(uint32_t cp)
  {
    auto s = CodepointToUtf16(cp);
    if (!s.empty())
      InjectUnicodeString(s);
  }

} // namespace
