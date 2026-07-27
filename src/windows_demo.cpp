module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

import epoch.gui.rounded_rect;

namespace
{
    using Microsoft::WRL::ComPtr;
    namespace gui = epochengine::gui_lib;
    namespace rounded = epochengine::gui_lib::rounded_rect;

    constexpr float kCanvasWidth = 940.0f;
    constexpr float kCanvasHeight = 590.0f;

    struct DemoVisual
    {
        std::wstring_view title{};
        rounded::RoundedRectOptions options{};
        D2D1_COLOR_F fill{};
        D2D1_COLOR_F border{};
        rounded::RoundedRectMesh mesh{};
        ComPtr<ID2D1PathGeometry> outer_geometry{};
        ComPtr<ID2D1PathGeometry> inner_geometry{};
    };

    [[nodiscard]] HRESULT make_geometry(
        ID2D1Factory* factory,
        std::span<const gui::Vec2> contour,
        ComPtr<ID2D1PathGeometry>& geometry) noexcept
    {
        if (!factory || contour.size() < 3)
            return E_INVALIDARG;

        HRESULT result = factory->CreatePathGeometry(geometry.ReleaseAndGetAddressOf());
        if (FAILED(result))
            return result;

        ComPtr<ID2D1GeometrySink> sink;
        result = geometry->Open(sink.GetAddressOf());
        if (FAILED(result))
            return result;

        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        sink->BeginFigure(
            D2D1::Point2F(contour.front().x, contour.front().y),
            D2D1_FIGURE_BEGIN_FILLED);

        for (const gui::Vec2 point : contour.subspan(1))
            sink->AddLine(D2D1::Point2F(point.x, point.y));

        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        return sink->Close();
    }

    class DemoApplication final
    {
    public:
        [[nodiscard]] HRESULT initialize(HINSTANCE instance) noexcept
        {
            instance_ = instance;

            HRESULT result = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                factory_.ReleaseAndGetAddressOf());
            if (FAILED(result))
                return result;

            result = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(write_factory_.ReleaseAndGetAddressOf()));
            if (FAILED(result))
                return result;

            result = create_text_formats();
            if (FAILED(result))
                return result;

            result = create_visuals();
            if (FAILED(result))
                return result;

            constexpr wchar_t class_name[] = L"EpochGuiRoundedCornersWindow";
            WNDCLASSEXW window_class{
                .cbSize = sizeof(WNDCLASSEXW),
                .style = CS_HREDRAW | CS_VREDRAW,
                .lpfnWndProc = &DemoApplication::window_proc,
                .cbClsExtra = 0,
                .cbWndExtra = 0,
                .hInstance = instance_,
                .hIcon = LoadIconW(nullptr, IDI_APPLICATION),
                .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                .hbrBackground = nullptr,
                .lpszMenuName = nullptr,
                .lpszClassName = class_name,
                .hIconSm = LoadIconW(nullptr, IDI_APPLICATION)
            };

            if (!RegisterClassExW(&window_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return HRESULT_FROM_WIN32(GetLastError());

            RECT window_rect{ 0, 0, static_cast<LONG>(kCanvasWidth), static_cast<LONG>(kCanvasHeight) };
            constexpr DWORD style = WS_OVERLAPPEDWINDOW;
            if (!AdjustWindowRectEx(&window_rect, style, FALSE, 0))
                return HRESULT_FROM_WIN32(GetLastError());

            const int width = window_rect.right - window_rect.left;
            const int height = window_rect.bottom - window_rect.top;
            const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
            const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

            window_ = CreateWindowExW(
                0,
                class_name,
                L"EpochGUI Rounded Corners",
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
                return HRESULT_FROM_WIN32(GetLastError());

            ShowWindow(window_, SW_SHOWNORMAL);
            UpdateWindow(window_);
            return S_OK;
        }

        [[nodiscard]] int run() noexcept
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
        [[nodiscard]] HRESULT create_text_formats() noexcept
        {
            HRESULT result = write_factory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                28.0f,
                L"en-us",
                heading_format_.ReleaseAndGetAddressOf());
            if (FAILED(result))
                return result;

            result = write_factory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                15.0f,
                L"en-us",
                body_format_.ReleaseAndGetAddressOf());
            if (FAILED(result))
                return result;

            result = write_factory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_SEMI_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                19.0f,
                L"en-us",
                label_format_.ReleaseAndGetAddressOf());
            if (FAILED(result))
                return result;

            return write_factory_->CreateTextFormat(
                L"Consolas",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                13.0f,
                L"en-us",
                metrics_format_.ReleaseAndGetAddressOf());
        }

        [[nodiscard]] HRESULT create_visuals() noexcept
        {
            visuals_ = {
                DemoVisual{
                    .title = L"Uniform corners",
                    .options = {
                        .bounds = { { 70.0f, 110.0f }, { 360.0f, 120.0f } },
                        .radii = { 24.0f, 24.0f, 24.0f, 24.0f },
                        .border_width = 3.0f,
                        .segments_per_corner = 12
                    },
                    .fill = D2D1::ColorF(0x182235),
                    .border = D2D1::ColorF(0x65a7ff)
                },
                DemoVisual{
                    .title = L"Pill control",
                    .options = {
                        .bounds = { { 510.0f, 126.0f }, { 360.0f, 88.0f } },
                        .radii = { 44.0f, 44.0f, 44.0f, 44.0f },
                        .border_width = 2.0f,
                        .segments_per_corner = 16
                    },
                    .fill = D2D1::ColorF(0x243248),
                    .border = D2D1::ColorF(0x7dd3fc)
                },
                DemoVisual{
                    .title = L"Asymmetric corners",
                    .options = {
                        .bounds = { { 70.0f, 340.0f }, { 360.0f, 150.0f } },
                        .radii = { 8.0f, 44.0f, 12.0f, 36.0f },
                        .border_width = 4.0f,
                        .segments_per_corner = 12
                    },
                    .fill = D2D1::ColorF(0x231c32),
                    .border = D2D1::ColorF(0xc084fc)
                },
                DemoVisual{
                    .title = L"Oversized radii clamp safely",
                    .options = {
                        .bounds = { { 510.0f, 350.0f }, { 360.0f, 130.0f } },
                        .radii = { 160.0f, 120.0f, 180.0f, 140.0f },
                        .border_width = 5.0f,
                        .segments_per_corner = 16
                    },
                    .fill = D2D1::ColorF(0x1b302b),
                    .border = D2D1::ColorF(0x6ee7b7)
                }
            };

            for (DemoVisual& visual : visuals_)
            {
                visual.mesh = rounded::make_rounded_rect_mesh(visual.options);
                if (!visual.mesh.valid)
                    return E_FAIL;

                HRESULT result = make_geometry(
                    factory_.Get(),
                    visual.mesh.outer_contour(),
                    visual.outer_geometry);
                if (FAILED(result))
                    return result;

                if (!visual.mesh.inner_contour().empty())
                {
                    result = make_geometry(
                        factory_.Get(),
                        visual.mesh.inner_contour(),
                        visual.inner_geometry);
                    if (FAILED(result))
                        return result;
                }
            }

            return S_OK;
        }

        [[nodiscard]] HRESULT ensure_render_target() noexcept
        {
            if (render_target_)
                return S_OK;

            RECT client{};
            GetClientRect(window_, &client);
            const D2D1_SIZE_U size{
                static_cast<UINT32>((std::max)(1L, client.right - client.left)),
                static_cast<UINT32>((std::max)(1L, client.bottom - client.top))
            };

            HRESULT result = factory_->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(window_, size),
                render_target_.ReleaseAndGetAddressOf());
            if (FAILED(result))
                return result;

            return render_target_->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                brush_.ReleaseAndGetAddressOf());
        }

        void discard_device_resources() noexcept
        {
            brush_.Reset();
            render_target_.Reset();
        }

        void draw_text(
            std::wstring_view text,
            D2D1_RECT_F bounds,
            IDWriteTextFormat* format,
            D2D1_COLOR_F color) noexcept
        {
            brush_->SetColor(color);
            render_target_->DrawTextW(
                text.data(),
                static_cast<UINT32>(text.size()),
                format,
                bounds,
                brush_.Get(),
                D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }

        void render() noexcept
        {
            if (FAILED(ensure_render_target()))
                return;

            const D2D1_SIZE_F target_size = render_target_->GetSize();
            const float scale = (std::min)(
                target_size.width / kCanvasWidth,
                target_size.height / kCanvasHeight);
            const float offset_x = (target_size.width - kCanvasWidth * scale) * 0.5f;
            const float offset_y = (target_size.height - kCanvasHeight * scale) * 0.5f;

            render_target_->BeginDraw();
            render_target_->SetTransform(D2D1::Matrix3x2F(
                scale, 0.0f,
                0.0f, scale,
                offset_x, offset_y));
            render_target_->Clear(D2D1::ColorF(0x0b1020));

            draw_text(
                L"EpochGUI Rounded Corners",
                D2D1::RectF(54.0f, 24.0f, 886.0f, 64.0f),
                heading_format_.Get(),
                D2D1::ColorF(0xf8fafc));
            draw_text(
                L"Backend-neutral tessellated geometry generated from EpochGUI Rect and Vec2.",
                D2D1::RectF(54.0f, 66.0f, 886.0f, 92.0f),
                body_format_.Get(),
                D2D1::ColorF(0x94a3b8));

            for (const DemoVisual& visual : visuals_)
            {
                brush_->SetColor(visual.border_width > 0.0f ? visual.border : visual.fill);
                render_target_->FillGeometry(visual.outer_geometry.Get(), brush_.Get());

                if (visual.inner_geometry)
                {
                    brush_->SetColor(visual.fill);
                    render_target_->FillGeometry(visual.inner_geometry.Get(), brush_.Get());
                }

                const gui::Rect bounds = visual.mesh.bounds;
                draw_text(
                    visual.title,
                    D2D1::RectF(
                        bounds.position.x + 22.0f,
                        bounds.position.y + 20.0f,
                        bounds.position.x + bounds.size.x - 18.0f,
                        bounds.position.y + 52.0f),
                    label_format_.Get(),
                    D2D1::ColorF(0xf8fafc));

                const std::wstring metrics = std::to_wstring(visual.mesh.outer_count)
                    + L" contour vertices | "
                    + std::to_wstring(visual.mesh.fill_indices.size() / 3U)
                    + L" fill triangles";
                draw_text(
                    metrics,
                    D2D1::RectF(
                        bounds.position.x + 22.0f,
                        bounds.position.y + 56.0f,
                        bounds.position.x + bounds.size.x - 18.0f,
                        bounds.position.y + 82.0f),
                    metrics_format_.Get(),
                    D2D1::ColorF(0xaab7cb));
            }

            draw_text(
                L"Escape closes the demo. Resize the window to verify scale-independent geometry.",
                D2D1::RectF(54.0f, 530.0f, 886.0f, 560.0f),
                body_format_.Get(),
                D2D1::ColorF(0x64748b));

            const HRESULT result = render_target_->EndDraw();
            if (result == D2DERR_RECREATE_TARGET)
                discard_device_resources();
        }

        void resize(UINT width, UINT height) noexcept
        {
            if (render_target_ && width > 0 && height > 0)
                render_target_->Resize(D2D1::SizeU(width, height));
            InvalidateRect(window_, nullptr, FALSE);
        }

        [[nodiscard]] LRESULT handle_message(
            HWND window,
            UINT message,
            WPARAM wparam,
            LPARAM lparam) noexcept
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
                resize(LOWORD(lparam), HIWORD(lparam));
                return 0;
            case WM_DISPLAYCHANGE:
                InvalidateRect(window, nullptr, FALSE);
                return 0;
            case WM_ERASEBKGND:
                return 1;
            case WM_KEYDOWN:
                if (wparam == VK_ESCAPE)
                {
                    DestroyWindow(window);
                    return 0;
                }
                break;
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
            LPARAM lparam) noexcept
        {
            DemoApplication* application = reinterpret_cast<DemoApplication*>(
                GetWindowLongPtrW(window, GWLP_USERDATA));

            if (message == WM_NCCREATE)
            {
                const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
                application = static_cast<DemoApplication*>(create->lpCreateParams);
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
        ComPtr<ID2D1Factory> factory_{};
        ComPtr<IDWriteFactory> write_factory_{};
        ComPtr<ID2D1HwndRenderTarget> render_target_{};
        ComPtr<ID2D1SolidColorBrush> brush_{};
        ComPtr<IDWriteTextFormat> heading_format_{};
        ComPtr<IDWriteTextFormat> body_format_{};
        ComPtr<IDWriteTextFormat> label_format_{};
        ComPtr<IDWriteTextFormat> metrics_format_{};
        std::array<DemoVisual, 4> visuals_{};
    };
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    DemoApplication application;
    const HRESULT result = application.initialize(instance);
    if (FAILED(result))
    {
        MessageBoxW(
            nullptr,
            L"The EpochGUI rounded-corner demo could not initialize Direct2D.",
            L"EpochGUI Rounded Corners",
            MB_OK | MB_ICONERROR);
        return static_cast<int>(result);
    }

    return application.run();
}
