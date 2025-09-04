#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>

namespace hexcompose::util
{

  class TrayIcon
  {
  public:
    TrayIcon() = default;
    ~TrayIcon() { shutdown(); }

    // Crea finestra nascosta e aggiunge l'icona di tray
    bool init(const wchar_t *tooltip = L"HexCompose (Ctrl+Shift+U)");

    // Rimuove icona e distrugge finestra
    void shutdown();

  private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool addIcon();
    void removeIcon();
    void showContextMenu(POINT pt);
    void rebuildAfterTaskbarRestart();

    HWND hwnd_ = nullptr;
    HICON hIcon_ = nullptr;
    UINT taskbarRestartMsg_ = 0;
    static constexpr UINT kTrayMessage = WM_APP + 1;
    static constexpr UINT kTrayId = 1;
    static constexpr UINT kCmdExit = 1001;
    std::wstring tooltip_;
  };

} // namespace hexcompose::util
