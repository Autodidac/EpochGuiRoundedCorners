#pragma once

#include <algorithm>
#include <windows.h>
#include <uxtheme.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(value) static_cast<int>(static_cast<short>(LOWORD(value)))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(value) static_cast<int>(static_cast<short>(HIWORD(value)))
#endif
