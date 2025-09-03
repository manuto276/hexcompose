#include "UnicodeComposeHook.h"
#include "../util/WinUtils.h"
#include "../util/Logging.h"
#include <cwctype>

namespace hexcompose::hooks
{

  bool UnicodeComposeHook::IsHexDigit(wchar_t ch)
  {
    return (ch >= L'0' && ch <= L'9') ||
           (ch >= L'a' && ch <= L'f') ||
           (ch >= L'A' && ch <= L'F');
  }

  uint32_t UnicodeComposeHook::ParseHex(const std::wstring &s, bool &ok)
  {
    ok = false;
    if (s.empty() || s.size() > 6)
      return 0;
    uint64_t val = 0;
    for (wchar_t c : s)
    {
      val <<= 4;
      if (c >= L'0' && c <= L'9')
        val += (c - L'0');
      else if (c >= L'a' && c <= L'f')
        val += (c - L'a' + 10);
      else if (c >= L'A' && c <= L'F')
        val += (c - L'A' + 10);
      else
        return 0;
      if (val > 0x10FFFFull)
        return 0;
    }
    if (val >= 0xD800 && val <= 0xDFFF)
      return 0; // surrogati non validi
    ok = true;
    return static_cast<uint32_t>(val);
  }

  bool UnicodeComposeHook::IsCtrl(const BYTE ks[256]) { return (ks[VK_CONTROL] & 0x80) || (ks[VK_LCONTROL] & 0x80) || (ks[VK_RCONTROL] & 0x80); }
  bool UnicodeComposeHook::IsShift(const BYTE ks[256]) { return (ks[VK_SHIFT] & 0x80) || (ks[VK_LSHIFT] & 0x80) || (ks[VK_RSHIFT] & 0x80); }
  bool UnicodeComposeHook::IsAlt(const BYTE ks[256]) { return (ks[VK_MENU] & 0x80) || (ks[VK_LMENU] & 0x80) || (ks[VK_RMENU] & 0x80); }
  bool UnicodeComposeHook::IsWin(const BYTE ks[256]) { return (ks[VK_LWIN] & 0x80) || (ks[VK_RWIN] & 0x80); }

  void UnicodeComposeHook::start(HWND fg)
  {
    isActive_ = true;
    hexBuffer_.clear();
    startWnd_ = fg;
    lastInput_ = Clock::now();
    blockNextKeyUpVK_ = 'U'; // evitiamo di consegnare il KeyUp di U senza il corrispondente KeyDown
    hexcompose::log::debug(L"Unicode mode: START");
  }

  void UnicodeComposeHook::cancel()
  {
    if (isActive_)
    {
      hexcompose::log::debug(L"Unicode mode: CANCEL");
    }
    isActive_ = false;
    hexBuffer_.clear();
    startWnd_ = nullptr;
    blockNextKeyUpVK_ = -1;
  }

  void UnicodeComposeHook::touch()
  {
    lastInput_ = Clock::now();
  }

  void UnicodeComposeHook::backspaceOne()
  {
    if (!hexBuffer_.empty())
    {
      hexBuffer_.pop_back();
    }
    touch();
  }

  void UnicodeComposeHook::commitIfValid()
  {
    bool ok = false;
    uint32_t cp = ParseHex(hexBuffer_, ok);
    if (ok)
    {
      win::InjectCodepoint(cp);
      hexcompose::log::debug(L"Unicode mode: COMMIT");
    }
    else
    {
      hexcompose::log::debug(L"Unicode mode: INVALID -> CANCEL");
    }
    cancel();
  }

  bool UnicodeComposeHook::handleBackslashLike(const KeyEvent &ev, bool keyDown)
  {
    // Consideriamo sia VK_OEM_5 sia VK_OEM_102 come possibili tasti '\'
    if (KeyIs(ev, VK_OEM_5) || KeyIs(ev, VK_OEM_102))
    {
      if (keyDown)
      {
        if (IsCtrl(ev.kbdState))
          cancel();
        else
          backspaceOne();
      }
      return true;
    }
    // Valuta il carattere prodotto per intercettare '\' su layout non standard
    if (keyDown)
    {
      wchar_t out[4] = {0};
      int r = win::KeyToUnicode(ev.layout, ev.kbdState, ev.kbd, out, 4);
      if (r > 0 && out[0] == L'\\')
      {
        if (IsCtrl(ev.kbdState))
          cancel();
        else
          backspaceOne();
        return true;
      }
    }
    // Supporto anche Backspace come “back one” (Ctrl+Backspace = cancel)
    if (KeyIs(ev, VK_BACK))
    {
      if (keyDown)
      {
        if (IsCtrl(ev.kbdState))
          cancel();
        else
          backspaceOne();
      }
      return true;
    }
    return false;
  }

  HookDecision UnicodeComposeHook::handle(const KeyEvent &ev)
  {
    // Ignora eventi iniettati (evita ricorsione)
    if ((ev.kbd.flags & LLKHF_INJECTED) != 0)
    {
      return HookDecision::Pass;
    }

    const bool keyDown = (ev.wparam == WM_KEYDOWN || ev.wparam == WM_SYSKEYDOWN);
    const bool keyUp = (ev.wparam == WM_KEYUP || ev.wparam == WM_SYSKEYUP);

    // --- Attivazione: Ctrl+Shift+U senza Alt né Win ---
    if (!isActive_)
    {
      if (keyDown && KeyIs(ev, 'U'))
      {
        if (IsCtrl(ev.kbdState) && IsShift(ev.kbdState) && !IsAlt(ev.kbdState) && !IsWin(ev.kbdState))
        {
          start(GetForegroundWindow());
          return HookDecision::Block; // non digitare 'U'
        }
      }
      return HookDecision::Pass;
    }

    // --- Modalità attiva: controlli di sicurezza globali ---
    if (timeouted())
    {
      cancel();
      return HookDecision::Pass;
    }
    if (GetForegroundWindow() != startWnd_)
    {
      cancel();
      return HookDecision::Pass;
    }
    if ((ev.kbd.flags & LLKHF_ALTDOWN) || IsWin(ev.kbdState))
    {
      cancel();
      return HookDecision::Pass;
    }

    // Blocca solo il prossimo KeyUp di 'U' dopo l'avvio (coerenza eventi)
    if (keyUp && ev.kbd.vkCode == (UINT)blockNextKeyUpVK_)
    {
      blockNextKeyUpVK_ = -1;
      return HookDecision::Block;
    }

    // ESC → annulla (blocchiamo sia down che up per non propagare ESC)
    if (KeyIs(ev, VK_ESCAPE))
    {
      if (keyDown)
        cancel();
      return HookDecision::Block;
    }

    // Backslash/Backspace (o char '\') → editing/cancel
    if (handleBackslashLike(ev, keyDown))
    {
      return HookDecision::Block; // consumiamo questi tasti in modalità Unicode
    }

    // Space / Enter → commit
    if (KeyIs(ev, VK_SPACE) || KeyIs(ev, VK_RETURN))
    {
      if (keyDown)
        commitIfValid();
      return HookDecision::Block;
    }

    // Solo sui KeyDown interpretiamo potenziali digit esadecimali
    if (keyDown)
    {
      wchar_t out[4] = {0};
      int r = win::KeyToUnicode(ev.layout, ev.kbdState, ev.kbd, out, 4);
      if (r > 0)
      {
        wchar_t ch = out[0];
        if (IsHexDigit(ch))
        {
          // normalizza a lower-case
          if (ch >= L'A' && ch <= L'F')
            ch = (wchar_t)(ch - L'A' + L'a');
          if ((int)hexBuffer_.size() < kMaxDigits)
          {
            hexBuffer_.push_back(ch);
          }
          touch();
          return HookDecision::Block; // consumiamo il digit
        }
        else
        {
          // Non è un hex digit → usciamo e lasciamo passare
          cancel();
          return HookDecision::Pass;
        }
      }
      else
      {
        // Tasto speciale senza char (es. F-keys): usciamo e lasciamo passare
        cancel();
        return HookDecision::Pass;
      }
    }

    // Per i KeyUp dei tasti gestiti (hex/space/enter/backslash/backspace) lasciamo passare
    return HookDecision::Pass;
  }

} // namespace
