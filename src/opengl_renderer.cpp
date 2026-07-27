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

namespace epochengine::gui_demo
{
    namespace
    {
        namespace gui = epochengine::gui_lib;
        namespace rounded = epochengine::gui_lib::rounded_rect;

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

        constexpr Color background = rgb(0x0b1020);
        constexpr Color header_fill = rgb(0x111a2f);
        constexpr Color panel_fill = rgb(0x121b2d);
        constexpr Color panel_border = rgb(0x2a3a55);
        constexpr Color control_fill = rgb(0x1a263d);
        constexpr Color control_hover = rgb(0x233653);
        constexpr Color control_selected = rgb(0x315a8a);
        constexpr Color content_fill = rgb(0x0f1728);
        constexpr Color accent = rgb(0x65a7ff);
        constexpr Color accent_soft = rgb(0x244b77);
        constexpr Color green = rgb(0x6ee7b7);
        constexpr Color warning = rgb(0xf7c873);
        constexpr Color text_primary = rgb(0xe8eef8);
        constexpr Color text_secondary = rgb(0x9fb0c8);
        constexpr Color text_muted = rgb(0x71829b);

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

        void draw_segmented_control(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            constexpr std::array<float, 3> widths{ 104.0f, 104.0f, 104.0f };
            const gui::SegmentedControlLayoutOptions options{
                .position = { x, y },
                .item_widths = widths,
                .height = 34.0f,
                .gap = 4.0f
            };

            for (std::uint32_t index = 0; index < widths.size(); ++index)
            {
                const gui::Rect item = gui::segmented_control_item_layout(options, index);
                draw_surface(
                    batch,
                    item,
                    use_rounded,
                    8.0f,
                    index == 1 ? control_selected : control_fill,
                    index == 1 ? accent : panel_border,
                    1.0f);
            }

            draw_text(batch, x + 27.0f, y + 11.0f, "LAYOUT", 1.5f, text_secondary);
            draw_text(batch, x + 126.0f, y + 11.0f, "STATE", 1.5f, text_primary);
            draw_text(batch, x + 234.0f, y + 11.0f, "INPUT", 1.5f, text_secondary);
        }

        void draw_progress(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            const gui::ProgressBarLayout progress = gui::make_progress_bar_layout({
                .track = { { x, y }, { 332.0f, 24.0f } },
                .value = 72.0f,
                .minimum = 0.0f,
                .maximum = 100.0f,
                .padding = 3.0f,
                .direction = gui::ProgressBarDirection::left_to_right
            });

            draw_surface(batch, progress.track, use_rounded, 7.0f, content_fill, panel_border, 1.0f);
            draw_surface(batch, progress.fill, use_rounded, 5.0f, accent, accent, 0.0f);
            draw_text(batch, x + 272.0f, y + 8.0f, "72", 1.4f, text_primary);
        }

        void draw_splitter(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            const gui::SplitterLayout split = gui::make_splitter_layout({
                .area = { { x, y }, { 552.0f, 104.0f } },
                .axis = gui::SplitterAxis::vertical,
                .split_fraction = 0.38f,
                .thickness = 8.0f,
                .min_before = 100.0f,
                .min_after = 160.0f
            });

            draw_surface(batch, split.before, use_rounded, 8.0f, content_fill, panel_border, 1.0f);
            draw_surface(batch, split.after, use_rounded, 8.0f, control_fill, panel_border, 1.0f);
            draw_surface(batch, split.handle, use_rounded, 4.0f, accent_soft, accent_soft, 0.0f);

            draw_text(batch, split.before.position.x + 18.0f, split.before.position.y + 18.0f, "TREE", 1.6f, text_secondary);
            draw_text(batch, split.after.position.x + 18.0f, split.after.position.y + 18.0f, "CONTENT", 1.6f, text_secondary);

            for (int row = 0; row < 3; ++row)
            {
                draw_surface(batch, {
                    { split.before.position.x + 18.0f, split.before.position.y + 46.0f + row * 16.0f },
                    { 126.0f + row * 12.0f, 7.0f }
                }, use_rounded, 3.0f, row == 1 ? accent_soft : panel_border, panel_border, 0.0f);
            }

            for (int row = 0; row < 3; ++row)
            {
                draw_surface(batch, {
                    { split.after.position.x + 18.0f, split.after.position.y + 46.0f + row * 16.0f },
                    { 244.0f - row * 22.0f, 7.0f }
                }, use_rounded, 3.0f, row == 0 ? green : panel_border, panel_border, 0.0f);
            }
        }

        void draw_selectable_list(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            const gui::SelectableListLayoutOptions options{
                .viewport = { { x, y }, { 266.0f, 150.0f } },
                .row_count = 4,
                .row_height = 31.0f,
                .row_gap = 4.0f,
                .scroll_offset = 0.0f,
                .content_padding_x = 8.0f,
                .content_padding_y = 7.0f
            };

            draw_surface(batch, options.viewport, use_rounded, 8.0f, content_fill, panel_border, 1.0f);

            for (std::uint32_t index = 0; index < options.row_count; ++index)
            {
                const bool selected = index == 1;
                const gui::Vec2 pointer{
                    options.viewport.position.x + 20.0f,
                    options.viewport.position.y + 7.0f + 2.0f * (options.row_height + options.row_gap) + 10.0f
                };
                const gui::SelectableRowLayout row = gui::make_selectable_row_layout(
                    options,
                    index,
                    pointer,
                    selected);
                if (!row.visible)
                    continue;

                Color row_color = control_fill;
                if (row.hovered)
                    row_color = control_hover;
                if (row.selected)
                    row_color = control_selected;

                draw_surface(
                    batch,
                    row.row,
                    use_rounded,
                    6.0f,
                    row_color,
                    row.selected ? accent : row_color,
                    row.selected ? 1.0f : 0.0f);

                constexpr std::array<std::string_view, 4> labels{
                    "SCENE", "INSPECTOR", "ASSETS", "CONSOLE"
                };
                draw_text(batch, row.content.position.x + 4.0f, row.content.position.y + 9.0f, labels[index], 1.5f,
                    row.selected ? text_primary : text_secondary);
            }
        }

        void draw_popup(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            const gui::Rect owner{ { x, y }, { 218.0f, 34.0f } };
            draw_surface(batch, owner, use_rounded, 7.0f, control_selected, accent, 1.0f);
            draw_text(batch, x + 20.0f, y + 11.0f, "OPEN POPUP", 1.5f, text_primary);

            gui::PopupPlacement placement{};
            bool flipped{};
            bool clamped{};
            const gui::Rect popup = gui::place_popup({
                .owner = owner,
                .preferred_size = { 218.0f, 110.0f },
                .viewport_size = { canvas_width, canvas_height },
                .placement = gui::PopupPlacement::below,
                .gap = 6.0f,
                .margin = 10.0f,
                .flip_to_fit = true,
                .clamp_to_viewport = true
            }, {}, &placement, &flipped, &clamped);

            draw_surface(batch, popup, use_rounded, 10.0f, panel_fill, accent_soft, 1.5f);
            constexpr std::array<std::string_view, 3> items{ "NEW PANEL", "DUPLICATE", "CLOSE" };
            for (std::size_t index = 0; index < items.size(); ++index)
            {
                const gui::Rect item{
                    { popup.position.x + 10.0f, popup.position.y + 10.0f + static_cast<float>(index) * 31.0f },
                    { popup.size.x - 20.0f, 26.0f }
                };
                draw_surface(batch, item, use_rounded, 5.0f,
                    index == 0 ? control_hover : content_fill,
                    index == 0 ? accent_soft : content_fill,
                    0.0f);
                draw_text(batch, item.position.x + 10.0f, item.position.y + 8.0f, items[index], 1.4f,
                    index == 2 ? warning : text_secondary);
            }
        }

        void draw_floating_window(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            gui::FloatingWindowState state{
                .position = { x, y },
                .size = { 250.0f, 104.0f },
                .open = true,
                .initialized = true,
                .focus_order = 3
            };
            const gui::FloatingWindowLayout layout = gui::update_floating_window(
                state,
                {
                    .default_position = { x, y },
                    .default_size = { 250.0f, 104.0f },
                    .min_size = { 180.0f, 90.0f },
                    .viewport_size = { canvas_width, canvas_height },
                    .title_bar_height = 25.0f,
                    .content_padding = 6.0f,
                    .movable = true,
                    .resizable = true,
                    .closable = true
                },
                {});

            draw_surface(batch, layout.window, use_rounded, 9.0f, panel_fill, accent_soft, 1.5f);
            draw_surface(batch, layout.title_bar, use_rounded, 8.0f, control_selected, control_selected, 0.0f);
            batch.append_rect({
                { layout.title_bar.position.x, layout.title_bar.position.y + layout.title_bar.size.y - 8.0f },
                { layout.title_bar.size.x, 8.0f }
            }, control_selected);
            draw_text(batch, layout.title_bar.position.x + 10.0f, layout.title_bar.position.y + 8.0f,
                "FLOATING WINDOW", 1.35f, text_primary);
            draw_surface(batch, layout.close_button, true, 6.0f, warning, warning, 0.0f);
            draw_surface(batch, {
                { layout.content.position.x + 10.0f, layout.content.position.y + 13.0f },
                { layout.content.size.x - 20.0f, 8.0f }
            }, use_rounded, 3.0f, panel_border, panel_border, 0.0f);
            draw_surface(batch, {
                { layout.content.position.x + 10.0f, layout.content.position.y + 32.0f },
                { layout.content.size.x * 0.58f, 8.0f }
            }, use_rounded, 3.0f, accent_soft, accent_soft, 0.0f);
        }

        void draw_loading_layout(SceneBatch& batch, float x, float y, bool use_rounded)
        {
            const gui::LoadingScreenLayout loading = gui::make_loading_screen_layout({
                .viewport = { { x, y }, { 552.0f, 126.0f } },
                .preferred_panel_size = { 552.0f, 126.0f },
                .minimum_panel_size = { 420.0f, 110.0f },
                .margin = 0.0f,
                .padding = 12.0f,
                .gap = 6.0f,
                .title_height = 14.0f,
                .message_height = 18.0f,
                .progress_height = 18.0f,
                .status_height = 12.0f,
                .action_height = 22.0f,
                .progress_padding = 2.0f,
                .progress_value = 0.64f
            });

            draw_surface(batch, loading.panel, use_rounded, 10.0f, content_fill, panel_border, 1.0f);
            draw_text(batch, loading.title.position.x, loading.title.position.y + 2.0f, "LOADING LAYOUT", 1.5f, text_primary);
            draw_surface(batch, {
                { loading.message.position.x, loading.message.position.y + 5.0f },
                { loading.message.size.x * 0.72f, 7.0f }
            }, use_rounded, 3.0f, panel_border, panel_border, 0.0f);
            draw_surface(batch, loading.progress.track, use_rounded, 5.0f, panel_fill, panel_border, 1.0f);
            draw_surface(batch, loading.progress.fill, use_rounded, 4.0f, green, green, 0.0f);
            draw_text(batch, loading.status.position.x, loading.status.position.y + 1.0f, "64 PERCENT", 1.25f, text_secondary);
            draw_surface(batch, loading.action, use_rounded, 6.0f, control_fill, panel_border, 1.0f);
        }

        void draw_column(SceneBatch& batch, float x, bool use_rounded)
        {
            const gui::Rect column{ { x, 96.0f }, { 604.0f, 700.0f } };
            draw_surface(batch, column, use_rounded, 14.0f, panel_fill, panel_border, 1.5f);

            draw_text(batch, x + 24.0f, 116.0f,
                use_rounded ? "OPTIONAL ROUNDED" : "CORE RECTANGULAR",
                2.0f,
                use_rounded ? green : accent);
            draw_text(batch, x + 24.0f, 142.0f,
                use_rounded ? "EPOCH.GUI.ROUNDED_RECT ENABLED" : "EPOCH.GUI CORE LAYOUT ONLY",
                1.25f,
                text_muted);

            draw_text(batch, x + 24.0f, 176.0f, "SEGMENTED CONTROL", 1.35f, text_secondary);
            draw_segmented_control(batch, x + 24.0f, 196.0f, use_rounded);

            draw_text(batch, x + 24.0f, 246.0f, "PROGRESS BAR", 1.35f, text_secondary);
            draw_progress(batch, x + 24.0f, 266.0f, use_rounded);

            draw_text(batch, x + 24.0f, 312.0f, "SPLITTER AND PANES", 1.35f, text_secondary);
            draw_splitter(batch, x + 24.0f, 332.0f, use_rounded);
            draw_floating_window(batch, x + 326.0f, 340.0f, use_rounded);

            draw_text(batch, x + 24.0f, 458.0f, "SELECTABLE LIST", 1.35f, text_secondary);
            draw_selectable_list(batch, x + 24.0f, 478.0f, use_rounded);

            draw_text(batch, x + 310.0f, 458.0f, "POPUP LAYOUT", 1.35f, text_secondary);
            draw_popup(batch, x + 310.0f, 478.0f, use_rounded);

            draw_text(batch, x + 24.0f, 642.0f, "LOADING SCREEN LAYOUT", 1.35f, text_secondary);
            draw_loading_layout(batch, x + 24.0f, 662.0f, use_rounded);
        }

        void build_scene(SceneBatch& batch)
        {
            batch.vertices.clear();
            batch.commands.clear();
            batch.vertices.reserve(24000);
            batch.commands.reserve(1800);

            batch.append_rect({ { 0.0f, 0.0f }, { canvas_width, canvas_height } }, background);
            batch.append_rect({ { 0.0f, 0.0f }, { canvas_width, 80.0f } }, header_fill);
            batch.append_rect({ { 0.0f, 78.0f }, { canvas_width, 2.0f } }, accent_soft);

            draw_text(batch, 28.0f, 22.0f, "EPOCHGUI DEMO", 3.0f, text_primary);
            draw_text(batch, 307.0f, 31.0f, "CORE LAYOUTS VS OPTIONAL ROUNDED GEOMETRY", 1.5f, text_secondary);
            draw_surface(batch, { { 1100.0f, 21.0f }, { 150.0f, 34.0f } }, true, 17.0f, accent_soft, accent, 1.0f);
            draw_text(batch, 1122.0f, 32.0f, "EPOCHGUI", 1.7f, text_primary);

            draw_column(batch, 24.0f, false);
            draw_column(batch, 652.0f, true);
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
        SceneBatch scene{};
        GLuint program{};
        GLuint vertex_array{};
        GLuint vertex_buffer{};
        GLint canvas_uniform{ -1 };
        GLint color_uniform{ -1 };
        int framebuffer_width{ static_cast<int>(canvas_width) };
        int framebuffer_height{ static_cast<int>(canvas_height) };
        bool initialized{};

        ~Impl()
        {
            release();
        }

        void release() noexcept
        {
            if (!initialized)
                return;

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
                return false;

            gl.gen_vertex_arrays(1, &vertex_array);
            gl.bind_vertex_array(vertex_array);
            gl.gen_buffers(1, &vertex_buffer);
            gl.bind_buffer(GL_ARRAY_BUFFER, vertex_buffer);
            gl.enable_vertex_attrib_array(0);
            gl.vertex_attrib_pointer(
                0,
                2,
                GL_FLOAT,
                GL_FALSE,
                static_cast<GLsizei>(sizeof(gui::Vec2)),
                nullptr);
            gl.bind_vertex_array(0);

            initialized = vertex_array != 0 && vertex_buffer != 0;
            return initialized;
        }

        void resize(int width, int height) noexcept
        {
            framebuffer_width = (std::max)(1, width);
            framebuffer_height = (std::max)(1, height);
        }

        void render() noexcept
        {
            if (!initialized)
                return;

            build_scene(scene);

            gl.viewport(0, 0, framebuffer_width, framebuffer_height);
            gl.clear_color(background.r, background.g, background.b, background.a);
            gl.clear(GL_COLOR_BUFFER_BIT);

            const float target_aspect = canvas_width / canvas_height;
            const float framebuffer_aspect =
                static_cast<float>(framebuffer_width) / static_cast<float>(framebuffer_height);

            int viewport_width = framebuffer_width;
            int viewport_height = framebuffer_height;
            int viewport_x = 0;
            int viewport_y = 0;

            if (framebuffer_aspect > target_aspect)
            {
                viewport_width = static_cast<int>(static_cast<float>(framebuffer_height) * target_aspect);
                viewport_x = (framebuffer_width - viewport_width) / 2;
            }
            else
            {
                viewport_height = static_cast<int>(static_cast<float>(framebuffer_width) / target_aspect);
                viewport_y = (framebuffer_height - viewport_height) / 2;
            }

            gl.viewport(viewport_x, viewport_y, viewport_width, viewport_height);
            gl.use_program(program);
            gl.uniform_2f(canvas_uniform, canvas_width, canvas_height);
            gl.bind_vertex_array(vertex_array);
            gl.bind_buffer(GL_ARRAY_BUFFER, vertex_buffer);
            gl.buffer_data(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(gui::Vec2)),
                scene.vertices.data(),
                GL_DYNAMIC_DRAW);

            for (const DrawCommand& command : scene.commands)
            {
                gl.uniform_4f(
                    color_uniform,
                    command.color.r,
                    command.color.g,
                    command.color.b,
                    command.color.a);
                gl.draw_arrays(GL_TRIANGLES, command.first, command.count);
            }

            gl.bind_vertex_array(0);
            gl.use_program(0);
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
        if (impl_)
            impl_->resize(width, height);
    }

    void OpenGLRenderer::render() noexcept
    {
        if (impl_)
            impl_->render();
    }
}
