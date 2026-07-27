#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>

#include <GL/gl.h>

#include "epochgui_demo/app_bridge.hpp"

#include <array>
#include <cstdint>
#include <cstring>

namespace
{
    constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
    constexpr int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
    constexpr int WGL_CONTEXT_FLAGS_ARB = 0x2094;
    constexpr int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
    constexpr int WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x0002;
    constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;
    constexpr UINT apply_native_frame_message = WM_APP + 1U;

    constexpr DWORD framed_style = WS_OVERLAPPEDWINDOW;
    constexpr DWORD borderless_style = WS_POPUP
        | WS_THICKFRAME
        | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX
        | WS_SYSMENU;

    using WglCreateContextAttribs = HGLRC(WINAPI*)(HDC, HGLRC, const int*);
    using WglSwapInterval = BOOL(WINAPI*)(int);

    [[nodiscard]] bool invalid_wgl_address(epoch_gui_demo_gl_proc address) noexcept
    {
        const auto value = reinterpret_cast<std::uintptr_t>(address);
        return address == nullptr
            || value == 1U
            || value == 2U
            || value == 3U
            || value == static_cast<std::uintptr_t>(-1);
    }

    [[nodiscard]] epoch_gui_demo_gl_proc system_opengl_proc(const char* name) noexcept
    {
        if (std::strcmp(name, "glClearColor") == 0)
            return reinterpret_cast<epoch_gui_demo_gl_proc>(&::glClearColor);
        if (std::strcmp(name, "glClear") == 0)
            return reinterpret_cast<epoch_gui_demo_gl_proc>(&::glClear);
        if (std::strcmp(name, "glViewport") == 0)
            return reinterpret_cast<epoch_gui_demo_gl_proc>(&::glViewport);
        if (std::strcmp(name, "glDrawArrays") == 0)
            return reinterpret_cast<epoch_gui_demo_gl_proc>(&::glDrawArrays);
        return nullptr;
    }

    [[nodiscard]] epoch_gui_demo_gl_proc load_opengl_proc(const char* name) noexcept
    {
        if (!name)
            return nullptr;
        if (const auto system_proc = system_opengl_proc(name))
            return system_proc;

        const auto address = reinterpret_cast<epoch_gui_demo_gl_proc>(wglGetProcAddress(name));
        return invalid_wgl_address(address) ? nullptr : address;
    }

    [[nodiscard]] int key_from_virtual_key(WPARAM key) noexcept
    {
        switch (key)
        {
        case VK_ESCAPE: return EPOCH_GUI_DEMO_KEY_ESCAPE;
        case VK_RETURN: return EPOCH_GUI_DEMO_KEY_ENTER;
        case VK_TAB: return EPOCH_GUI_DEMO_KEY_TAB;
        case VK_SPACE: return EPOCH_GUI_DEMO_KEY_SPACE;
        case VK_BACK: return EPOCH_GUI_DEMO_KEY_BACKSPACE;
        case VK_DELETE: return EPOCH_GUI_DEMO_KEY_DELETE;
        case VK_LEFT: return EPOCH_GUI_DEMO_KEY_LEFT;
        case VK_RIGHT: return EPOCH_GUI_DEMO_KEY_RIGHT;
        case VK_UP: return EPOCH_GUI_DEMO_KEY_UP;
        case VK_DOWN: return EPOCH_GUI_DEMO_KEY_DOWN;
        case VK_HOME: return EPOCH_GUI_DEMO_KEY_HOME;
        case VK_END: return EPOCH_GUI_DEMO_KEY_END;
        case VK_SHIFT: return EPOCH_GUI_DEMO_KEY_SHIFT;
        case VK_CONTROL: return EPOCH_GUI_DEMO_KEY_CONTROL;
        case VK_MENU: return EPOCH_GUI_DEMO_KEY_ALT;
        case VK_LWIN:
        case VK_RWIN:
            return EPOCH_GUI_DEMO_KEY_SUPER;
        default:
            return -1;
        }
    }

    [[nodiscard]] LRESULT native_hit_test_from(int region) noexcept
    {
        switch (region)
        {
        case EPOCH_GUI_DEMO_WINDOW_CAPTION: return HTCAPTION;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_LEFT: return HTLEFT;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_RIGHT: return HTRIGHT;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP: return HTTOP;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM: return HTBOTTOM;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP_LEFT: return HTTOPLEFT;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP_RIGHT: return HTTOPRIGHT;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM_LEFT: return HTBOTTOMLEFT;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM_RIGHT: return HTBOTTOMRIGHT;
        default:
            return HTCLIENT;
        }
    }

    class WindowsApplication final
    {
    public:
        ~WindowsApplication()
        {
            shutdown_graphics();
        }

        [[nodiscard]] bool initialize(HINSTANCE instance)
        {
            instance_ = instance;
            constexpr wchar_t class_name[] = L"EpochGuiCompleteDemoOpenGL";

            WNDCLASSEXW window_class{};
            window_class.cbSize = sizeof(window_class);
            window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            window_class.lpfnWndProc = &WindowsApplication::window_proc;
            window_class.hInstance = instance_;
            window_class.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
            window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            window_class.lpszClassName = class_name;
            window_class.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

            if (!RegisterClassExW(&window_class)
                && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            {
                return false;
            }

            RECT frame{ 0, 0, 1280, 820 };
            if (!AdjustWindowRectEx(&frame, framed_style, FALSE, 0))
                return false;

            const int width = frame.right - frame.left;
            const int height = frame.bottom - frame.top;
            const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
            const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

            window_ = CreateWindowExW(
                0,
                class_name,
                L"EpochGui Complete Demo",
                framed_style,
                x,
                y,
                width,
                height,
                nullptr,
                nullptr,
                instance_,
                this);
            if (!window_)
                return false;

            apply_dwm_frame();
            device_context_ = GetDC(window_);
            if (!device_context_ || !create_opengl_context())
                return false;

            renderer_ = epoch_gui_demo_create();
            if (!renderer_ || !epoch_gui_demo_initialize(renderer_, &load_opengl_proc))
                return false;

            update_renderer_size_from_client();

            ShowWindow(window_, SW_SHOWNORMAL);
            UpdateWindow(window_);
            render_and_apply_commands();
            return true;
        }

        [[nodiscard]] int run()
        {
            MSG message{};
            while (GetMessageW(&message, nullptr, 0, 0) > 0)
            {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            return static_cast<int>(message.wParam);
        }

    private:
        [[nodiscard]] bool create_opengl_context()
        {
            PIXELFORMATDESCRIPTOR descriptor{};
            descriptor.nSize = sizeof(descriptor);
            descriptor.nVersion = 1;
            descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            descriptor.iPixelType = PFD_TYPE_RGBA;
            descriptor.cColorBits = 32;
            descriptor.cAlphaBits = 8;
            descriptor.cDepthBits = 24;
            descriptor.cStencilBits = 8;
            descriptor.iLayerType = PFD_MAIN_PLANE;

            const int pixel_format = ChoosePixelFormat(device_context_, &descriptor);
            if (pixel_format == 0 || !SetPixelFormat(device_context_, pixel_format, &descriptor))
                return false;

            HGLRC legacy_context = wglCreateContext(device_context_);
            if (!legacy_context || !wglMakeCurrent(device_context_, legacy_context))
            {
                if (legacy_context)
                    wglDeleteContext(legacy_context);
                return false;
            }

            const auto create_context = reinterpret_cast<WglCreateContextAttribs>(
                wglGetProcAddress("wglCreateContextAttribsARB"));
            if (!create_context)
            {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(legacy_context);
                return false;
            }

            constexpr int attributes[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };

            render_context_ = create_context(device_context_, nullptr, attributes);
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(legacy_context);
            if (!render_context_ || !wglMakeCurrent(device_context_, render_context_))
                return false;

            const auto swap_interval = reinterpret_cast<WglSwapInterval>(
                wglGetProcAddress("wglSwapIntervalEXT"));
            if (swap_interval)
                swap_interval(1);
            return true;
        }

        void shutdown_graphics()
        {
            if (graphics_shutdown_)
                return;
            graphics_shutdown_ = true;

            if (render_context_ && device_context_)
                wglMakeCurrent(device_context_, render_context_);

            epoch_gui_demo_destroy(renderer_);
            renderer_ = nullptr;

            wglMakeCurrent(nullptr, nullptr);
            if (render_context_)
            {
                wglDeleteContext(render_context_);
                render_context_ = nullptr;
            }
            if (device_context_ && window_)
            {
                ReleaseDC(window_, device_context_);
                device_context_ = nullptr;
            }
        }

        void apply_dwm_frame()
        {
            if (!window_)
                return;

            const MARGINS margins = native_frame_enabled_
                ? MARGINS{ 0, 0, 0, 0 }
                : MARGINS{ 1, 1, 1, 1 };
            DwmExtendFrameIntoClientArea(window_, &margins);
        }

        void update_renderer_size_from_client()
        {
            if (!renderer_ || !window_)
                return;

            RECT client{};
            if (GetClientRect(window_, &client))
            {
                epoch_gui_demo_resize(
                    renderer_,
                    (std::max)(1L, client.right - client.left),
                    (std::max)(1L, client.bottom - client.top));
            }
        }

        void request_native_frame_mode(bool enabled)
        {
            if (!window_ || graphics_shutdown_)
                return;

            pending_native_frame_enabled_ = enabled;
            if (frame_change_message_pending_)
                return;

            frame_change_message_pending_ = true;
            if (!PostMessageW(window_, apply_native_frame_message, 0, 0))
                frame_change_message_pending_ = false;
        }

        void apply_native_frame_mode(bool enabled)
        {
            if (!window_
                || graphics_shutdown_
                || frame_change_in_progress_
                || native_frame_enabled_ == enabled)
            {
                return;
            }

            frame_change_in_progress_ = true;
            const bool previous_mode = native_frame_enabled_;
            native_frame_enabled_ = enabled;

            SetLastError(ERROR_SUCCESS);
            const LONG_PTR previous_style = SetWindowLongPtrW(
                window_,
                GWL_STYLE,
                static_cast<LONG_PTR>(enabled ? framed_style : borderless_style));
            if (previous_style == 0 && GetLastError() != ERROR_SUCCESS)
            {
                native_frame_enabled_ = previous_mode;
                frame_change_in_progress_ = false;
                return;
            }

            apply_dwm_frame();
            SetWindowPos(
                window_,
                nullptr,
                0,
                0,
                0,
                0,
                SWP_NOMOVE
                    | SWP_NOSIZE
                    | SWP_NOZORDER
                    | SWP_NOACTIVATE
                    | SWP_FRAMECHANGED);

            update_renderer_size_from_client();
            frame_change_in_progress_ = false;
            RedrawWindow(
                window_,
                nullptr,
                nullptr,
                RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
        }

        void update_modifiers()
        {
            if (!renderer_)
                return;
            epoch_gui_demo_modifiers(
                renderer_,
                (GetKeyState(VK_SHIFT) & 0x8000) != 0,
                (GetKeyState(VK_CONTROL) & 0x8000) != 0,
                (GetKeyState(VK_MENU) & 0x8000) != 0,
                (GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0);
        }

        void render_and_apply_commands()
        {
            if (!renderer_
                || !device_context_
                || rendering_
                || frame_change_in_progress_
                || graphics_shutdown_)
            {
                return;
            }

            rendering_ = true;
            epoch_gui_demo_render(renderer_);
            SwapBuffers(device_context_);
            const int window_command = epoch_gui_demo_take_window_command(renderer_);
            const int frame_mode = epoch_gui_demo_take_native_frame_mode(renderer_);
            rendering_ = false;

            switch (window_command)
            {
            case EPOCH_GUI_DEMO_COMMAND_MINIMIZE:
                ShowWindow(window_, SW_MINIMIZE);
                break;
            case EPOCH_GUI_DEMO_COMMAND_TOGGLE_MAXIMIZE:
                ShowWindow(window_, IsZoomed(window_) ? SW_RESTORE : SW_MAXIMIZE);
                break;
            case EPOCH_GUI_DEMO_COMMAND_CLOSE:
                SendMessageW(window_, WM_CLOSE, 0, 0);
                break;
            default:
                break;
            }

            if (frame_mode == EPOCH_GUI_DEMO_FRAME_MODE_NATIVE)
                request_native_frame_mode(true);
            else if (frame_mode == EPOCH_GUI_DEMO_FRAME_MODE_BORDERLESS)
                request_native_frame_mode(false);
        }

        void feed_pointer_position(LPARAM lparam)
        {
            if (!renderer_)
                return;
            epoch_gui_demo_pointer_move(
                renderer_,
                static_cast<float>(GET_X_LPARAM(lparam)),
                static_cast<float>(GET_Y_LPARAM(lparam)));
        }

        void feed_pointer_button(LPARAM lparam, int button, bool down)
        {
            feed_pointer_position(lparam);
            epoch_gui_demo_pointer_button(renderer_, button, down);
            if (down)
                SetCapture(window_);
            else if (GetCapture() == window_)
                ReleaseCapture();
            render_and_apply_commands();
        }

        void feed_character(wchar_t character)
        {
            if (!renderer_ || character < 32 || character == 127)
                return;

            std::array<char, 8> utf8{};
            const int written = WideCharToMultiByte(
                CP_UTF8,
                0,
                &character,
                1,
                utf8.data(),
                static_cast<int>(utf8.size() - 1),
                nullptr,
                nullptr);
            if (written <= 0)
                return;
            utf8[static_cast<std::size_t>(written)] = '\0';
            epoch_gui_demo_text_input(renderer_, utf8.data());
            render_and_apply_commands();
        }

        [[nodiscard]] LRESULT handle_message(
            HWND window,
            UINT message,
            WPARAM wparam,
            LPARAM lparam)
        {
            switch (message)
            {
            case apply_native_frame_message:
                frame_change_message_pending_ = false;
                apply_native_frame_mode(pending_native_frame_enabled_);
                return 0;

            case WM_NCCALCSIZE:
                if (!native_frame_enabled_ && wparam != 0)
                    return 0;
                break;

            case WM_NCHITTEST:
            {
                if (frame_change_in_progress_)
                    return DefWindowProcW(window, message, wparam, lparam);

                if (native_frame_enabled_)
                {
                    const LRESULT native_result = DefWindowProcW(window, message, wparam, lparam);
                    if (native_result != HTCLIENT)
                        return native_result;
                }
                if (!renderer_)
                    break;
                POINT point{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                ScreenToClient(window, &point);
                const int region = epoch_gui_demo_window_hit_test(
                    renderer_,
                    static_cast<float>(point.x),
                    static_cast<float>(point.y));
                return native_hit_test_from(region);
            }

            case WM_GETMINMAXINFO:
            {
                auto* info = reinterpret_cast<MINMAXINFO*>(lparam);
                info->ptMinTrackSize.x = 880;
                info->ptMinTrackSize.y = 580;
                return 0;
            }

            case WM_PAINT:
            {
                PAINTSTRUCT paint{};
                BeginPaint(window, &paint);
                render_and_apply_commands();
                EndPaint(window, &paint);
                return 0;
            }

            case WM_SIZE:
                if (renderer_)
                {
                    epoch_gui_demo_resize(
                        renderer_,
                        (std::max)(1, static_cast<int>(LOWORD(lparam))),
                        (std::max)(1, static_cast<int>(HIWORD(lparam))));
                    InvalidateRect(window_, nullptr, FALSE);
                }
                return 0;

            case WM_MOUSEMOVE:
                feed_pointer_position(lparam);
                render_and_apply_commands();
                return 0;

            case WM_LBUTTONDOWN:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_LEFT, true);
                return 0;
            case WM_LBUTTONUP:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_LEFT, false);
                return 0;
            case WM_RBUTTONDOWN:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_RIGHT, true);
                return 0;
            case WM_RBUTTONUP:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_RIGHT, false);
                return 0;
            case WM_MBUTTONDOWN:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_MIDDLE, true);
                return 0;
            case WM_MBUTTONUP:
                feed_pointer_button(lparam, EPOCH_GUI_DEMO_POINTER_MIDDLE, false);
                return 0;

            case WM_MOUSEWHEEL:
            {
                POINT point{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                ScreenToClient(window, &point);
                epoch_gui_demo_pointer_move(renderer_, static_cast<float>(point.x), static_cast<float>(point.y));
                epoch_gui_demo_wheel(
                    renderer_,
                    0.0f,
                    static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<float>(WHEEL_DELTA));
                render_and_apply_commands();
                return 0;
            }

            case WM_CHAR:
                feed_character(static_cast<wchar_t>(wparam));
                return 0;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                const int key = key_from_virtual_key(wparam);
                if (key >= 0 && renderer_)
                {
                    const bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
                    const bool repeated = down && (lparam & (1LL << 30)) != 0;
                    epoch_gui_demo_key_event(renderer_, key, down, repeated);
                    update_modifiers();
                    render_and_apply_commands();
                    return 0;
                }
                break;
            }

            case WM_ERASEBKGND:
                return 1;

            case WM_CLOSE:
                shutdown_graphics();
                DestroyWindow(window);
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                break;
            }

            return DefWindowProcW(window, message, wparam, lparam);
        }

        static LRESULT CALLBACK window_proc(
            HWND window,
            UINT message,
            WPARAM wparam,
            LPARAM lparam)
        {
            WindowsApplication* application = reinterpret_cast<WindowsApplication*>(
                GetWindowLongPtrW(window, GWLP_USERDATA));

            if (message == WM_NCCREATE)
            {
                const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
                application = static_cast<WindowsApplication*>(create->lpCreateParams);
                application->window_ = window;
                SetWindowLongPtrW(
                    window,
                    GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(application));
            }

            return application
                ? application->handle_message(window, message, wparam, lparam)
                : DefWindowProcW(window, message, wparam, lparam);
        }

        HINSTANCE instance_{};
        HWND window_{};
        HDC device_context_{};
        HGLRC render_context_{};
        epoch_gui_demo_renderer* renderer_{};
        bool native_frame_enabled_{ true };
        bool pending_native_frame_enabled_{ true };
        bool frame_change_message_pending_{};
        bool frame_change_in_progress_{};
        bool rendering_{};
        bool graphics_shutdown_{};
    };
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WindowsApplication application;
    if (!application.initialize(instance))
    {
        MessageBoxW(
            nullptr,
            L"OpenGL 3.2 core initialization failed.",
            L"EpochGui Complete Demo",
            MB_OK | MB_ICONERROR);
        return 1;
    }
    return application.run();
}
