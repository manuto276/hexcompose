#pragma once
#include "HookModule.h"

namespace hexcompose::hooks
{

  // Modulo: se CapsLock è attivo e il tasto produce una lettera minuscola (incluse accentate),
  // emette la versione maiuscola secondo il locale del layout corrente.
  // Nota: gestisce sia tasti "diretti" sia sequenze con dead key in modo best-effort.
  class CapsAccentsHook : public HookModule
  {
  public:
    HookDecision handle(const KeyEvent &ev) override;

  private:
    bool deadKeyWasPressed_ = false; // semplice tracking del dead key precedente
  };

} // namespace
