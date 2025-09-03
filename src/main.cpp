#include "HexComposeApp.h"
#include "util/Logging.h"
#include <windows.h>

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR /*cmdLine*/, int)
{
  hexcompose::log::debug(L"HexCompose starting...");
  hexcompose::HexComposeApp app;
  int code = app.run();
  hexcompose::log::debug(L"HexCompose exiting...");
  return code;
}
