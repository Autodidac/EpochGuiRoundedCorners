#define splitter_hit_test(splitter_layout, pointer_position, hit_slop) \
    splitter_hit_test_with_capture( \
        (splitter_layout), \
        (pointer_position), \
        (hit_slop), \
        frame.pointer(input::PointerButton::left).pressed)

#include "complete_demo_renderer.cpp"
