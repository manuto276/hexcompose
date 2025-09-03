#pragma once
#include "HookModule.h"
#include <memory>
#include <vector>

namespace hexcompose::hooks
{

  class HookManager
  {
  public:
    void addModule(std::unique_ptr<HookModule> m);
    HookDecision dispatch(const KeyEvent &ev);

  private:
    std::vector<std::unique_ptr<HookModule>> modules_;
  };

} // namespace
