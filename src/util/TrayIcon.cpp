#include "TrayIcon.h"
#include "../util/Logging.h"
#include "../../res/resource.h" // IDI_APPICON

namespace hexcompose::util
{

  static const wchar_t *kWndClass = L"HexComposeHiddenWndClass";

  bool TrayIcon::init(const wchar_t *tooltip)
  {
    tooltip_ = tooltip ? tooltip : L"HexCompose";

    HINSTANCE hInst = GetModuleHandleW(nullptr);

    // Carichiamo le icone di classe (con LR_SHARED, non vanno distrutte)
    UINT bigW = GetSystemMetrics(SM_CXICON);
    UINT bigH = GetSystemMetrics(SM_CYICON);
    UINT smW = GetSystemMetrics(SM_CXSMICON);
    UINT smH = GetSystemMetrics(SM_CYSMICON);

    HICON hBig = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON),
                                   IMAGE_ICON, bigW, bigH, LR_DEFAULTCOLOR | LR_SHARED);
    HICON hSm = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON),
                                  IMAGE_ICON, smW, smH, LR_DEFAULTCOLOR | LR_SHARED);

    if (!hBig)
      hBig = LoadIconW(nullptr, IDI_APPLICATION);
    if (!hSm)
      hSm = LoadIconW(nullptr, IDI_APPLICATION);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &TrayIcon::WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kWndClass;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = hBig;
    wc.hIconSm = hSm;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
      hexcompose::log::debug(L"TrayIcon: RegisterClassExW failed");
      return false;
    }

    // Finestra nascosta
    hwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW, kWndClass, L"",
        WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInst, this);

    if (!hwnd_)
    {
      hexcompose::log::debug(L"TrayIcon: CreateWindowExW failed");
      return false;
    }

    // Icona per la tray (handle "nostro": senza LR_SHARED, lo distruggiamo in shutdown)
    hIcon_ = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON),
                               IMAGE_ICON, smW, smH, LR_DEFAULTCOLOR);
    if (!hIcon_)
    {
      hIcon_ = LoadIconW(nullptr, IDI_APPLICATION);
    }

    taskbarRestartMsg_ = RegisterWindowMessageW(L"TaskbarCreated");

    if (!addIcon())
    {
      DestroyWindow(hwnd_);
      hwnd_ = nullptr;
      if (hIcon_ && hIcon_ != LoadIconW(nullptr, IDI_APPLICATION))
      {
        DestroyIcon(hIcon_);
        hIcon_ = nullptr;
      }
      return false;
    }

    return true;
  }

  void TrayIcon::shutdown()
  {
    removeIcon();
    if (hIcon_ && hIcon_ != LoadIconW(nullptr, IDI_APPLICATION))
    {
      DestroyIcon(hIcon_);
      hIcon_ = nullptr;
    }
    if (hwnd_)
    {
      DestroyWindow(hwnd_);
      hwnd_ = nullptr;
    }
  }

  bool TrayIcon::addIcon()
  {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = kTrayId;
    nid.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
    nid.uCallbackMessage = kTrayMessage;
    nid.hIcon = hIcon_;
    wcsncpy_s(nid.szTip, tooltip_.c_str(), _TRUNCATE);

    if (!Shell_NotifyIconW(NIM_ADD, &nid))
    {
      hexcompose::log::debug(L"TrayIcon: Shell_NotifyIcon(NIM_ADD) failed");
      return false;
    }

    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &nid);
    return true;
  }

  void TrayIcon::removeIcon()
  {
    if (!hwnd_)
      return;
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd_;
    nid.uID = kTrayId;
    Shell_NotifyIconW(NIM_DELETE, &nid);
  }

  void TrayIcon::showContextMenu(POINT pt)
  {
    HMENU menu = CreatePopupMenu();
    if (!menu)
      return;

    AppendMenuW(menu, MF_ENABLED | MF_STRING, kCmdExit, L"Exit");

    SetForegroundWindow(hwnd_);
    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd_, nullptr);
    PostMessageW(hwnd_, WM_NULL, 0, 0);

    DestroyMenu(menu);
  }

  void TrayIcon::rebuildAfterTaskbarRestart()
  {
    removeIcon();
    addIcon();
  }

  LRESULT CALLBACK TrayIcon::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    TrayIcon *self = reinterpret_cast<TrayIcon *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
      auto cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
      SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
      return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    if (!self)
    {
      return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    if (self->taskbarRestartMsg_ != 0 && msg == self->taskbarRestartMsg_)
    {
      self->rebuildAfterTaskbarRestart();
      return 0;
    }

    switch (msg)
    {
    case kTrayMessage:
    {
      switch (LOWORD(lParam))
      {
      case WM_RBUTTONUP:
      case WM_CONTEXTMENU:
      {
        POINT pt{};
        GetCursorPos(&pt);
        self->showContextMenu(pt);
        return 0;
      }
      case WM_LBUTTONDBLCLK:
      case WM_LBUTTONUP:
        return 0;
      }
    }
    break;

    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case kCmdExit:
        PostQuitMessage(0);
        return 0;
      }
    }
    break;

    case WM_DESTROY:
      self->removeIcon();
      break;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
  }

} // namespace hexcompose::util
