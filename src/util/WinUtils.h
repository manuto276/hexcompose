#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace hexcompose::win
{

  constexpr ULONG_PTR kHexComposeTag = 0x48455843ull; // 'HEXC'

  HKL GetForegroundKeyboardLayout();
  bool IsCapsLockOn();
  void GetKeyboardStateSnapshot(BYTE out[256], WPARAM wparam, const KBDLLHOOKSTRUCT &kbdStruct);

  std::wstring ToUpperForLayout(const wchar_t *input, int len, HKL hkl);
  int KeyToUnicode(HKL hkl, const BYTE kbdState[256], const KBDLLHOOKSTRUCT &kbd, wchar_t *outBuf, int outBufLen);

  void InjectUnicodeString(const std::wstring &s);
  void InjectUnicodeChar(wchar_t ch);
  void InjectBackspace();

  // Nuovo: inietta un code point Unicode (gestisce anche fuori BMP con surrogati)
  void InjectCodepoint(uint32_t cp);

  // Helper UTF-16 da codepoint
  std::wstring CodepointToUtf16(uint32_t cp);

} // namespace
