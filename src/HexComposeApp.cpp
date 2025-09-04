#include "HexComposeApp.h"
#include "util/Logging.h"
#include "util/WinUtils.h"
#include "util/TrayIcon.h"
#include "hooks/CapsAccentsHook.h"
#include "hooks/UnicodeComposeHook.h"
#include <memory>

using hexcompose::hooks::HookDecision;
using hexcompose::hooks::KeyEvent;

namespace hexcompose
{

  HexComposeApp *HexComposeApp::s_instance = nullptr;

  HexComposeApp::HexComposeApp() : manager_(std::make_unique<hooks::HookManager>())
  {
    s_instance = this;

    // Tray icon con menu Exit
    tray_ = std::make_unique<util::TrayIcon>();
    if (!tray_->init(L"HexCompose (Ctrl+Shift+U)"))
    {
      hexcompose::log::debug(L"Tray icon init failed");
    }

    // Ordine: prima UnicodeCompose (consuma i tasti della modalità), poi gli altri
    manager_->addModule(std::make_unique<hooks::UnicodeComposeHook>());
    manager_->addModule(std::make_unique<hooks::CapsAccentsHook>());

    installHook();
  }

  HexComposeApp::~HexComposeApp()
  {
    uninstallHook();
    tray_.reset();
    s_instance = nullptr;
  }

  void HexComposeApp::installHook()
  {
    hook_ = ::SetWindowsHookExW(WH_KEYBOARD_LL, &HexComposeApp::LowLevelKeyboardProc, nullptr, 0);
    if (!hook_)
    {
      hexcompose::log::debug(L"SetWindowsHookExW(WH_KEYBOARD_LL) FAILED");
    }
    else
    {
      hexcompose::log::debug(L"Keyboard LL hook installed");
    }
  }

  void HexComposeApp::uninstallHook()
  {
    if (hook_)
    {
      ::UnhookWindowsHookEx(hook_);
      hook_ = nullptr;
      hexcompose::log::debug(L"Keyboard LL hook uninstalled");
    }
  }

  LRESULT CALLBACK HexComposeApp::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
  {
    if (nCode == HC_ACTION && s_instance && s_instance->manager_)
    {
      const KBDLLHOOKSTRUCT *k = reinterpret_cast<const KBDLLHOOKSTRUCT *>(lParam);

      // Ignora input iniettato da noi (marcato)
      if ((k->flags & LLKHF_INJECTED) && k->dwExtraInfo == hexcompose::win::kHexComposeTag)
      {
        return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
      }

      // Snapshot evento
      KeyEvent ev{};
      ev.wparam = wParam;
      ev.kbd = *k;
      ev.layout = hexcompose::win::GetForegroundKeyboardLayout();
      ev.capsOn = hexcompose::win::IsCapsLockOn();
      hexcompose::win::GetKeyboardStateSnapshot(ev.kbdState, wParam, *k);

      // PANIC HOTKEY: Ctrl+Shift+Pause/Break → chiudi app
      const bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
      if (keyDown && k->vkCode == VK_CANCEL)
      { // Pause/Break
        bool ctrl = (ev.kbdState[VK_CONTROL] & 0x80) || (ev.kbdState[VK_LCONTROL] & 0x80) || (ev.kbdState[VK_RCONTROL] & 0x80);
        bool shift = (ev.kbdState[VK_SHIFT] & 0x80) || (ev.kbdState[VK_LSHIFT] & 0x80) || (ev.kbdState[VK_RSHIFT] & 0x80);
        bool alt = (ev.kbdState[VK_MENU] & 0x80) || (ev.kbdState[VK_LMENU] & 0x80) || (ev.kbdState[VK_RMENU] & 0x80);
        if (ctrl && shift && !alt)
        {
          hexcompose::log::debug(L"PANIC: quitting HexCompose");
          PostQuitMessage(0);
          return 1;
        }
      }

      HookDecision d = s_instance->manager_->dispatch(ev);
      if (d == HookDecision::Block)
      {
        return 1;
      }
    }
    return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  int HexComposeApp::run()
  {
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
  }

} // namespace
