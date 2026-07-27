#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "epochgui_demo/app_bridge.hpp"

namespace
{
    constexpr int GLX_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
    constexpr int GLX_CONTEXT_MINOR_VERSION_ARB = 0x2092;
    constexpr int GLX_CONTEXT_FLAGS_ARB = 0x2094;
    constexpr int GLX_CONTEXT_PROFILE_MASK_ARB = 0x9126;
    constexpr int GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x0002;
    constexpr int GLX_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;

    constexpr long MWM_HINTS_DECORATIONS = 1L << 1;

    struct MotifWmHints
    {
        unsigned long flags{};
        unsigned long functions{};
        unsigned long decorations{};
        long input_mode{};
        unsigned long status{};
    };

    using GlxCreateContextAttribs = GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    using GlxSwapIntervalExt = void (*)(Display*, GLXDrawable, int);

    [[nodiscard]] epoch_gui_demo_gl_proc load_opengl_proc(const char* name)
    {
        return reinterpret_cast<epoch_gui_demo_gl_proc>(
            glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name)));
    }

    [[nodiscard]] int pointer_button(unsigned int button) noexcept
    {
        switch (button)
        {
        case Button3: return EPOCH_GUI_DEMO_POINTER_RIGHT;
        case Button2: return EPOCH_GUI_DEMO_POINTER_MIDDLE;
        default: return EPOCH_GUI_DEMO_POINTER_LEFT;
        }
    }

    [[nodiscard]] int key_from_keysym(KeySym key) noexcept
    {
        switch (key)
        {
        case XK_Escape: return EPOCH_GUI_DEMO_KEY_ESCAPE;
        case XK_Return: return EPOCH_GUI_DEMO_KEY_ENTER;
        case XK_Tab: return EPOCH_GUI_DEMO_KEY_TAB;
        case XK_space: return EPOCH_GUI_DEMO_KEY_SPACE;
        case XK_BackSpace: return EPOCH_GUI_DEMO_KEY_BACKSPACE;
        case XK_Delete: return EPOCH_GUI_DEMO_KEY_DELETE;
        case XK_Left: return EPOCH_GUI_DEMO_KEY_LEFT;
        case XK_Right: return EPOCH_GUI_DEMO_KEY_RIGHT;
        case XK_Up: return EPOCH_GUI_DEMO_KEY_UP;
        case XK_Down: return EPOCH_GUI_DEMO_KEY_DOWN;
        case XK_Home: return EPOCH_GUI_DEMO_KEY_HOME;
        case XK_End: return EPOCH_GUI_DEMO_KEY_END;
        case XK_Shift_L:
        case XK_Shift_R:
            return EPOCH_GUI_DEMO_KEY_SHIFT;
        case XK_Control_L:
        case XK_Control_R:
            return EPOCH_GUI_DEMO_KEY_CONTROL;
        case XK_Alt_L:
        case XK_Alt_R:
            return EPOCH_GUI_DEMO_KEY_ALT;
        case XK_Super_L:
        case XK_Super_R:
            return EPOCH_GUI_DEMO_KEY_SUPER;
        default:
            return -1;
        }
    }

    [[nodiscard]] int moveresize_direction(int region) noexcept
    {
        switch (region)
        {
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP_LEFT: return 0;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP: return 1;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_TOP_RIGHT: return 2;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_RIGHT: return 3;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM_RIGHT: return 4;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM: return 5;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_BOTTOM_LEFT: return 6;
        case EPOCH_GUI_DEMO_WINDOW_RESIZE_LEFT: return 7;
        case EPOCH_GUI_DEMO_WINDOW_CAPTION: return 8;
        default:
            return -1;
        }
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
            attributes.event_mask = ExposureMask
                | StructureNotifyMask
                | KeyPressMask
                | KeyReleaseMask
                | ButtonPressMask
                | ButtonReleaseMask
                | PointerMotionMask;
            colormap_ = attributes.colormap;

            window_ = XCreateWindow(
                display_,
                RootWindow(display_, visual->screen),
                0,
                0,
                1280,
                820,
                0,
                visual->depth,
                InputOutput,
                visual->visual,
                CWColormap | CWEventMask,
                &attributes);
            XFree(visual);
            if (!window_)
                return false;

            XStoreName(display_, window_, "EpochGui Live Demo");
            delete_message_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
            XSetWMProtocols(display_, window_, &delete_message_, 1);

            const Atom motif_hints = XInternAtom(display_, "_MOTIF_WM_HINTS", False);
            const MotifWmHints hints{
                .flags = static_cast<unsigned long>(MWM_HINTS_DECORATIONS),
                .decorations = 0
            };
            XChangeProperty(
                display_,
                window_,
                motif_hints,
                motif_hints,
                32,
                PropModeReplace,
                reinterpret_cast<const unsigned char*>(&hints),
                5);

            moveresize_atom_ = XInternAtom(display_, "_NET_WM_MOVERESIZE", False);
            wm_state_atom_ = XInternAtom(display_, "_NET_WM_STATE", False);
            maximized_horz_atom_ = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
            maximized_vert_atom_ = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);

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

            epoch_gui_demo_resize(renderer_, 1280, 820);
            XMapWindow(display_, window_);
            XFlush(display_);
            return true;
        }

        [[nodiscard]] int run()
        {
            running_ = true;
            while (running_)
            {
                XEvent event{};
                XNextEvent(display_, &event);

                switch (event.type)
                {
                case Expose:
                    if (event.xexpose.count == 0)
                        render_and_apply_commands();
                    break;

                case ConfigureNotify:
                    epoch_gui_demo_resize(
                        renderer_,
                        event.xconfigure.width,
                        event.xconfigure.height);
                    render_and_apply_commands();
                    break;

                case MotionNotify:
                    epoch_gui_demo_pointer_move(
                        renderer_,
                        static_cast<float>(event.xmotion.x),
                        static_cast<float>(event.xmotion.y));
                    render_and_apply_commands();
                    break;

                case ButtonPress:
                    handle_button(event.xbutton, true);
                    break;

                case ButtonRelease:
                    handle_button(event.xbutton, false);
                    break;

                case KeyPress:
                case KeyRelease:
                {
                    const KeySym symbol = XLookupKeysym(&event.xkey, 0);
                    const int key = key_from_keysym(symbol);
                    if (key >= 0)
                    {
                        epoch_gui_demo_key_event(
                            renderer_,
                            key,
                            event.type == KeyPress,
                            false);
                        epoch_gui_demo_modifiers(
                            renderer_,
                            (event.xkey.state & ShiftMask) != 0,
                            (event.xkey.state & ControlMask) != 0,
                            (event.xkey.state & Mod1Mask) != 0,
                            (event.xkey.state & Mod4Mask) != 0);
                        render_and_apply_commands();
                    }
                    break;
                }

                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == delete_message_)
                        running_ = false;
                    break;

                default:
                    break;
                }
            }
            return 0;
        }

    private:
        void begin_native_move_resize(const XButtonEvent& event, int direction)
        {
            if (direction < 0)
                return;

            XUngrabPointer(display_, CurrentTime);
            XEvent message{};
            message.xclient.type = ClientMessage;
            message.xclient.message_type = moveresize_atom_;
            message.xclient.display = display_;
            message.xclient.window = window_;
            message.xclient.format = 32;
            message.xclient.data.l[0] = event.x_root;
            message.xclient.data.l[1] = event.y_root;
            message.xclient.data.l[2] = direction;
            message.xclient.data.l[3] = Button1;
            message.xclient.data.l[4] = 1;
            XSendEvent(
                display_,
                DefaultRootWindow(display_),
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                &message);
        }

        void toggle_maximize()
        {
            XEvent message{};
            message.xclient.type = ClientMessage;
            message.xclient.message_type = wm_state_atom_;
            message.xclient.display = display_;
            message.xclient.window = window_;
            message.xclient.format = 32;
            message.xclient.data.l[0] = 2;
            message.xclient.data.l[1] = static_cast<long>(maximized_horz_atom_);
            message.xclient.data.l[2] = static_cast<long>(maximized_vert_atom_);
            message.xclient.data.l[3] = 1;
            XSendEvent(
                display_,
                DefaultRootWindow(display_),
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                &message);
        }

        void handle_button(const XButtonEvent& event, bool down)
        {
            epoch_gui_demo_pointer_move(
                renderer_,
                static_cast<float>(event.x),
                static_cast<float>(event.y));

            if (event.button == Button4 || event.button == Button5)
            {
                if (down)
                {
                    epoch_gui_demo_wheel(
                        renderer_,
                        0.0f,
                        event.button == Button4 ? 1.0f : -1.0f);
                    render_and_apply_commands();
                }
                return;
            }

            epoch_gui_demo_pointer_button(renderer_, pointer_button(event.button), down);
            if (down && event.button == Button1)
            {
                const int region = epoch_gui_demo_window_hit_test(
                    renderer_,
                    static_cast<float>(event.x),
                    static_cast<float>(event.y));
                begin_native_move_resize(event, moveresize_direction(region));
            }
            render_and_apply_commands();
        }

        void render_and_apply_commands()
        {
            if (!renderer_ || !display_ || !window_)
                return;

            epoch_gui_demo_render(renderer_);
            glXSwapBuffers(display_, window_);

            switch (epoch_gui_demo_take_window_command(renderer_))
            {
            case EPOCH_GUI_DEMO_COMMAND_MINIMIZE:
                XIconifyWindow(display_, window_, DefaultScreen(display_));
                break;
            case EPOCH_GUI_DEMO_COMMAND_TOGGLE_MAXIMIZE:
                toggle_maximize();
                break;
            case EPOCH_GUI_DEMO_COMMAND_CLOSE:
                running_ = false;
                break;
            default:
                break;
            }
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
        Atom moveresize_atom_{};
        Atom wm_state_atom_{};
        Atom maximized_horz_atom_{};
        Atom maximized_vert_atom_{};
        GLXFBConfig framebuffer_config_{};
        GLXContext context_{};
        epoch_gui_demo_renderer* renderer_{};
        bool running_{};
        bool shutdown_{};
    };
}

int main()
{
    LinuxApplication application;
    return application.initialize() ? application.run() : 1;
}
