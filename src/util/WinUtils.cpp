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
    // Manteniamo consistente lo stato della key corrente
    if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
    {
      out[k.vkCode] |= 0x80;
    }
    else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)
    {
      out[k.vkCode] &= ~0x80;
    }
    // Se il flag ALTDOWN è settato nel KBDLLHOOKSTRUCT, riflettilo
    if (k.flags & LLKHF_ALTDOWN)
      out[VK_MENU] |= 0x80;
  }

  std::wstring ToUpperForLayout(const wchar_t *input, int len, HKL hkl)
  {
    if (len <= 0)
      return L"";
    // Ricava locale name da HKL
    WCHAR localeName[LOCALE_NAME_MAX_LENGTH] = {0};
    LCID lcid = MAKELCID(LOWORD((UINT_PTR)hkl), SORT_DEFAULT);
    int ok = ::LCIDToLocaleName(lcid, localeName, LOCALE_NAME_MAX_LENGTH, 0);
    if (ok == 0)
    {
      // fallback: user default
      lstrcpyW(localeName, LOCALE_NAME_USER_DEFAULT);
    }
    // Uppercase
    wchar_t out[8] = {0};
    int r = ::LCMapStringEx(localeName, LCMAP_UPPERCASE, input, len, out, 8, nullptr, nullptr, 0);
    if (r > 0)
      return std::wstring(out, out + r);
    // Fallback a towupper su singolo char
    if (len == 1)
    {
      wchar_t c = towupper(*input);
      return std::wstring(1, c);
    }
    return std::wstring(input, input + len);
  }

  int KeyToUnicode(HKL hkl, const BYTE kbdState[256], const KBDLLHOOKSTRUCT &kbd, wchar_t *outBuf, int outBufLen)
  {
    // flags per ToUnicodeEx: 0 (se necessario si potrebbero usare UI flags)
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
      down.ki.time = 0;
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
      inps[i].ki.wScan = 0;
      inps[i].ki.dwFlags = (i == 0) ? 0 : KEYEVENTF_KEYUP;
      inps[i].ki.dwExtraInfo = kHexComposeTag;
    }
    ::SendInput(2, inps, sizeof(INPUT));
  }

} // namespace
