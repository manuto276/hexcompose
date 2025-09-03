#include "HookManager.h"

namespace hexcompose::hooks
{

  void HookManager::addModule(std::unique_ptr<HookModule> m)
  {
    if (m)
    {
      m->onInstall();
      modules_.push_back(std::move(m));
    }
  }

  HookDecision HookManager::dispatch(const KeyEvent &ev)
  {
    for (auto &m : modules_)
    {
      HookDecision d = m->handle(ev);
      if (d == HookDecision::Block)
        return d;
    }
    return HookDecision::Pass;
  }

} // namespace
