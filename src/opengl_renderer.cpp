module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

module epoch.gui.demo.opengl;

import epoch.gui.rounded_rect;

namespace epochengine::gui_demo
{
    namespace
    {
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
        constexpr GLenum GL_ELEMENT_ARRAY_BUFFER = 0x8893;
        constexpr GLenum GL_STATIC_DRAW = 0x88E4;
        constexpr GLenum GL_FLOAT = 0x1406;
        constexpr GLenum GL_UNSIGNED_INT = 0x1405;
        constexpr GLenum GL_TRIANGLES = 0x0004;
        constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
        constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
        constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
        constexpr GLenum GL_LINK_STATUS = 0x8B82;
        constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x00004000;
        constexpr GLboolean GL_FALSE = 0;

        constexpr float canvas_width = 940.0f;
        constexpr float canvas_height = 590.0f;

        struct Color
        {
            float r{};
            float g{};
            float b{};
            float a{ 1.0f };
        };

        [[nodiscard]] constexpr Color rgb(std::uint32_t value) noexcept
        {
            return {
                static_cast<float>((value >> 16U) & 0xffU) / 255.0f,
                static_cast<float>((value >> 8U) & 0xffU) / 255.0f,
                static_cast<float>(value & 0xffU) / 255.0f,
                1.0f
            };
        }

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
            using DrawElements = void (*)(GLenum, GLsizei, GLenum, const void*);
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
            DrawElements draw_elements{};
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
                    && load_function(draw_elements, loader, "glDrawElements")
                    && load_function(clear_color, loader, "glClearColor")
                    && load_function(clear, loader, "glClear")
                    && load_function(viewport, loader, "glViewport");
            }
        };

        struct DrawMesh
        {
            GLuint vertex_array{};
            GLuint vertex_buffer{};
            GLuint fill_index_buffer{};
            GLuint border_index_buffer{};
            GLsizei fill_index_count{};
            GLsizei border_index_count{};
            Color fill{};
            Color border{};
        };

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

        [[nodiscard]] std::array<rounded_rect::RoundedRectOptions, 4> demo_options() noexcept
        {
            return {
                rounded_rect::RoundedRectOptions{
                    .bounds = { { 70.0f, 110.0f }, { 360.0f, 120.0f } },
                    .radii = { 24.0f, 24.0f, 24.0f, 24.0f },
                    .border_width = 4.0f,
                    .segments_per_corner = 12
                },
                rounded_rect::RoundedRectOptions{
                    .bounds = { { 510.0f, 126.0f }, { 360.0f, 88.0f } },
                    .radii = { 44.0f, 44.0f, 44.0f, 44.0f },
                    .border_width = 3.0f,
                    .segments_per_corner = 16
                },
                rounded_rect::RoundedRectOptions{
                    .bounds = { { 70.0f, 340.0f }, { 360.0f, 150.0f } },
                    .radii = { 8.0f, 44.0f, 12.0f, 36.0f },
                    .border_width = 5.0f,
                    .segments_per_corner = 12
                },
                rounded_rect::RoundedRectOptions{
                    .bounds = { { 510.0f, 350.0f }, { 360.0f, 130.0f } },
                    .radii = { 160.0f, 120.0f, 180.0f, 140.0f },
                    .border_width = 6.0f,
                    .segments_per_corner = 16
                }
            };
        }

        constexpr std::array<Color, 4> fill_colors{
            rgb(0x182235), rgb(0x243248), rgb(0x231c32), rgb(0x1b302b)
        };

        constexpr std::array<Color, 4> border_colors{
            rgb(0x65a7ff), rgb(0x7dd3fc), rgb(0xc084fc), rgb(0x6ee7b7)
        };
    }

    struct OpenGLRenderer::Impl
    {
        OpenGLFunctions gl{};
        std::array<DrawMesh, 4> meshes{};
        GLuint program{};
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

            for (DrawMesh& mesh : meshes)
            {
                if (mesh.border_index_buffer != 0)
                    gl.delete_buffers(1, &mesh.border_index_buffer);
                if (mesh.fill_index_buffer != 0)
                    gl.delete_buffers(1, &mesh.fill_index_buffer);
                if (mesh.vertex_buffer != 0)
                    gl.delete_buffers(1, &mesh.vertex_buffer);
                if (mesh.vertex_array != 0)
                    gl.delete_vertex_arrays(1, &mesh.vertex_array);
                mesh = {};
            }

            if (program != 0)
                gl.delete_program(program);
            program = 0;
            initialized = false;
        }

        [[nodiscard]] bool upload_mesh(
            DrawMesh& destination,
            const rounded_rect::RoundedRectMesh& source) noexcept
        {
            if (!source.valid || source.vertices.empty() || source.fill_indices.empty())
                return false;

            gl.gen_vertex_arrays(1, &destination.vertex_array);
            gl.bind_vertex_array(destination.vertex_array);

            gl.gen_buffers(1, &destination.vertex_buffer);
            gl.bind_buffer(GL_ARRAY_BUFFER, destination.vertex_buffer);
            gl.buffer_data(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(source.vertices.size() * sizeof(source.vertices.front())),
                source.vertices.data(),
                GL_STATIC_DRAW);

            gl.enable_vertex_attrib_array(0);
            gl.vertex_attrib_pointer(
                0,
                2,
                GL_FLOAT,
                GL_FALSE,
                static_cast<GLsizei>(sizeof(source.vertices.front())),
                nullptr);

            gl.gen_buffers(1, &destination.fill_index_buffer);
            gl.bind_buffer(GL_ELEMENT_ARRAY_BUFFER, destination.fill_index_buffer);
            gl.buffer_data(
                GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(source.fill_indices.size() * sizeof(source.fill_indices.front())),
                source.fill_indices.data(),
                GL_STATIC_DRAW);
            destination.fill_index_count = static_cast<GLsizei>(source.fill_indices.size());

            if (!source.border_indices.empty())
            {
                gl.gen_buffers(1, &destination.border_index_buffer);
                gl.bind_buffer(GL_ELEMENT_ARRAY_BUFFER, destination.border_index_buffer);
                gl.buffer_data(
                    GL_ELEMENT_ARRAY_BUFFER,
                    static_cast<GLsizeiptr>(source.border_indices.size() * sizeof(source.border_indices.front())),
                    source.border_indices.data(),
                    GL_STATIC_DRAW);
                destination.border_index_count = static_cast<GLsizei>(source.border_indices.size());
            }

            gl.bind_vertex_array(0);
            return destination.vertex_array != 0
                && destination.vertex_buffer != 0
                && destination.fill_index_buffer != 0;
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
                gl.delete_program(program);
                program = 0;
                return false;
            }

            const auto options = demo_options();
            for (std::size_t index = 0; index < meshes.size(); ++index)
            {
                const rounded_rect::RoundedRectMesh mesh =
                    rounded_rect::make_rounded_rect_mesh(options[index]);
                meshes[index].fill = fill_colors[index];
                meshes[index].border = border_colors[index];
                if (!upload_mesh(meshes[index], mesh))
                {
                    initialized = true;
                    release();
                    return false;
                }
            }

            initialized = true;
            return true;
        }

        void resize(int width, int height) noexcept
        {
            framebuffer_width = (std::max)(1, width);
            framebuffer_height = (std::max)(1, height);
        }

        void draw(const DrawMesh& mesh) noexcept
        {
            gl.bind_vertex_array(mesh.vertex_array);

            gl.uniform_4f(
                color_uniform,
                mesh.fill.r,
                mesh.fill.g,
                mesh.fill.b,
                mesh.fill.a);
            gl.bind_buffer(GL_ELEMENT_ARRAY_BUFFER, mesh.fill_index_buffer);
            gl.draw_elements(GL_TRIANGLES, mesh.fill_index_count, GL_UNSIGNED_INT, nullptr);

            if (mesh.border_index_count > 0)
            {
                gl.uniform_4f(
                    color_uniform,
                    mesh.border.r,
                    mesh.border.g,
                    mesh.border.b,
                    mesh.border.a);
                gl.bind_buffer(GL_ELEMENT_ARRAY_BUFFER, mesh.border_index_buffer);
                gl.draw_elements(GL_TRIANGLES, mesh.border_index_count, GL_UNSIGNED_INT, nullptr);
            }
        }

        void render() noexcept
        {
            if (!initialized)
                return;

            gl.viewport(0, 0, framebuffer_width, framebuffer_height);
            gl.clear_color(0.043f, 0.063f, 0.125f, 1.0f);
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

            for (const DrawMesh& mesh : meshes)
                draw(mesh);

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

    bool OpenGLRenderer::startup_animation_complete() const noexcept
    {
        return true;
    }
}
