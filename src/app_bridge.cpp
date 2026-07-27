#include "epochgui_demo/app_bridge.hpp"

#include <new>

import epoch.gui.demo.opengl;

struct epoch_gui_demo_renderer
{
    epochengine::gui_demo::OpenGLRenderer renderer{};
};

epoch_gui_demo_renderer* epoch_gui_demo_create(void)
{
    return new (std::nothrow) epoch_gui_demo_renderer{};
}

void epoch_gui_demo_destroy(epoch_gui_demo_renderer* renderer)
{
    delete renderer;
}

bool epoch_gui_demo_initialize(
    epoch_gui_demo_renderer* renderer,
    epoch_gui_demo_proc_loader loader)
{
    if (!renderer || !loader)
        return false;

    return renderer->renderer.initialize(
        reinterpret_cast<epochengine::gui_demo::OpenGLProcLoader>(loader));
}

void epoch_gui_demo_resize(
    epoch_gui_demo_renderer* renderer,
    int width,
    int height)
{
    if (renderer)
        renderer->renderer.resize(width, height);
}

void epoch_gui_demo_render(epoch_gui_demo_renderer* renderer)
{
    if (renderer)
        renderer->renderer.render();
}

bool epoch_gui_demo_startup_animation_complete(
    const epoch_gui_demo_renderer* renderer)
{
    return !renderer || renderer->renderer.startup_animation_complete();
}
