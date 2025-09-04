#include "HexComposeApp.h"
#include "util/Logging.h"
#include "HexComposeVersion.h"
#include <windows.h>

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR /*cmdLine*/, int)
{
  std::wstring banner = L"HexCompose " + hexcompose::log::w(HEXCOMPOSE_VERSION) +
                        L" by " + hexcompose::log::w(HEXCOMPOSE_AUTHOR);
  hexcompose::log::debug(banner);
  hexcompose::log::debug(L"HexCompose starting...");

  hexcompose::HexComposeApp app;
  int code = app.run();

  hexcompose::log::debug(L"HexCompose exiting...");
  return code;
}
