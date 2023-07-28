#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


int main() {
    Display *display;
    Window win;

    display = XOpenDisplay(NULL);
    assert(display != NULL);

    win = XCreateSimpleWindow(display,
            RootWindow(display, 0),
            0, 0,
            300, 150,
            0,
            0x000000,
            0xFFDD00);

    XMapWindow(display, win);
    XFlush(display);

    getchar();

   XUnmapWindow(display, win);
   XDestroyWindow(display, win);
   XCloseDisplay(display);
}
