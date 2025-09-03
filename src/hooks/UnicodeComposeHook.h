#pragma once
#include "HookModule.h"
#include <string>
#include <cstdint>
#include <chrono>
#include <windows.h>

namespace hexcompose::hooks
{
  class UnicodeComposeHook : public HookModule
  {
  public:
    HookDecision handle(const KeyEvent &ev) override;

  private:
    // Stato
    bool isActive_ = false;
    std::wstring hexBuffer_;
    HWND startWnd_ = nullptr;
    int blockNextKeyUpVK_ = -1;

    using Clock = std::chrono::steady_clock;
    Clock::time_point lastInput_;
    static constexpr std::chrono::milliseconds kTimeout{5000};
    static constexpr int kMaxDigits = 6;

    // Helper
    static bool IsHexDigit(wchar_t ch);
    static uint32_t ParseHex(const std::wstring &s, bool &ok);
    static bool IsCtrl(const BYTE ks[256]);
    static bool IsShift(const BYTE ks[256]);
    static bool IsAlt(const BYTE ks[256]);
    static bool IsWin(const BYTE ks[256]);

    static bool KeyIs(const KeyEvent &ev, int vk) { return ev.kbd.vkCode == (UINT)vk; }

    // Azioni stato
    void start(HWND fg);
    void cancel();
    void touch();
    void backspaceOne();

    void commitIfValid();
    bool handleBackslashLike(const KeyEvent &ev, bool keyDown);
    bool timeouted() const { return isActive_ && (Clock::now() - lastInput_ > kTimeout); }
  };

} // namespace
