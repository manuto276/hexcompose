#pragma once
#include <windows.h>

namespace hexcompose::hooks
{

  struct KeyEvent
  {
    WPARAM wparam;
    KBDLLHOOKSTRUCT kbd;
    HKL layout;
    bool capsOn;
    BYTE kbdState[256];
  };

  enum class HookDecision
  {
    Pass, // lascia passare l'evento
    Block // blocca l'evento originale (già gestito dal modulo)
  };

  class HookModule
  {
  public:
    virtual ~HookModule() = default;
    virtual void onInstall() {}
    virtual void onUninstall() {}
    virtual HookDecision handle(const KeyEvent &ev) = 0;
  };

} // namespace
