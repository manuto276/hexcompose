#include "CapsAccentsHook.h"
#include "../util/WinUtils.h"
#include "../util/Logging.h"
#include <cwctype>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace hexcompose::hooks
{

  HookDecision CapsAccentsHook::handle(const KeyEvent &ev)
  {
    // Ignora keyup, eventi non-azione o eventi che non generano caratteri.
    if (!(ev.wparam == WM_KEYDOWN || ev.wparam == WM_SYSKEYDOWN))
    {
      return HookDecision::Pass;
    }

    // Ignora eventi iniettati da noi (VK_PACKET/flag injected); verranno marcati dal dwExtraInfo.
    if ((ev.kbd.flags & LLKHF_INJECTED) != 0)
    {
      return HookDecision::Pass;
    }

    // Considera solo quando Caps è attivo
    if (!ev.capsOn)
    {
      // Reset tracking dead key se necessario
      deadKeyWasPressed_ = false;
      return HookDecision::Pass;
    }

    wchar_t outBuf[4] = {0};
    int rc = win::KeyToUnicode(ev.layout, ev.kbdState, ev.kbd, outBuf, 4);

    if (rc == -1)
    {
      // Questo tasto è un dead key -> il prossimo tasto produrrà il carattere combinato
      deadKeyWasPressed_ = true;
      return HookDecision::Pass;
    }

    if (rc < 1)
    {
      // Nessun carattere generato (tasto funzione, ecc.)
      deadKeyWasPressed_ = false;
      return HookDecision::Pass;
    }

    // rc >= 1 -> abbiamo almeno un code unit
    std::wstring produced(outBuf, outBuf + rc);
    // Uppercase in base al layout
    std::wstring upper = win::ToUpperForLayout(produced.c_str(), (int)produced.size(), ev.layout);

    // Interveniamo solo se il mapping è 1:1 e cambia davvero (esclude casi ß->SS ecc.)
    if (upper.size() == 1 && produced.size() == 1)
    {
      wchar_t ch = produced[0];
      wchar_t up = upper[0];

      // iswlower intercetta anche lettere accentate minuscole in Unicode.
      if (std::iswalpha(ch) && std::iswlower(ch) && up != ch)
      {
        if (deadKeyWasPressed_)
        {
          // Caso "dead key + base": lasciamo che Windows immetta il char minuscolo,
          // poi subito dopo iniettiamo BACKSPACE + versione maiuscola.
          // Lo facciamo su un thread separato con micro-delay per stare in coda.
          std::thread([up]()
                      {
                    // un piccolo delay per lasciare che WM_CHAR venga consegnato
                    std::this_thread::sleep_for(2ms);
                    win::InjectBackspace();
                    win::InjectUnicodeChar(up); })
              .detach();

          deadKeyWasPressed_ = false;
          return HookDecision::Pass; // non bloccare l'originale, lo correggeremo
        }
        else
        {
          // Tasto "diretto" (non parte di dead sequence): blocca e sostituisci
          win::InjectUnicodeChar(up);
          return HookDecision::Block;
        }
      }
    }

    deadKeyWasPressed_ = false;
    return HookDecision::Pass;
  }

} // namespace
