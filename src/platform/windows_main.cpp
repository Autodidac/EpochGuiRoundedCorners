#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <GL/gl.h>

#include "epochgui_demo/app_bridge.hpp"

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

        const auto address = reinterpret_cast<epoch_gui_demo_gl_proc>(
            wglGetProcAddress(name));
        return invalid_wgl_address(address) ? nullptr : address;
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

            constexpr wchar_t class_name[] = L"EpochGuiDemoOpenGL";
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
            constexpr DWORD style = WS_OVERLAPPEDWINDOW;
            if (!AdjustWindowRectEx(&frame, style, FALSE, 0))
                return false;

            const int width = frame.right - frame.left;
            const int height = frame.bottom - frame.top;
            const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
            const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

            window_ = CreateWindowExW(
                0,
                class_name,
                L"EpochGui Demo - Core and Optional Features",
                style,
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

            device_context_ = GetDC(window_);
            if (!device_context_ || !create_opengl_context())
                return false;

            renderer_ = epoch_gui_demo_create();
            if (!renderer_ || !epoch_gui_demo_initialize(renderer_, &load_opengl_proc))
                return false;

            RECT client{};
            GetClientRect(window_, &client);
            epoch_gui_demo_resize(
                renderer_,
                client.right - client.left,
                client.bottom - client.top);

            ShowWindow(window_, SW_SHOWNORMAL);
            UpdateWindow(window_);
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

        void render()
        {
            if (!renderer_ || !device_context_)
                return;

            epoch_gui_demo_render(renderer_);
            SwapBuffers(device_context_);
        }

        [[nodiscard]] LRESULT handle_message(
            HWND window,
            UINT message,
            WPARAM wparam,
            LPARAM lparam)
        {
            switch (message)
            {
            case WM_PAINT:
            {
                PAINTSTRUCT paint{};
                BeginPaint(window, &paint);
                render();
                EndPaint(window, &paint);
                return 0;
            }
            case WM_SIZE:
                if (renderer_)
                {
                    epoch_gui_demo_resize(
                        renderer_,
                        static_cast<int>(LOWORD(lparam)),
                        static_cast<int>(HIWORD(lparam)));
                    InvalidateRect(window, nullptr, FALSE);
                }
                return 0;
            case WM_ERASEBKGND:
                return 1;
            case WM_KEYDOWN:
                if (wparam == VK_ESCAPE)
                {
                    SendMessageW(window, WM_CLOSE, 0, 0);
                    return 0;
                }
                break;
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
            L"EpochGui Demo",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    return application.run();
}
