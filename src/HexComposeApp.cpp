#include "HexComposeApp.h"
#include "util/Logging.h"
#include "util/WinUtils.h"
#include "hooks/CapsAccentsHook.h"
#include <memory>

using hexcompose::hooks::HookDecision;
using hexcompose::hooks::KeyEvent;

namespace hexcompose
{

  HexComposeApp *HexComposeApp::s_instance = nullptr;

  HexComposeApp::HexComposeApp() : manager_(std::make_unique<hooks::HookManager>())
  {
    s_instance = this;
    // Registra moduli (in futuro potrai aggiungerne altri)
    manager_->addModule(std::make_unique<hooks::CapsAccentsHook>());

    installHook();
  }

  HexComposeApp::~HexComposeApp()
  {
    uninstallHook();
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

      // Ignora eventi iniettati da noi esplicitamente
      if ((k->flags & LLKHF_INJECTED) && k->dwExtraInfo == hexcompose::win::kHexComposeTag)
      {
        return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
      }

      // Prepara KeyEvent
      KeyEvent ev{};
      ev.wparam = wParam;
      ev.kbd = *k;
      ev.layout = hexcompose::win::GetForegroundKeyboardLayout();
      ev.capsOn = hexcompose::win::IsCapsLockOn();
      hexcompose::win::GetKeyboardStateSnapshot(ev.kbdState, wParam, *k);

      HookDecision d = s_instance->manager_->dispatch(ev);
      if (d == HookDecision::Block)
      {
        return 1; // sopprime l'evento originale
      }
    }
    return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  int HexComposeApp::run()
  {
    // Message loop "nascosto" (nessuna finestra): l'hook vive qui.
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
  }

} // namespace
