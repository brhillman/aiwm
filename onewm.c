#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>

typedef struct {
    unsigned int mod;
    const char *key;
    void (*func)(Display *, Window, XEvent *);
} Binding;

static void focus_top(Display *d, Window w) {
    XSetInputFocus(d, w, RevertToPointerRoot, CurrentTime);
}

static void cycle_up(Display *d, Window r, XEvent *e) {
    XCirculateSubwindowsUp(d, r);
    focus_top(d, e->xkey.window);
}

static void cycle_down(Display *d, Window r, XEvent *e) {
    XCirculateSubwindowsDown(d, r);
    focus_top(d, e->xkey.window);
}

static void kill_client(Display *d, Window r, XEvent *e) {
    (void)r;
    XKillClient(d, e->xkey.subwindow);
}

static void run_dmenu(Display *d, Window r, XEvent *e) {
    (void)d; (void)r; (void)e;
    system("dmenu_run &");
}

#define CLEANMASK(m) ((m) & ~(LockMask))

static Binding bindings[] = {
    { Mod4Mask,             "Tab",   cycle_up   },
    { Mod4Mask | ShiftMask, "Tab",   cycle_down },
    { Mod4Mask,             "q",     kill_client },
    { Mod4Mask,             "space", run_dmenu },
};

int main(void) {
    Display *d = XOpenDisplay(0);
    Window r = DefaultRootWindow(d);
    XEvent e;
    XWindowAttributes wa;
    Window trans;

    for (unsigned int i = 0; i < sizeof(bindings)/sizeof(bindings[0]); i++) {
        KeyCode code = XKeysymToKeycode(d, XStringToKeysym(bindings[i].key));
        XGrabKey(d, code, bindings[i].mod, r, True, GrabModeAsync, GrabModeAsync);
    }

    XSelectInput(d, r, SubstructureRedirectMask);

    while (!XNextEvent(d, &e)) {
        if (e.type == ConfigureRequest) {
            XConfigureRequestEvent *c = &e.xconfigurerequest;
            XWindowChanges wc = {
                .x = c->x,
                .y = c->y,
                .width = c->width,
                .height = c->height,
                .border_width = c->border_width,
                .sibling = c->above,
                .stack_mode = c->detail
            };
            XConfigureWindow(d, c->window, c->value_mask, &wc);
        }

        if (e.type == MapRequest) {
            if (!XGetTransientForHint(d, e.xmaprequest.window, &trans)) {
                XGetWindowAttributes(d, r, &wa);
                XMoveResizeWindow(d, e.xmaprequest.window, 0, 0, wa.width, wa.height);
                XMapRaised(d, e.xmaprequest.window);
                XSetInputFocus(d, e.xmaprequest.window, RevertToPointerRoot, CurrentTime);
            } else {
                XMapRaised(d, e.xmaprequest.window);
            }
        }

        if (e.type == KeyPress) {
            for (unsigned int i = 0; i < sizeof(bindings)/sizeof(bindings[0]); i++) {
                KeyCode code = XKeysymToKeycode(d, XStringToKeysym(bindings[i].key));
                if (e.xkey.keycode == code &&
                    CLEANMASK(e.xkey.state) == CLEANMASK(bindings[i].mod)) {
                    bindings[i].func(d, r, &e);
                }
            }
        }
    }
}
