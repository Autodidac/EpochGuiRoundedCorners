module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

import epoch.gui.rounded_rect;

namespace
{
    namespace gui = epochengine::gui_lib;
    namespace rounded = epochengine::gui_lib::rounded_rect;

    struct DemoCase
    {
        std::string_view title{};
        rounded::RoundedRectOptions options{};
        std::string_view fill{};
        std::string_view border{};
    };

    [[nodiscard]] bool indices_valid(
        std::span<const std::uint32_t> indices,
        std::size_t vertex_count) noexcept
    {
        return indices.size() % 3U == 0U
            && std::ranges::all_of(indices, [vertex_count](std::uint32_t index) noexcept
            {
                return index < vertex_count;
            });
    }

    [[nodiscard]] bool mesh_valid(const rounded::RoundedRectMesh& mesh) noexcept
    {
        if (!mesh.valid || mesh.outer_contour().empty())
            return false;
        if (!indices_valid(mesh.fill_indices, mesh.vertices.size()))
            return false;
        if (!indices_valid(mesh.border_indices, mesh.vertices.size()))
            return false;

        const float width = mesh.bounds.size.x;
        const float height = mesh.bounds.size.y;
        constexpr float epsilon = 0.01f;
        return mesh.radii.top_left + mesh.radii.top_right <= width + epsilon
            && mesh.radii.bottom_left + mesh.radii.bottom_right <= width + epsilon
            && mesh.radii.top_left + mesh.radii.bottom_left <= height + epsilon
            && mesh.radii.top_right + mesh.radii.bottom_right <= height + epsilon
            && std::ranges::all_of(mesh.vertices, [](gui::Vec2 point) noexcept
            {
                return std::isfinite(point.x) && std::isfinite(point.y);
            });
    }

    [[nodiscard]] std::string svg_points(std::span<const gui::Vec2> points)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2);
        for (const gui::Vec2 point : points)
            stream << point.x << ',' << point.y << ' ';
        return stream.str();
    }

    void write_mesh(
        std::ostream& output,
        const rounded::RoundedRectMesh& mesh,
        std::string_view fill,
        std::string_view border)
    {
        if (mesh.border_width > 0.0f)
        {
            output << "  <polygon points=\"" << svg_points(mesh.outer_contour())
                   << "\" fill=\"" << border << "\"/>\n";
            if (!mesh.inner_contour().empty())
            {
                output << "  <polygon points=\"" << svg_points(mesh.inner_contour())
                       << "\" fill=\"" << fill << "\"/>\n";
            }
            return;
        }

        output << "  <polygon points=\"" << svg_points(mesh.outer_contour())
               << "\" fill=\"" << fill << "\"/>\n";
    }
}

int main(int argc, char** argv)
{
    const std::filesystem::path output_path = argc > 1
        ? std::filesystem::path{ argv[1] }
        : std::filesystem::path{ "epochgui_rounded_corners_demo.svg" };

    if (output_path.has_parent_path())
        std::filesystem::create_directories(output_path.parent_path());

    constexpr std::array cases{
        DemoCase{
            .title = "Uniform corners",
            .options = {
                .bounds = { { 70.0f, 110.0f }, { 360.0f, 120.0f } },
                .radii = { 24.0f, 24.0f, 24.0f, 24.0f },
                .border_width = 3.0f,
                .segments_per_corner = 12
            },
            .fill = "#182235",
            .border = "#65a7ff"
        },
        DemoCase{
            .title = "Pill control",
            .options = {
                .bounds = { { 510.0f, 126.0f }, { 360.0f, 88.0f } },
                .radii = { 44.0f, 44.0f, 44.0f, 44.0f },
                .border_width = 2.0f,
                .segments_per_corner = 16
            },
            .fill = "#243248",
            .border = "#7dd3fc"
        },
        DemoCase{
            .title = "Asymmetric corners",
            .options = {
                .bounds = { { 70.0f, 340.0f }, { 360.0f, 150.0f } },
                .radii = { 8.0f, 44.0f, 12.0f, 36.0f },
                .border_width = 4.0f,
                .segments_per_corner = 12
            },
            .fill = "#231c32",
            .border = "#c084fc"
        },
        DemoCase{
            .title = "Oversized radii clamp safely",
            .options = {
                .bounds = { { 510.0f, 350.0f }, { 360.0f, 130.0f } },
                .radii = { 160.0f, 120.0f, 180.0f, 140.0f },
                .border_width = 5.0f,
                .segments_per_corner = 16
            },
            .fill = "#1b302b",
            .border = "#6ee7b7"
        }
    };

    std::array<rounded::RoundedRectMesh, cases.size()> meshes{};
    for (std::size_t index = 0; index < cases.size(); ++index)
    {
        meshes[index] = rounded::make_rounded_rect_mesh(cases[index].options);
        if (!mesh_valid(meshes[index]))
            return 1;
    }

    std::ofstream output{ output_path, std::ios::binary | std::ios::trunc };
    if (!output)
        return 2;

    output << R"(<svg xmlns="http://www.w3.org/2000/svg" width="940" height="590" viewBox="0 0 940 590">
  <rect width="940" height="590" fill="#0b1020"/>
  <text x="54" y="52" fill="#f8fafc" font-family="Segoe UI, sans-serif" font-size="28" font-weight="700">EpochGUI Rounded Corners</text>
  <text x="54" y="79" fill="#94a3b8" font-family="Segoe UI, sans-serif" font-size="15">Backend-neutral tessellated geometry generated from EpochGUI Rect and Vec2.</text>
)";

    for (std::size_t index = 0; index < cases.size(); ++index)
    {
        write_mesh(output, meshes[index], cases[index].fill, cases[index].border);
        const gui::Rect bounds = meshes[index].bounds;
        output << "  <text x=\"" << bounds.position.x + 22.0f
               << "\" y=\"" << bounds.position.y + 42.0f
               << "\" fill=\"#f8fafc\" font-family=\"Segoe UI, sans-serif\" font-size=\"19\" font-weight=\"600\">"
               << cases[index].title << "</text>\n";
        output << "  <text x=\"" << bounds.position.x + 22.0f
               << "\" y=\"" << bounds.position.y + 72.0f
               << "\" fill=\"#aab7cb\" font-family=\"Consolas, monospace\" font-size=\"13\">"
               << meshes[index].outer_count << " contour vertices | "
               << meshes[index].fill_indices.size() / 3U << " fill triangles"
               << "</text>\n";
    }

    output << R"(  <text x="54" y="550" fill="#64748b" font-family="Segoe UI, sans-serif" font-size="13">The same mesh can be consumed by OpenGL, Vulkan, DirectX, SDL, SFML, Raylib, or the software renderer.</text>
</svg>
)";

    output.close();
    return output ? 0 : 3;
}
