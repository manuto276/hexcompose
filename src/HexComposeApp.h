#pragma once
#include <windows.h>
#include <memory>
#include "hooks/HookManager.h"

namespace hexcompose
{

  class HexComposeApp
  {
  public:
    HexComposeApp();
    ~HexComposeApp();

    int run();

  private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    void installHook();
    void uninstallHook();

    static HexComposeApp *s_instance;
    HHOOK hook_ = nullptr;
    std::unique_ptr<hooks::HookManager> manager_;
  };

} // namespace
