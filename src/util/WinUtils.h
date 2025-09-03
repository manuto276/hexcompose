#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace hexcompose::win
{

  constexpr ULONG_PTR kHexComposeTag = 0x48455843ull; // 'HEXC' per dwExtraInfo

  // HKL del thread owner della finestra in foreground (fallback: layout corrente del processo)
  HKL GetForegroundKeyboardLayout();

  // Caps Lock toggled?
  bool IsCapsLockOn();

  // Snapshot dello stato tastiera (256) con eventuale key corrente già marcata "down"
  void GetKeyboardStateSnapshot(BYTE out[256], WPARAM wparam, const KBDLLHOOKSTRUCT &kbdStruct);

  // Uppercasing Unicode in base al layout (locale) associato all'HKL.
  // Può restituire più code unit (es. particolarità come ß->SS); per i nostri scopi
  // se >1 char preferiamo NON intervenire.
  std::wstring ToUpperForLayout(const wchar_t *input, int len, HKL hkl);

  // Traduce un VK/scanCode in Unicode con ToUnicodeEx (ritorna count come ToUnicodeEx).
  int KeyToUnicode(HKL hkl, const BYTE kbdState[256], const KBDLLHOOKSTRUCT &kbd, wchar_t *outBuf, int outBufLen);

  // Iniezioni testuali sicure (KEYEVENTF_UNICODE) marcate con dwExtraInfo = kHexComposeTag.
  void InjectUnicodeString(const std::wstring &s);
  void InjectUnicodeChar(wchar_t ch);
  void InjectBackspace();

} // namespace
