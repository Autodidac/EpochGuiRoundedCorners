#define splitter_hit_test(splitter_layout, pointer_position, hit_slop) \
    splitter_hit_test_with_capture( \
        (splitter_layout), \
        (pointer_position), \
        (hit_slop), \
        frame.pointer(input::PointerButton::left).pressed)

#define splitter_fraction_from_point(splitter_options, pointer_position) \
    splitter_fraction_from_visible_point( \
        (splitter_options), \
        (pointer_position))

#include "complete_demo_renderer.cpp"
