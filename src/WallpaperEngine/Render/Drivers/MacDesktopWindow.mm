// macOS port: turns the GLFW window into a desktop wallpaper window (behind icons, click-through, on every Space)
// and reads the global cursor so the scene can react to the mouse although the window never gets events.
#import <Cocoa/Cocoa.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

extern "C" void wpMakeDesktopWindow (GLFWwindow* window) {
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    NSWindow* win = glfwGetCocoaWindow (window);
    if (win == nil) return;
    win.level = kCGDesktopWindowLevel + 1;
    win.ignoresMouseEvents = YES;
    win.hasShadow = NO;
    win.opaque = YES;
    win.backgroundColor = NSColor.blackColor;
    win.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces | NSWindowCollectionBehaviorStationary
        | NSWindowCollectionBehaviorIgnoresCycle | NSWindowCollectionBehaviorFullScreenAuxiliary;
    [win orderBack:nil];
}

// Cursor in global screen points, origin top left of the primary display (same space as glfwGetWindowPos).
extern "C" void wpGlobalCursor (double* x, double* y) {
    NSPoint p = [NSEvent mouseLocation];
    CGFloat primaryHeight = NSScreen.screens.firstObject.frame.size.height;
    *x = p.x;
    *y = primaryHeight - p.y;
}

// The host launches us and must never be outlived: exit as soon as the parent is gone (it was SIGKILLed or crashed).
#include <dispatch/dispatch.h>
#include <unistd.h>
__attribute__ ((constructor)) static void wpWatchParent () {
    const pid_t parent = getppid ();
    dispatch_source_t timer = dispatch_source_create (DISPATCH_SOURCE_TYPE_TIMER, 0, 0,
        dispatch_get_global_queue (QOS_CLASS_UTILITY, 0));
    dispatch_source_set_timer (timer, DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC, NSEC_PER_SEC);
    dispatch_source_set_event_handler (timer, ^{ if (getppid () != parent) _exit (0); });
    dispatch_resume (timer);
}

// Visible = some part of the window can be seen (not fully covered, display awake, not on a hidden Space).
extern "C" bool wpWindowVisible (GLFWwindow* window) {
    NSWindow* win = glfwGetCocoaWindow (window);
    return win == nil || (win.occlusionState & NSWindowOcclusionStateVisible) != 0;
}
