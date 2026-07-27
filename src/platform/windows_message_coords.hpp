#pragma once

#include <algorithm>
#include <windows.h>
#include <uxtheme.h>

[[nodiscard]] inline HRESULT epoch_gui_demo_restore_window_theme(HWND window) noexcept
{
    return ::SetWindowTheme(window, nullptr, nullptr);
}

#define SetWindowTheme(window, sub_application, sub_id) \
    epoch_gui_demo_restore_window_theme(window)

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(value) static_cast<int>(static_cast<short>(LOWORD(value)))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(value) static_cast<int>(static_cast<short>(HIWORD(value)))
#endif
