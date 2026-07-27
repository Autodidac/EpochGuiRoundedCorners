#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include "epochgui_demo/app_bridge.hpp"

#include <dlfcn.h>

namespace
{
    [[nodiscard]] epoch_gui_demo_gl_proc load_opengl_proc(const char* name)
    {
        return reinterpret_cast<epoch_gui_demo_gl_proc>(dlsym(RTLD_DEFAULT, name));
    }

    [[nodiscard]] int key_from_code(unsigned short code) noexcept
    {
        switch (code)
        {
        case 53: return EPOCH_GUI_DEMO_KEY_ESCAPE;
        case 36: return EPOCH_GUI_DEMO_KEY_ENTER;
        case 48: return EPOCH_GUI_DEMO_KEY_TAB;
        case 49: return EPOCH_GUI_DEMO_KEY_SPACE;
        case 51: return EPOCH_GUI_DEMO_KEY_BACKSPACE;
        case 117: return EPOCH_GUI_DEMO_KEY_DELETE;
        case 123: return EPOCH_GUI_DEMO_KEY_LEFT;
        case 124: return EPOCH_GUI_DEMO_KEY_RIGHT;
        case 126: return EPOCH_GUI_DEMO_KEY_UP;
        case 125: return EPOCH_GUI_DEMO_KEY_DOWN;
        case 115: return EPOCH_GUI_DEMO_KEY_HOME;
        case 119: return EPOCH_GUI_DEMO_KEY_END;
        case 56:
        case 60:
            return EPOCH_GUI_DEMO_KEY_SHIFT;
        case 59:
        case 62:
            return EPOCH_GUI_DEMO_KEY_CONTROL;
        case 58:
        case 61:
            return EPOCH_GUI_DEMO_KEY_ALT;
        case 54:
        case 55:
            return EPOCH_GUI_DEMO_KEY_SUPER;
        default:
            return -1;
        }
    }
}

@interface EpochOpenGLView : NSOpenGLView
{
    epoch_gui_demo_renderer* renderer_;
}
- (void)renderAndApplyCommands;
@end

@implementation EpochOpenGLView

- (instancetype)initWithFrame:(NSRect)frame
{
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize,
        24,
        NSOpenGLPFAAlphaSize,
        8,
        0
    };

    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    self = [super initWithFrame:frame pixelFormat:format];
    [format release];
    return self;
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    [[self openGLContext] makeCurrentContext];

    GLint swap_interval = 1;
    [[self openGLContext] setValues:&swap_interval forParameter:NSOpenGLCPSwapInterval];

    renderer_ = epoch_gui_demo_create();
    if (!renderer_ || !epoch_gui_demo_initialize(renderer_, &load_opengl_proc))
    {
        [NSApp terminate:nil];
        return;
    }

    [[self window] setAcceptsMouseMovedEvents:YES];
    [self reshape];
}

- (void)reshape
{
    [super reshape];
    [[self openGLContext] makeCurrentContext];

    const NSRect backing = [self convertRectToBacking:[self bounds]];
    epoch_gui_demo_resize(
        renderer_,
        static_cast<int>(backing.size.width),
        static_cast<int>(backing.size.height));
    [self setNeedsDisplay:YES];
}

- (NSPoint)rendererPointForEvent:(NSEvent*)event
{
    const NSPoint local = [self convertPoint:[event locationInWindow] fromView:nil];
    const NSPoint backing = [self convertPointToBacking:local];
    const NSRect backingBounds = [self convertRectToBacking:[self bounds]];
    return NSMakePoint(backing.x, backingBounds.size.height - backing.y);
}

- (void)feedPointer:(NSEvent*)event
{
    if (!renderer_)
        return;
    const NSPoint point = [self rendererPointForEvent:event];
    epoch_gui_demo_pointer_move(
        renderer_,
        static_cast<float>(point.x),
        static_cast<float>(point.y));
}

- (void)feedModifiers:(NSEvent*)event
{
    const NSEventModifierFlags flags = [event modifierFlags];
    epoch_gui_demo_modifiers(
        renderer_,
        (flags & NSEventModifierFlagShift) != 0,
        (flags & NSEventModifierFlagControl) != 0,
        (flags & NSEventModifierFlagOption) != 0,
        (flags & NSEventModifierFlagCommand) != 0);
}

- (void)renderAndApplyCommands
{
    if (!renderer_)
        return;

    [[self openGLContext] makeCurrentContext];
    epoch_gui_demo_render(renderer_);
    [[self openGLContext] flushBuffer];

    switch (epoch_gui_demo_take_window_command(renderer_))
    {
    case EPOCH_GUI_DEMO_COMMAND_MINIMIZE:
        [[self window] miniaturize:nil];
        break;
    case EPOCH_GUI_DEMO_COMMAND_TOGGLE_MAXIMIZE:
        [[self window] zoom:nil];
        break;
    case EPOCH_GUI_DEMO_COMMAND_CLOSE:
        [[self window] close];
        break;
    default:
        break;
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    (void)dirtyRect;
    [self renderAndApplyCommands];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseMoved:(NSEvent*)event
{
    [self feedPointer:event];
    [self renderAndApplyCommands];
}

- (void)mouseDragged:(NSEvent*)event
{
    [self feedPointer:event];
    [self renderAndApplyCommands];
}

- (void)rightMouseDragged:(NSEvent*)event
{
    [self feedPointer:event];
    [self renderAndApplyCommands];
}

- (void)otherMouseDragged:(NSEvent*)event
{
    [self feedPointer:event];
    [self renderAndApplyCommands];
}

- (void)mouseDown:(NSEvent*)event
{
    [self feedPointer:event];
    const NSPoint point = [self rendererPointForEvent:event];
    const int region = epoch_gui_demo_window_hit_test(
        renderer_,
        static_cast<float>(point.x),
        static_cast<float>(point.y));

    if (region == EPOCH_GUI_DEMO_WINDOW_CAPTION)
    {
        [[self window] performWindowDragWithEvent:event];
        [self renderAndApplyCommands];
        return;
    }

    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_LEFT, true);
    [self renderAndApplyCommands];
}

- (void)mouseUp:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_LEFT, false);
    [self renderAndApplyCommands];
}

- (void)rightMouseDown:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_RIGHT, true);
    [self renderAndApplyCommands];
}

- (void)rightMouseUp:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_RIGHT, false);
    [self renderAndApplyCommands];
}

- (void)otherMouseDown:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_MIDDLE, true);
    [self renderAndApplyCommands];
}

- (void)otherMouseUp:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_pointer_button(renderer_, EPOCH_GUI_DEMO_POINTER_MIDDLE, false);
    [self renderAndApplyCommands];
}

- (void)scrollWheel:(NSEvent*)event
{
    [self feedPointer:event];
    epoch_gui_demo_wheel(
        renderer_,
        static_cast<float>([event scrollingDeltaX]),
        static_cast<float>([event scrollingDeltaY]));
    [self renderAndApplyCommands];
}

- (void)keyDown:(NSEvent*)event
{
    const int key = key_from_code([event keyCode]);
    if (key >= 0)
    {
        epoch_gui_demo_key_event(renderer_, key, true, [event isARepeat]);
        [self feedModifiers:event];
        [self renderAndApplyCommands];
    }
    else
    {
        [super keyDown:event];
    }
}

- (void)keyUp:(NSEvent*)event
{
    const int key = key_from_code([event keyCode]);
    if (key >= 0)
    {
        epoch_gui_demo_key_event(renderer_, key, false, false);
        [self feedModifiers:event];
        [self renderAndApplyCommands];
    }
    else
    {
        [super keyUp:event];
    }
}

- (void)flagsChanged:(NSEvent*)event
{
    [self feedModifiers:event];
    [self renderAndApplyCommands];
}

- (void)dealloc
{
    [[self openGLContext] makeCurrentContext];
    epoch_gui_demo_destroy(renderer_);
    renderer_ = nullptr;
    [super dealloc];
}

@end

@interface EpochApplicationDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
    NSWindow* window_;
}
@end

@implementation EpochApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    (void)notification;

    const NSRect frame = NSMakeRect(0.0, 0.0, 1280.0, 820.0);
    window_ = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskBorderless
            | NSWindowStyleMaskResizable
            | NSWindowStyleMaskMiniaturizable)
        backing:NSBackingStoreBuffered
        defer:NO];

    [window_ setTitle:@"EpochGui Live Demo"];
    [window_ setDelegate:self];
    [window_ setOpaque:YES];
    [window_ setHasShadow:YES];
    [window_ setMovableByWindowBackground:NO];
    [window_ center];

    EpochOpenGLView* view = [[EpochOpenGLView alloc] initWithFrame:frame];
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [window_ setContentView:view];
    [window_ makeFirstResponder:view];
    [view release];

    [window_ makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    (void)sender;
    return YES;
}

- (void)dealloc
{
    [window_ release];
    [super dealloc];
}

@end

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    @autoreleasepool
    {
        NSApplication* application = [NSApplication sharedApplication];
        [application setActivationPolicy:NSApplicationActivationPolicyRegular];

        EpochApplicationDelegate* delegate = [[EpochApplicationDelegate alloc] init];
        [application setDelegate:delegate];
        [application run];
        [delegate release];
    }
    return 0;
}
