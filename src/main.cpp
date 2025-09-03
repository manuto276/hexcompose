#include "HexComposeApp.h"
#include "util/Logging.h"
#include <windows.h>

// Facoltativo: opzioni da riga di comando in futuro (es. --install-startup / --remove-startup).
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR /*cmdLine*/, int)
{
  hexcompose::log::debug(L"HexCompose starting...");
  hexcompose::HexComposeApp app;
  int code = app.run();
  hexcompose::log::debug(L"HexCompose exiting...");
  return code;
}
