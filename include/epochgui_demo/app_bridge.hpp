#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*epoch_gui_demo_gl_proc)(void);
    typedef epoch_gui_demo_gl_proc (*epoch_gui_demo_proc_loader)(const char* name);

    struct epoch_gui_demo_renderer;

    [[nodiscard]] epoch_gui_demo_renderer* epoch_gui_demo_create(void);
    void epoch_gui_demo_destroy(epoch_gui_demo_renderer* renderer);

    [[nodiscard]] bool epoch_gui_demo_initialize(
        epoch_gui_demo_renderer* renderer,
        epoch_gui_demo_proc_loader loader);

    void epoch_gui_demo_resize(
        epoch_gui_demo_renderer* renderer,
        int width,
        int height);

    void epoch_gui_demo_render(epoch_gui_demo_renderer* renderer);

    [[nodiscard]] bool epoch_gui_demo_startup_animation_complete(
        const epoch_gui_demo_renderer* renderer);

#ifdef __cplusplus
}
#endif
