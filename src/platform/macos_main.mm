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
}

@interface EpochOpenGLView : NSOpenGLView
{
    epoch_gui_demo_renderer* renderer_;
}
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

- (void)drawRect:(NSRect)dirtyRect
{
    (void)dirtyRect;
    [[self openGLContext] makeCurrentContext];
    epoch_gui_demo_render(renderer_);
    [[self openGLContext] flushBuffer];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent*)event
{
    if ([event keyCode] == 53)
        [[self window] close];
    else
        [super keyDown:event];
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
        styleMask:(NSWindowStyleMaskTitled
            | NSWindowStyleMaskClosable
            | NSWindowStyleMaskMiniaturizable
            | NSWindowStyleMaskResizable)
        backing:NSBackingStoreBuffered
        defer:NO];

    [window_ setTitle:@"EpochGui Demo - Core and Optional Features"];
    [window_ setDelegate:self];
    [window_ center];

    EpochOpenGLView* view = [[EpochOpenGLView alloc] initWithFrame:frame];
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [window_ setContentView:view];
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
