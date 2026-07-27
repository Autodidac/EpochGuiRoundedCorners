module;

#include <memory>
#include <string_view>

export module epoch.gui.demo.opengl;

export import epoch.gui.rounded_rect;
export import epoch.gui.input;

export namespace epochengine::gui_demo
{
    namespace rounded_rect = epochengine::gui_lib::rounded_rect;
    namespace input = epochengine::gui_lib::input;

    using OpenGLProc = void (*)(void);
    using OpenGLProcLoader = OpenGLProc (*)(const char* name);
    using PointerButton = input::PointerButton;
    using Key = input::Key;
    using WindowChromeRegion = input::WindowChromeRegion;
    using WindowCommand = input::WindowCommand;

    class OpenGLRenderer final
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer();

        OpenGLRenderer(OpenGLRenderer&&) noexcept;
        OpenGLRenderer& operator=(OpenGLRenderer&&) noexcept;

        OpenGLRenderer(const OpenGLRenderer&) = delete;
        OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

        [[nodiscard]] bool initialize(OpenGLProcLoader loader) noexcept;
        void resize(int width, int height) noexcept;
        void pointer_move(float x, float y) noexcept;
        void pointer_button(PointerButton button, bool down) noexcept;
        void wheel(float horizontal, float vertical) noexcept;
        void key(Key key_code, bool down, bool repeated = false) noexcept;
        void modifiers(input::ModifierState state) noexcept;
        void text_input(std::string_view text);
        void render() noexcept;

        [[nodiscard]] WindowChromeRegion window_chrome_hit_test(float x, float y) const noexcept;
        [[nodiscard]] WindowCommand take_window_command() noexcept;
        [[nodiscard]] int take_native_frame_mode() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
