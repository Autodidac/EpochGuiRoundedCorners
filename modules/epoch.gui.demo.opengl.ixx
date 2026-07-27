module;

#include <memory>

export module epoch.gui.demo.opengl;

export namespace epochengine::gui_demo
{
    using OpenGLProc = void (*)(void);
    using OpenGLProcLoader = OpenGLProc (*)(const char* name);

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
        void render() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
