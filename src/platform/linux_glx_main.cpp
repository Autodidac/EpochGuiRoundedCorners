#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "epochgui_demo/app_bridge.hpp"

#include <chrono>
#include <thread>

namespace
{
    constexpr int GLX_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
    constexpr int GLX_CONTEXT_MINOR_VERSION_ARB = 0x2092;
    constexpr int GLX_CONTEXT_FLAGS_ARB = 0x2094;
    constexpr int GLX_CONTEXT_PROFILE_MASK_ARB = 0x9126;
    constexpr int GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x0002;
    constexpr int GLX_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;

    using GlxCreateContextAttribs = GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    using GlxSwapIntervalExt = void (*)(Display*, GLXDrawable, int);

    [[nodiscard]] epoch_gui_demo_gl_proc load_opengl_proc(const char* name)
    {
        return reinterpret_cast<epoch_gui_demo_gl_proc>(
            glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name)));
    }

    class LinuxApplication final
    {
    public:
        ~LinuxApplication()
        {
            shutdown();
        }

        [[nodiscard]] bool initialize()
        {
            display_ = XOpenDisplay(nullptr);
            if (!display_)
                return false;

            const int screen = DefaultScreen(display_);
            constexpr int framebuffer_attributes[] = {
                GLX_X_RENDERABLE, True,
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                GLX_RED_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_ALPHA_SIZE, 8,
                GLX_DEPTH_SIZE, 24,
                GLX_STENCIL_SIZE, 8,
                GLX_DOUBLEBUFFER, True,
                None
            };

            int config_count = 0;
            GLXFBConfig* configs = glXChooseFBConfig(
                display_,
                screen,
                framebuffer_attributes,
                &config_count);
            if (!configs || config_count <= 0)
            {
                if (configs)
                    XFree(configs);
                return false;
            }

            framebuffer_config_ = configs[0];
            XVisualInfo* visual = glXGetVisualFromFBConfig(display_, framebuffer_config_);
            XFree(configs);
            if (!visual)
                return false;

            XSetWindowAttributes attributes{};
            attributes.colormap = XCreateColormap(
                display_,
                RootWindow(display_, visual->screen),
                visual->visual,
                AllocNone);
            attributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
            colormap_ = attributes.colormap;

            window_ = XCreateWindow(
                display_,
                RootWindow(display_, visual->screen),
                0,
                0,
                960,
                620,
                0,
                visual->depth,
                InputOutput,
                visual->visual,
                CWColormap | CWEventMask,
                &attributes);
            XFree(visual);
            if (!window_)
                return false;

            XStoreName(display_, window_, "EpochGUI Rounded Corners - OpenGL");
            delete_message_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
            XSetWMProtocols(display_, window_, &delete_message_, 1);

            const auto create_context = reinterpret_cast<GlxCreateContextAttribs>(
                glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
            if (!create_context)
                return false;

            constexpr int context_attributes[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                None
            };

            context_ = create_context(
                display_,
                framebuffer_config_,
                nullptr,
                True,
                context_attributes);
            if (!context_ || !glXMakeCurrent(display_, window_, context_))
                return false;

            const auto swap_interval = reinterpret_cast<GlxSwapIntervalExt>(
                glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT")));
            if (swap_interval)
                swap_interval(display_, window_, 1);

            renderer_ = epoch_gui_demo_create();
            if (!renderer_ || !epoch_gui_demo_initialize(renderer_, &load_opengl_proc))
                return false;

            epoch_gui_demo_resize(renderer_, 960, 620);
            XMapWindow(display_, window_);
            XFlush(display_);
            return true;
        }

        [[nodiscard]] int run()
        {
            bool running = true;
            bool animation_final_frame_drawn = false;

            const auto handle_event = [this, &running](XEvent& event)
            {
                switch (event.type)
                {
                case Expose:
                    if (event.xexpose.count == 0)
                        render();
                    break;
                case ConfigureNotify:
                    epoch_gui_demo_resize(
                        renderer_,
                        event.xconfigure.width,
                        event.xconfigure.height);
                    render();
                    break;
                case KeyPress:
                {
                    KeySym key{};
                    char buffer[8]{};
                    XLookupString(&event.xkey, buffer, sizeof(buffer), &key, nullptr);
                    if (key == XK_Escape)
                        running = false;
                    break;
                }
                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == delete_message_)
                        running = false;
                    break;
                default:
                    break;
                }
            };

            while (running)
            {
                if (epoch_gui_demo_startup_animation_complete(renderer_))
                {
                    if (!animation_final_frame_drawn)
                    {
                        render();
                        animation_final_frame_drawn = true;
                    }

                    XEvent event{};
                    XNextEvent(display_, &event);
                    handle_event(event);
                    continue;
                }

                while (XPending(display_) > 0)
                {
                    XEvent event{};
                    XNextEvent(display_, &event);
                    handle_event(event);
                }

                render();
                std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
            }

            return 0;
        }

    private:
        void render()
        {
            if (!renderer_ || !display_ || !window_)
                return;

            epoch_gui_demo_render(renderer_);
            glXSwapBuffers(display_, window_);
        }

        void shutdown()
        {
            if (shutdown_)
                return;
            shutdown_ = true;

            if (display_ && context_)
                glXMakeCurrent(display_, window_, context_);

            epoch_gui_demo_destroy(renderer_);
            renderer_ = nullptr;

            if (!display_)
                return;

            glXMakeCurrent(display_, None, nullptr);
            if (context_)
            {
                glXDestroyContext(display_, context_);
                context_ = nullptr;
            }
            if (window_)
            {
                XDestroyWindow(display_, window_);
                window_ = 0;
            }
            if (colormap_)
            {
                XFreeColormap(display_, colormap_);
                colormap_ = 0;
            }
            XCloseDisplay(display_);
            display_ = nullptr;
        }

        Display* display_{};
        Window window_{};
        Colormap colormap_{};
        Atom delete_message_{};
        GLXFBConfig framebuffer_config_{};
        GLXContext context_{};
        epoch_gui_demo_renderer* renderer_{};
        bool shutdown_{};
    };
}

int main()
{
    LinuxApplication application;
    return application.initialize() ? application.run() : 1;
}
