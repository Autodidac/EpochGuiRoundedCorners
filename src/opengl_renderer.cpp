module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

module epoch.gui.demo.opengl;

import epoch.gui;
import epoch.gui.rounded_rect;
import epoch.gui.input;

namespace epochengine::gui_demo
{
    namespace
    {
        namespace gui = epochengine::gui_lib;
        namespace rounded = epochengine::gui_lib::rounded_rect;
        namespace input = epochengine::gui_lib::input;

        using GLenum = unsigned int;
        using GLboolean = unsigned char;
        using GLbitfield = unsigned int;
        using GLint = int;
        using GLsizei = int;
        using GLuint = unsigned int;
        using GLfloat = float;
        using GLchar = char;
        using GLsizeiptr = std::ptrdiff_t;

        constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
        constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;
        constexpr GLenum GL_FLOAT = 0x1406;
        constexpr GLenum GL_TRIANGLES = 0x0004;
        constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
        constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
        constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
        constexpr GLenum GL_LINK_STATUS = 0x8B82;
        constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x00004000;
        constexpr GLboolean GL_FALSE = 0;

        constexpr float canvas_width = 1280.0f;
        constexpr float canvas_height = 820.0f;
        constexpr float native_title_height = 54.0f;
        constexpr float toolbar_height = 58.0f;
        constexpr float workspace_top = native_title_height + toolbar_height;

        struct Color
        {
            float r{};
            float g{};
            float b{};
            float a{ 1.0f };
        };

        [[nodiscard]] constexpr Color rgb(std::uint32_t value, float alpha = 1.0f) noexcept
        {
            return {
                static_cast<float>((value >> 16U) & 0xffU) / 255.0f,
                static_cast<float>((value >> 8U) & 0xffU) / 255.0f,
                static_cast<float>(value & 0xffU) / 255.0f,
                alpha
            };
        }

        constexpr Color background = rgb(0x0a0f1c);
        constexpr Color title_fill = rgb(0x111a2b);
        constexpr Color toolbar_fill = rgb(0x0e1728);
        constexpr Color sidebar_fill = rgb(0x10192a);
        constexpr Color workspace_fill = rgb(0c0f1827);
        constexpr Color panel_fill = rgb(0x141f33);
        constexpr Color panel_border = rgb(0x2a3b57);
        constexpr Color control_fill = rgb(0x1b2941);
        constexpr Color control_hover = rgb(0x263b5c);
        constexpr Color control_selected = rgb(0x315f91);
        constexpr Color accent = rgb(0x65a7ff);
        constexpr Color accent_soft = rgb(0x244d79);
        constexpr Color green = rgb(0x6ee7b7);
        constexpr Color warning = rgb(0xf2bd62);
        constexpr Color danger = rgb(0xef6b73);
        constexpr Color text_primary = rgb(0xeaf0fa);
        constexpr Color text_secondary = rgb(0xa9b9cf);
        constexpr Color text_muted = rgb(0x71839d);

        template <typename Function>
        [[nodiscard]] bool load_function(
            Function& function,
            OpenGLProcLoader loader,
            const char* name) noexcept
        {
            function = reinterpret_cast<Function>(loader(name));
            return function != nullptr;
        }

        struct OpenGLFunctions
        {
            using GenVertexArrays = void (*)(GLsizei, GLuint*);
            using BindVertexArray = void (*)(GLuint);
            using DeleteVertexArrays = void (*)(GLsizei, const GLuint*);
            using GenBuffers = void (*)(GLsizei, GLuint*);
            using BindBuffer = void (*)(GLenum, GLuint);
            using BufferData = void (*)(GLenum, GLsizeiptr, const void*, GLenum);
            using DeleteBuffers = void (*)(GLsizei, const GLuint*);
            using EnableVertexAttribArray = void (*)(GLuint);
            using VertexAttribPointer = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
            using CreateShader = GLuint (*)(GLenum);
            using ShaderSource = void (*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
            using CompileShader = void (*)(GLuint);
            using GetShaderiv = void (*)(GLuint, GLenum, GLint*);
            using DeleteShader = void (*)(GLuint);
            using CreateProgram = GLuint (*)(void);
            using AttachShader = void (*)(GLuint, GLuint);
            using BindAttribLocation = void (*)(GLuint, GLuint, const GLchar*);
            using LinkProgram = void (*)(GLuint);
            using GetProgramiv = void (*)(GLuint, GLenum, GLint*);
            using DeleteProgram = void (*)(GLuint);
            using UseProgram = void (*)(GLuint);
            using GetUniformLocation = GLint (*)(GLuint, const GLchar*);
            using Uniform2f = void (*)(GLint, GLfloat, GLfloat);
            using Uniform4f = void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
            using DrawArrays = void (*)(GLenum, GLint, GLsizei);
            using ClearColor = void (*)(GLfloat, GLfloat, GLfloat, GLfloat);
            using Clear = void (*)(GLbitfield);
            using Viewport = void (*)(GLint, GLint, GLsizei, GLsizei);

            GenVertexArrays gen_vertex_arrays{};
            BindVertexArray bind_vertex_array{};
            DeleteVertexArrays delete_vertex_arrays{};
            GenBuffers gen_buffers{};
            BindBuffer bind_buffer{};
            BufferData buffer_data{};
            DeleteBuffers delete_buffers{};
            EnableVertexAttribArray enable_vertex_attrib_array{};
            VertexAttribPointer vertex_attrib_pointer{};
            CreateShader create_shader{};
            ShaderSource shader_source{};
            CompileShader compile_shader{};
            GetShaderiv get_shader_iv{};
            DeleteShader delete_shader{};
            CreateProgram create_program{};
            AttachShader attach_shader{};
            BindAttribLocation bind_attrib_location{};
            LinkProgram link_program{};
            GetProgramiv get_program_iv{};
            DeleteProgram delete_program{};
            UseProgram use_program{};
            GetUniformLocation get_uniform_location{};
            Uniform2f uniform_2f{};
            Uniform4f uniform_4f{};
            DrawArrays draw_arrays{};
            ClearColor clear_color{};
            Clear clear{};
            Viewport viewport{};

            [[nodiscard]] bool load(OpenGLProcLoader loader) noexcept
            {
                if (!loader)
                    return false;

                return load_function(gen_vertex_arrays, loader, "glGenVertexArrays")
                    && load_function(bind_vertex_array, loader, "glBindVertexArray")
                    && load_function(delete_vertex_arrays, loader, "glDeleteVertexArrays")
                    && load_function(gen_buffers, loader, "glGenBuffers")
                    && load_function(bind_buffer, loader, "glBindBuffer")
                    && load_function(buffer_data, loader, "glBufferData")
                    && load_function(delete_buffers, loader, "glDeleteBuffers")
                    && load_function(enable_vertex_attrib_array, loader, "glEnableVertexAttribArray")
                    && load_function(vertex_attrib_pointer, loader, "glVertexAttribPointer")
                    && load_function(create_shader, loader, "glCreateShader")
                    && load_function(shader_source, loader, "glShaderSource")
                    && load_function(compile_shader, loader, "glCompileShader")
                    && load_function(get_shader_iv, loader, "glGetShaderiv")
                    && load_function(delete_shader, loader, "glDeleteShader")
                    && load_function(create_program, loader, "glCreateProgram")
                    && load_function(attach_shader, loader, "glAttachShader")
                    && load_function(bind_attrib_location, loader, "glBindAttribLocation")
                    && load_function(link_program, loader, "glLinkProgram")
                    && load_function(get_program_iv, loader, "glGetProgramiv")
                    && load_function(delete_program, loader, "glDeleteProgram")
                    && load_function(use_program, loader, "glUseProgram")
                    && load_function(get_uniform_location, loader, "glGetUniformLocation")
                    && load_function(uniform_2f, loader, "glUniform2f")
                    && load_function(uniform_4f, loader, "glUniform4f")
                    && load_function(draw_arrays, loader, "glDrawArrays")
                    && load_function(clear_color, loader, "glClearColor")
                    && load_function(clear, loader, "glClear")
                    && load_function(viewport, loader, "glViewport");
            }
        };

        struct DrawCommand
        {
            GLint first{};
            GLsizei count{};
            Color color{};
        };

        struct SceneBatch
        {
            std::vector<gui::Vec2> vertices{};
            std::vector<DrawCommand> commands{};

            void clear()
            {
                vertices.clear();
                commands.clear();
            }

            void append_triangle(gui::Vec2 a, gui::Vec2 b, gui::Vec2 c)
            {
                vertices.push_back(a);
                vertices.push_back(b);
                vertices.push_back(c);
            }

            void append_rect(gui::Rect rect, Color color)
            {
                if (rect.size.x <= 0.0f || rect.size.y <= 0.0f)
                    return;

                const GLint first = static_cast<GLint>(vertices.size());
                const float left = rect.position.x;
                const float top = rect.position.y;
                const float right = left + rect.size.x;
                const float bottom = top + rect.size.y;

                append_triangle({ left, top }, { right, top }, { right, bottom });
                append_triangle({ left, top }, { right, bottom }, { left, bottom });
                commands.push_back({ first, 6, color });
            }

            void append_indexed(
                const rounded::RoundedRectMesh& mesh,
                std::span<const std::uint32_t> indices,
                Color color)
            {
                if (indices.empty())
                    return;

                const GLint first = static_cast<GLint>(vertices.size());
                for (const std::uint32_t index : indices)
                {
                    if (index < mesh.vertices.size())
                        vertices.push_back(mesh.vertices[index]);
                }

                const GLsizei count = static_cast<GLsizei>(vertices.size()) - first;
                if (count > 0)
                    commands.push_back({ first, count, color });
            }
        };

        [[nodiscard]] gui::Rect inset(gui::Rect rect, float amount) noexcept
        {
            rect.position.x += amount;
            rect.position.y += amount;
            rect.size.x = (std::max)(0.0f, rect.size.x - amount * 2.0f);
            rect.size.y = (std::max)(0.0f, rect.size.y - amount * 2.0f);
            return rect;
        }

        void draw_surface(
            SceneBatch& batch,
            gui::Rect rect,
            bool use_rounded,
            float radius,
            Color fill,
            Color border = panel_border,
            float border_width = 1.0f)
        {
            if (!use_rounded)
            {
                if (border_width > 0.0f)
                {
                    batch.append_rect(rect, border);
                    batch.append_rect(inset(rect, border_width), fill);
                }
                else
                {
                    batch.append_rect(rect, fill);
                }
                return;
            }

            const rounded::RoundedRectMesh mesh = rounded::make_rounded_rect_mesh({
                .bounds = rect,
                .radii = { radius, radius, radius, radius },
                .border_width = border_width,
                .segments_per_corner = 10
            });
            if (!mesh.valid)
                return;

            batch.append_indexed(mesh, mesh.fill_indices, fill);
            batch.append_indexed(mesh, mesh.border_indices, border);
        }

        [[nodiscard]] constexpr std::array<std::uint8_t, 7> glyph(char c) noexcept
        {
            switch (c)
            {
            case 'A': return { 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001 };
            case 'B': return { 0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110 };
            case 'C': return { 0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111 };
            case 'D': return { 0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110 };
            case 'E': return { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111 };
            case 'F': return { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000 };
            case 'G': return { 0b01111, 0b10000, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111 };
            case 'H': return { 0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001 };
            case 'I': return { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111 };
            case 'J': return { 0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100 };
            case 'K': return { 0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001 };
            case 'L': return { 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 };
            case 'M': return { 0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001 };
            case 'N': return { 0b10001, 0b11001, 0b11001, 0b10101, 0b10011, 0b10011, 0b10001 };
            case 'O': return { 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 };
            case 'P': return { 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000 };
            case 'Q': return { 0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101 };
            case 'R': return { 0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001 };
            case 'S': return { 0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110 };
            case 'T': return { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100 };
            case 'U': return { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 };
            case 'V': return { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100 };
            case 'W': return { 0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010 };
            case 'X': return { 0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001 };
            case 'Y': return { 0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100 };
            case 'Z': return { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111 };
            case '0': return { 0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110 };
            case '1': return { 0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110 };
            case '2': return { 0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111 };
            case '3': return { 0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110 };
            case '4': return { 0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010 };
            case '5': return { 0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110 };
            case '6': return { 0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110 };
            case '7': return { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000 };
            case '8': return { 0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110 };
            case '9': return { 0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110 };
            case '-': return { 0, 0, 0, 0b11111, 0, 0, 0 };
            case '.': return { 0, 0, 0, 0, 0, 0b00110, 0b00110 };
            case ':': return { 0, 0b00110, 0b00110, 0, 0b00110, 0b00110, 0 };
            case '/': return { 0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000 };
            default: return { 0, 0, 0, 0, 0, 0, 0 };
            }
        }

        void draw_text(
            SceneBatch& batch,
            float x,
            float y,
            std::string_view text,
            float scale,
            Color color)
        {
            float cursor = x;
            for (const char character : text)
            {
                if (character != ' ')
                {
                    const auto rows = glyph(character);
                    for (std::size_t row = 0; row < rows.size(); ++row)
                    {
                        for (int column = 0; column < 5; ++column)
                        {
                            const std::uint8_t mask = static_cast<std::uint8_t>(1U << (4 - column));
                            if ((rows[row] & mask) != 0)
                            {
                                batch.append_rect({
                                    { cursor + static_cast<float>(column) * scale, y + static_cast<float>(row) * scale },
                                    { scale, scale }
                                }, color);
                            }
                        }
                    }
                }
                cursor += scale * 6.0f;
            }
        }

        [[nodiscard]] GLuint compile_shader(
            OpenGLFunctions& gl,
            GLenum type,
            const char* source) noexcept
        {
            const GLuint shader = gl.create_shader(type);
            if (shader == 0)
                return 0;

            gl.shader_source(shader, 1, &source, nullptr);
            gl.compile_shader(shader);

            GLint compiled = 0;
            gl.get_shader_iv(shader, GL_COMPILE_STATUS, &compiled);
            if (compiled == 0)
            {
                gl.delete_shader(shader);
                return 0;
            }
            return shader;
        }

        [[nodiscard]] GLuint create_program(OpenGLFunctions& gl) noexcept
        {
            constexpr const char* vertex_source = R"GLSL(#version 150 core
in vec2 inPosition;
uniform vec2 uCanvas;
void main()
{
    vec2 normalized = vec2(
        (inPosition.x / uCanvas.x) * 2.0 - 1.0,
        1.0 - (inPosition.y / uCanvas.y) * 2.0);
    gl_Position = vec4(normalized, 0.0, 1.0);
}
)GLSL";

            constexpr const char* fragment_source = R"GLSL(#version 150 core
out vec4 outColor;
uniform vec4 uColor;
void main()
{
    outColor = uColor;
}
)GLSL";

            const GLuint vertex_shader = compile_shader(gl, GL_VERTEX_SHADER, vertex_source);
            const GLuint fragment_shader = compile_shader(gl, GL_FRAGMENT_SHADER, fragment_source);
            if (vertex_shader == 0 || fragment_shader == 0)
            {
                if (vertex_shader != 0)
                    gl.delete_shader(vertex_shader);
                if (fragment_shader != 0)
                    gl.delete_shader(fragment_shader);
                return 0;
            }

            const GLuint program = gl.create_program();
            if (program == 0)
            {
                gl.delete_shader(fragment_shader);
                gl.delete_shader(vertex_shader);
                return 0;
            }

            gl.attach_shader(program, vertex_shader);
            gl.attach_shader(program, fragment_shader);
            gl.bind_attrib_location(program, 0, "inPosition");
            gl.link_program(program);
            gl.delete_shader(fragment_shader);
            gl.delete_shader(vertex_shader);

            GLint linked = 0;
            gl.get_program_iv(program, GL_LINK_STATUS, &linked);
            if (linked == 0)
            {
                gl.delete_program(program);
                return 0;
            }
            return program;
        }
    }

    struct OpenGLRenderer::Impl
    {
        OpenGLFunctions gl{};
        SceneBatch batch{};
        input::InputTracker input_tracker{};
        gui::FloatingWindowState core_window{};
        gui::FloatingWindowState rounded_window{};
        gui::FloatingWindowLayout core_layout{};
        gui::FloatingWindowLayout rounded_layout{};
        gui::PopupState context_popup{};
        gui::PopupLayout context_layout{};
        gui::Vec2 context_anchor{};
        input::BorderlessWindowChromeLayout chrome_layout{};
        input::WindowCommand pending_window_command{ input::WindowCommand::none };
        GLuint program{};
        GLuint vertex_array{};
        GLuint vertex_buffer{};
        GLint canvas_uniform{ -1 };
        GLint color_uniform{ -1 };
        int framebuffer_width{ static_cast<int>(canvas_width) };
        int framebuffer_height{ static_cast<int>(canvas_height) };
        std::uint32_t selected_sidebar_row{ 1 };
        std::uint32_t display_mode{ 2 };
        std::uint32_t next_focus_order{ 3 };
        bool initialized{};

        Impl()
        {
            reset_windows();
            rebuild_chrome();
        }

        ~Impl()
        {
            release();
        }

        void reset_windows() noexcept
        {
            core_window = gui::FloatingWindowState{
                .position = { 292.0f, 196.0f },
                .size = { 390.0f, 258.0f },
                .open = true,
                .initialized = true,
                .focus_order = 1
            };
            rounded_window = gui::FloatingWindowState{
                .position = { 650.0f, 300.0f },
                .size = { 410.0f, 276.0f },
                .open = true,
                .initialized = true,
                .focus_order = 2
            };
            next_focus_order = 3;
        }

        void rebuild_chrome() noexcept
        {
            chrome_layout = input::make_borderless_window_chrome_layout({
                .bounds = { { 0.0f, 0.0f }, { canvas_width, canvas_height } },
                .title_bar_height = native_title_height,
                .resize_border = 7.0f,
                .caption_padding_left = 18.0f,
                .button_width = 48.0f,
                .movable = true,
                .resizable = true,
                .minimizable = true,
                .maximizable = true,
                .closable = true
            });
        }

        [[nodiscard]] gui::Vec2 to_canvas(float x, float y) const noexcept
        {
            const float width = framebuffer_width > 0 ? static_cast<float>(framebuffer_width) : canvas_width;
            const float height = framebuffer_height > 0 ? static_cast<float>(framebuffer_height) : canvas_height;
            return {
                x * canvas_width / width,
                y * canvas_height / height
            };
        }

        void release() noexcept
        {
            if (vertex_buffer != 0)
                gl.delete_buffers(1, &vertex_buffer);
            if (vertex_array != 0)
                gl.delete_vertex_arrays(1, &vertex_array);
            if (program != 0)
                gl.delete_program(program);

            vertex_buffer = 0;
            vertex_array = 0;
            program = 0;
            initialized = false;
        }

        [[nodiscard]] bool initialize(OpenGLProcLoader loader) noexcept
        {
            if (initialized)
                return true;
            if (!gl.load(loader))
                return false;

            program = create_program(gl);
            if (program == 0)
                return false;

            canvas_uniform = gl.get_uniform_location(program, "uCanvas");
            color_uniform = gl.get_uniform_location(program, "uColor");
            if (canvas_uniform < 0 || color_uniform < 0)
            {
                release();
                return false;
            }

            gl.gen_vertex_arrays(1, &vertex_array);
            gl.bind_vertex_array(vertex_array);
            gl.gen_buffers(1, &vertex_buffer);
            gl.bind_buffer(GL_ARRAY_BUFFER, vertex_buffer);
            gl.enable_vertex_attrib_array(0);
            gl.vertex_attrib_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(gui::Vec2), nullptr);

            initialized = vertex_array != 0 && vertex_buffer != 0;
            if (!initialized)
                release();
            return initialized;
        }

        [[nodiscard]] gui::FloatingWindowOptions window_options(
            gui::Vec2 default_position,
            gui::Vec2 default_size) const noexcept
        {
            return {
                .default_position = default_position,
                .default_size = default_size,
                .min_size = { 270.0f, 190.0f },
                .viewport_size = { canvas_width, canvas_height },
                .title_bar_height = 32.0f,
                .content_padding = 10.0f,
                .movable = true,
                .resizable = true,
                .closable = true
            };
        }

        void clamp_window_to_workspace(gui::FloatingWindowState& state) noexcept
        {
            state.position.y = (std::max)(state.position.y, workspace_top + 8.0f);
            state.position.x = (std::max)(state.position.x, 228.0f);
        }

        void update_sidebar(const input::InputFrame& frame) noexcept
        {
            const gui::SelectableListLayoutOptions options{
                .viewport = { { 18.0f, 156.0f }, { 188.0f, 232.0f } },
                .row_count = 5,
                .row_height = 36.0f,
                .row_gap = 5.0f,
                .scroll_offset = 0.0f,
                .content_padding_x = 8.0f,
                .content_padding_y = 8.0f
            };

            if (frame.pointer(input::PointerButton::left).released)
            {
                const std::uint32_t row = gui::selectable_row_index_at(options, frame.pointer_position);
                if (row != gui::invalid_selectable_row_index)
                    selected_sidebar_row = row;
            }
        }

        void update_display_toggle(const input::InputFrame& frame) noexcept
        {
            constexpr std::array<float, 3> widths{ 92.0f, 102.0f, 92.0f };
            const gui::SegmentedControlLayoutOptions options{
                .position = { 254.0f, 67.0f },
                .item_widths = widths,
                .height = 32.0f,
                .gap = 4.0f
            };

            if (frame.pointer(input::PointerButton::left).released)
            {
                const std::uint32_t item = gui::segmented_control_item_at(options, frame.pointer_position);
                if (item != gui::invalid_selectable_row_index)
                    display_mode = item;
            }
        }

        [[nodiscard]] int press_target(const input::InputFrame& frame) const noexcept
        {
            if (!frame.pointer(input::PointerButton::left).pressed)
                return 0;
            if (context_layout.visible && gui::contains(context_layout.popup, frame.pointer_position))
                return 0;

            const bool in_core = core_window.open
                && display_mode != 1
                && gui::contains({ core_window.position, core_window.size }, frame.pointer_position);
            const bool in_rounded = rounded_window.open
                && display_mode != 0
                && gui::contains({ rounded_window.position, rounded_window.size }, frame.pointer_position);

            if (in_core && in_rounded)
                return core_window.focus_order > rounded_window.focus_order ? 1 : 2;
            if (in_core)
                return 1;
            if (in_rounded)
                return 2;
            return 0;
        }

        void update_floating_windows(const input::InputFrame& frame) noexcept
        {
            const int target = press_target(frame);
            const gui::FloatingWindowInput actual = input::floating_window_input(frame);

            gui::FloatingWindowInput core_input = actual;
            gui::FloatingWindowInput rounded_input = actual;

            const bool core_captured = core_window.dragging || core_window.resizing || target == 1;
            const bool rounded_captured = rounded_window.dragging || rounded_window.resizing || target == 2;

            if (!core_captured)
            {
                core_input.mouse_down = false;
                core_input.mouse_pressed = false;
                core_input.mouse_released = false;
            }
            if (!rounded_captured)
            {
                rounded_input.mouse_down = false;
                rounded_input.mouse_pressed = false;
                rounded_input.mouse_released = false;
            }

            if (display_mode != 1)
            {
                core_layout = gui::update_floating_window(
                    core_window,
                    window_options({ 292.0f, 196.0f }, { 390.0f, 258.0f }),
                    core_input);
                clamp_window_to_workspace(core_window);
                if (core_layout.focused)
                    core_window.focus_order = next_focus_order++;
            }
            else
            {
                core_layout = {};
            }

            if (display_mode != 0)
            {
                rounded_layout = gui::update_floating_window(
                    rounded_window,
                    window_options({ 650.0f, 300.0f }, { 410.0f, 276.0f }),
                    rounded_input);
                clamp_window_to_workspace(rounded_window);
                if (rounded_layout.focused)
                    rounded_window.focus_order = next_focus_order++;
            }
            else
            {
                rounded_layout = {};
            }
        }

        void apply_context_action(std::uint32_t item) noexcept
        {
            switch (item)
            {
            case 0:
                core_window.open = true;
                core_window.position = { context_anchor.x, (std::max)(workspace_top + 8.0f, context_anchor.y) };
                core_window.focus_order = next_focus_order++;
                break;
            case 1:
                rounded_window.open = true;
                rounded_window.position = { context_anchor.x, (std::max)(workspace_top + 8.0f, context_anchor.y) };
                rounded_window.focus_order = next_focus_order++;
                break;
            case 2:
                reset_windows();
                break;
            default:
                break;
            }
            context_popup.open = false;
        }

        void update_context_menu(const input::InputFrame& frame) noexcept
        {
            const gui::Rect workspace{ { 220.0f, workspace_top }, { canvas_width - 220.0f, canvas_height - workspace_top } };
            const input::ContextMenuRequest request = input::context_menu_request(frame, workspace);
            if (request.requested)
                context_anchor = request.position;

            const gui::PopupOptions options{
                .owner = { context_anchor, { 1.0f, 1.0f } },
                .preferred_size = { 230.0f, 126.0f },
                .viewport_size = { canvas_width, canvas_height },
                .cursor_offset = { 0.0f, 0.0f },
                .placement = gui::PopupPlacement::below,
                .gap = 0.0f,
                .margin = 8.0f,
                .flip_to_fit = true,
                .clamp_to_viewport = true,
                .close_on_outside_press = true
            };

            context_layout = gui::update_popup(
                context_popup,
                options,
                input::popup_input(frame, request.requested));

            if (context_layout.visible && frame.pointer(input::PointerButton::left).released)
            {
                for (std::uint32_t item = 0; item < 3; ++item)
                {
                    const gui::Rect item_rect{
                        {
                            context_layout.popup.position.x + 10.0f,
                            context_layout.popup.position.y + 10.0f + static_cast<float>(item) * 36.0f
                        },
                        { context_layout.popup.size.x - 20.0f, 30.0f }
                    };
                    if (gui::contains(item_rect, frame.pointer_position))
                    {
                        apply_context_action(item);
                        break;
                    }
                }
            }
        }

        void update_ui() noexcept
        {
            const input::InputFrame& frame = input_tracker.frame();

            const input::WindowCommand command = input::borderless_window_command(chrome_layout, frame);
            if (command != input::WindowCommand::none)
                pending_window_command = command;

            update_sidebar(frame);
            update_display_toggle(frame);
            update_context_menu(frame);
            update_floating_windows(frame);

            if (frame.key(input::Key::tab).pressed)
                display_mode = (display_mode + 1U) % 3U;
            if (frame.key(input::Key::escape).pressed && !context_popup.open)
                reset_windows();
        }

        void draw_native_title_bar(const input::InputFrame& frame)
        {
            batch.append_rect(chrome_layout.title_bar, title_fill);
            batch.append_rect({ { 0.0f, native_title_height - 1.0f }, { canvas_width, 1.0f } }, panel_border);
            draw_text(batch, 20.0f, 18.0f, "EPOCHGUI LIVE DEMO", 2.3f, text_primary);
            draw_text(batch, 292.0f, 22.0f, "BORDERLESS REPLACEMENT TITLE BAR", 1.35f, text_secondary);

            const input::WindowChromeRegion hovered = input::hit_test_borderless_window_chrome(
                chrome_layout,
                frame.pointer_position);

            const auto draw_button = [&](gui::Rect rect, input::WindowChromeRegion region, Color active, std::string_view label)
            {
                const bool is_hovered = hovered == region;
                batch.append_rect(rect, is_hovered ? active : title_fill);
                draw_text(batch, rect.position.x + 17.0f, rect.position.y + 20.0f, label, 1.35f,
                    is_hovered ? text_primary : text_secondary);
            };

            draw_button(chrome_layout.minimize_button, input::WindowChromeRegion::minimize_button, control_hover, "-");
            draw_button(chrome_layout.maximize_button, input::WindowChromeRegion::maximize_button, control_hover, "O");
            draw_button(chrome_layout.close_button, input::WindowChromeRegion::close_button, danger, "X");
        }

        void draw_toolbar(const input::InputFrame& frame)
        {
            batch.append_rect({ { 0.0f, native_title_height }, { canvas_width, toolbar_height } }, toolbar_fill);
            batch.append_rect({ { 0.0f, workspace_top - 1.0f }, { canvas_width, 1.0f } }, panel_border);

            draw_text(batch, 22.0f, 75.0f, "WINDOW STYLE", 1.4f, text_secondary);

            constexpr std::array<float, 3> widths{ 92.0f, 102.0f, 92.0f };
            const gui::SegmentedControlLayoutOptions options{
                .position = { 254.0f, 67.0f },
                .item_widths = widths,
                .height = 32.0f,
                .gap = 4.0f
            };
            constexpr std::array<std::string_view, 3> labels{ "RECT", "ROUNDED", "BOTH" };

            for (std::uint32_t index = 0; index < labels.size(); ++index)
            {
                const gui::Rect item = gui::segmented_control_item_layout(options, index);
                const bool hovered = gui::contains(item, frame.pointer_position);
                const bool selected = display_mode == index;
                draw_surface(batch, item, true, 7.0f,
                    selected ? control_selected : hovered ? control_hover : control_fill,
                    selected ? accent : panel_border,
                    1.0f);
                draw_text(batch, item.position.x + 16.0f, item.position.y + 11.0f,
                    labels[index], 1.35f, selected ? text_primary : text_secondary);
            }

            draw_surface(batch, { { 585.0f, 67.0f }, { 202.0f, 32.0f } }, true, 16.0f,
                accent_soft, accent, 1.0f);
            draw_text(batch, 604.0f, 78.0f, "INPUT FALLBACK ON", 1.35f, text_primary);
            draw_text(batch, 824.0f, 77.0f, "RIGHT CLICK WORKSPACE - TAB CHANGES MODE", 1.2f, text_muted);
        }

        void draw_sidebar(const input::InputFrame& frame)
        {
            batch.append_rect({ { 0.0f, workspace_top }, { 220.0f, canvas_height - workspace_top } }, sidebar_fill);
            batch.append_rect({ { 219.0f, workspace_top }, { 1.0f, canvas_height - workspace_top } }, panel_border);
            draw_text(batch, 18.0f, 132.0f, "EPOCHGUI FEATURES", 1.55f, text_primary);

            const gui::SelectableListLayoutOptions options{
                .viewport = { { 18.0f, 156.0f }, { 188.0f, 232.0f } },
                .row_count = 5,
                .row_height = 36.0f,
                .row_gap = 5.0f,
                .scroll_offset = 0.0f,
                .content_padding_x = 8.0f,
                .content_padding_y = 8.0f
            };
            constexpr std::array<std::string_view, 5> labels{
                "LAYOUT", "FLOATING", "POPUPS", "INPUT", "CHROME"
            };

            for (std::uint32_t index = 0; index < labels.size(); ++index)
            {
                const gui::SelectableRowLayout row = gui::make_selectable_row_layout(
                    options,
                    index,
                    frame.pointer_position,
                    selected_sidebar_row == index);
                if (!row.visible)
                    continue;

                draw_surface(batch, row.row, true, 7.0f,
                    row.selected ? control_selected : row.hovered ? control_hover : control_fill,
                    row.selected ? accent : panel_border,
                    row.selected ? 1.0f : 0.0f);
                draw_text(batch, row.content.position.x + 8.0f, row.content.position.y + 11.0f,
                    labels[index], 1.45f, row.selected ? text_primary : text_secondary);
            }

            draw_text(batch, 18.0f, 424.0f, "LIVE INPUT", 1.35f, text_secondary);
            draw_text(batch, 18.0f, 452.0f,
                frame.pointer(input::PointerButton::left).down ? "LEFT DOWN" : "LEFT UP",
                1.35f,
                frame.pointer(input::PointerButton::left).down ? green : text_muted);
            draw_text(batch, 18.0f, 476.0f,
                frame.pointer(input::PointerButton::right).down ? "RIGHT DOWN" : "RIGHT UP",
                1.35f,
                frame.pointer(input::PointerButton::right).down ? warning : text_muted);
            draw_text(batch, 18.0f, 516.0f, "DRAG TITLE BARS", 1.2f, text_muted);
            draw_text(batch, 18.0f, 538.0f, "RESIZE CORNERS", 1.2f, text_muted);
            draw_text(batch, 18.0f, 560.0f, "CLOSE AND REOPEN", 1.2f, text_muted);
            draw_text(batch, 18.0f, 582.0f, "FROM RIGHT CLICK", 1.2f, text_muted);
        }

        void draw_window_content(gui::Rect content, bool use_rounded)
        {
            draw_text(batch, content.position.x + 8.0f, content.position.y + 8.0f,
                use_rounded ? "OPTIONAL ROUNDED GEOMETRY" : "CORE RECTANGULAR FALLBACK",
                1.25f,
                use_rounded ? green : accent);

            const gui::ProgressBarLayout progress = gui::make_progress_bar_layout({
                .track = {
                    { content.position.x + 8.0f, content.position.y + 38.0f },
                    { (std::max)(80.0f, content.size.x - 16.0f), 22.0f }
                },
                .value = use_rounded ? 0.76f : 0.54f,
                .minimum = 0.0f,
                .maximum = 1.0f,
                .padding = 3.0f,
                .direction = gui::ProgressBarDirection::left_to_right
            });
            draw_surface(batch, progress.track, use_rounded, 7.0f, background, panel_border, 1.0f);
            draw_surface(batch, progress.fill, use_rounded, 5.0f,
                use_rounded ? green : accent,
                use_rounded ? green : accent,
                0.0f);

            const gui::SplitterLayout split = gui::make_splitter_layout({
                .area = {
                    { content.position.x + 8.0f, content.position.y + 76.0f },
                    { (std::max)(120.0f, content.size.x - 16.0f), (std::max)(72.0f, content.size.y - 86.0f) }
                },
                .axis = gui::SplitterAxis::vertical,
                .split_fraction = 0.36f,
                .thickness = 6.0f,
                .min_before = 70.0f,
                .min_after = 90.0f
            });
            draw_surface(batch, split.before, use_rounded, 6.0f, background, panel_border, 1.0f);
            draw_surface(batch, split.after, use_rounded, 6.0f, control_fill, panel_border, 1.0f);
            draw_surface(batch, split.handle, use_rounded, 3.0f, accent_soft, accent_soft, 0.0f);
            draw_text(batch, split.before.position.x + 10.0f, split.before.position.y + 12.0f,
                "TREE", 1.25f, text_secondary);
            draw_text(batch, split.after.position.x + 10.0f, split.after.position.y + 12.0f,
                "CONTENT", 1.25f, text_secondary);
        }

        void draw_floating_window(
            const gui::FloatingWindowState& state,
            const gui::FloatingWindowLayout& layout,
            bool use_rounded,
            bool active)
        {
            if (!state.open || !layout.visible)
                return;

            draw_surface(batch, layout.window, use_rounded, 11.0f,
                panel_fill,
                active ? accent : panel_border,
                active ? 2.0f : 1.0f);
            draw_surface(batch, layout.title_bar, use_rounded, 10.0f,
                active ? control_selected : control_fill,
                active ? control_selected : control_fill,
                0.0f);
            batch.append_rect({
                { layout.title_bar.position.x, layout.title_bar.position.y + layout.title_bar.size.y - 8.0f },
                { layout.title_bar.size.x, 8.0f }
            }, active ? control_selected : control_fill);

            draw_text(batch, layout.title_bar.position.x + 12.0f, layout.title_bar.position.y + 11.0f,
                use_rounded ? "ROUNDED WINDOW" : "CORE WINDOW",
                1.45f,
                text_primary);

            draw_surface(batch, layout.close_button, true, 6.0f,
                layout.close_hovered ? danger : accent_soft,
                layout.close_hovered ? danger : accent_soft,
                0.0f);
            draw_text(batch, layout.close_button.position.x + 8.0f, layout.close_button.position.y + 8.0f,
                "X", 1.15f, text_primary);

            draw_window_content(layout.content, use_rounded);

            draw_surface(batch, layout.resize_handle, false, 0.0f,
                layout.resize_hovered ? warning : panel_border,
                panel_border,
                0.0f);
        }

        void draw_context_menu(const input::InputFrame& frame)
        {
            if (!context_layout.visible)
                return;

            draw_surface(batch, context_layout.popup, true, 10.0f, panel_fill, accent, 1.5f);
            constexpr std::array<std::string_view, 3> labels{
                "OPEN CORE WINDOW", "OPEN ROUNDED WINDOW", "RESET LAYOUT"
            };

            for (std::uint32_t index = 0; index < labels.size(); ++index)
            {
                const gui::Rect item{
                    {
                        context_layout.popup.position.x + 10.0f,
                        context_layout.popup.position.y + 10.0f + static_cast<float>(index) * 36.0f
                    },
                    { context_layout.popup.size.x - 20.0f, 30.0f }
                };
                const bool hovered = gui::contains(item, frame.pointer_position);
                draw_surface(batch, item, true, 6.0f,
                    hovered ? control_hover : background,
                    hovered ? accent_soft : background,
                    0.0f);
                draw_text(batch, item.position.x + 10.0f, item.position.y + 10.0f,
                    labels[index], 1.2f, hovered ? text_primary : text_secondary);
            }
        }

        void build_scene()
        {
            batch.clear();
            batch.vertices.reserve(30000);
            batch.commands.reserve(2200);

            const input::InputFrame& frame = input_tracker.frame();
            batch.append_rect({ { 0.0f, 0.0f }, { canvas_width, canvas_height } }, background);
            draw_native_title_bar(frame);
            draw_toolbar(frame);
            draw_sidebar(frame);
            batch.append_rect({
                { 220.0f, workspace_top },
                { canvas_width - 220.0f, canvas_height - workspace_top }
            }, workspace_fill);

            draw_text(batch, 246.0f, 132.0f,
                "LIVE WORKSPACE - WINDOWS USE EPOCHGUI STATE AND HIT TESTING",
                1.35f,
                text_muted);

            const bool core_active = core_window.focus_order >= rounded_window.focus_order;
            if (core_active)
            {
                draw_floating_window(rounded_window, rounded_layout, true, false);
                draw_floating_window(core_window, core_layout, false, true);
            }
            else
            {
                draw_floating_window(core_window, core_layout, false, false);
                draw_floating_window(rounded_window, rounded_layout, true, true);
            }

            if (!core_window.open && !rounded_window.open)
            {
                draw_surface(batch, { { 430.0f, 330.0f }, { 520.0f, 150.0f } }, true, 14.0f,
                    panel_fill, panel_border, 1.0f);
                draw_text(batch, 482.0f, 372.0f, "ALL WINDOWS CLOSED", 2.0f, warning);
                draw_text(batch, 470.0f, 414.0f, "RIGHT CLICK TO REOPEN OR RESET", 1.35f, text_secondary);
            }

            draw_context_menu(frame);
        }

        void render() noexcept
        {
            if (!initialized)
                return;

            update_ui();
            build_scene();

            gl.viewport(0, 0, framebuffer_width, framebuffer_height);
            gl.clear_color(background.r, background.g, background.b, background.a);
            gl.clear(GL_COLOR_BUFFER_BIT);
            gl.use_program(program);
            gl.uniform_2f(canvas_uniform, canvas_width, canvas_height);
            gl.bind_vertex_array(vertex_array);
            gl.bind_buffer(GL_ARRAY_BUFFER, vertex_buffer);
            gl.buffer_data(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(batch.vertices.size() * sizeof(gui::Vec2)),
                batch.vertices.empty() ? nullptr : batch.vertices.data(),
                GL_DYNAMIC_DRAW);

            for (const DrawCommand& command : batch.commands)
            {
                gl.uniform_4f(
                    color_uniform,
                    command.color.r,
                    command.color.g,
                    command.color.b,
                    command.color.a);
                gl.draw_arrays(GL_TRIANGLES, command.first, command.count);
            }

            input_tracker.finish_frame();
        }
    };

    OpenGLRenderer::OpenGLRenderer()
        : impl_(std::make_unique<Impl>())
    {
    }

    OpenGLRenderer::~OpenGLRenderer() = default;
    OpenGLRenderer::OpenGLRenderer(OpenGLRenderer&&) noexcept = default;
    OpenGLRenderer& OpenGLRenderer::operator=(OpenGLRenderer&&) noexcept = default;

    bool OpenGLRenderer::initialize(OpenGLProcLoader loader) noexcept
    {
        return impl_ && impl_->initialize(loader);
    }

    void OpenGLRenderer::resize(int width, int height) noexcept
    {
        if (!impl_)
            return;
        impl_->framebuffer_width = (std::max)(1, width);
        impl_->framebuffer_height = (std::max)(1, height);
    }

    void OpenGLRenderer::pointer_move(float x, float y) noexcept
    {
        if (impl_)
            impl_->input_tracker.set_pointer_position(impl_->to_canvas(x, y));
    }

    void OpenGLRenderer::pointer_button(PointerButton button, bool down) noexcept
    {
        if (impl_)
            impl_->input_tracker.set_pointer_button(button, down);
    }

    void OpenGLRenderer::wheel(float horizontal, float vertical) noexcept
    {
        if (impl_)
            impl_->input_tracker.add_wheel_delta({ horizontal, vertical });
    }

    void OpenGLRenderer::key(Key key_code, bool down, bool repeated) noexcept
    {
        if (impl_)
            impl_->input_tracker.set_key(key_code, down, repeated);
    }

    void OpenGLRenderer::modifiers(input::ModifierState state) noexcept
    {
        if (impl_)
            impl_->input_tracker.set_modifiers(state);
    }

    void OpenGLRenderer::render() noexcept
    {
        if (impl_)
            impl_->render();
    }

    WindowChromeRegion OpenGLRenderer::window_chrome_hit_test(float x, float y) const noexcept
    {
        if (!impl_)
            return WindowChromeRegion::client;
        return input::hit_test_borderless_window_chrome(
            impl_->chrome_layout,
            impl_->to_canvas(x, y));
    }

    WindowCommand OpenGLRenderer::take_window_command() noexcept
    {
        if (!impl_)
            return WindowCommand::none;
        return std::exchange(impl_->pending_window_command, WindowCommand::none);
    }
}
