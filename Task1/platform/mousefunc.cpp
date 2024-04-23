#ifdef _WIN32

#include <Windows.h>

void getMouseCoordinates(int *x, int *y) {
    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
        *x = cursorPos.x;
        *y = cursorPos.y;
    } else {
        *x = *y = -1; // Return -1 for coordinates if failed to get cursor position
    }
}

#else 

#include <X11/Xlib.h>
#include <unistd.h>
void getMouseCoordinates(int *x, int *y) {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        *x = *y = -1; 
        return;
    }

    Window root_window = DefaultRootWindow(display);

    XEvent event;
    XQueryPointer(display, root_window, &event.xbutton.root, &event.xbutton.window,
                  &event.xbutton.x_root, &event.xbutton.y_root,
                  &event.xbutton.x_root, &event.xbutton.y, //fixed crash
                  &event.xbutton.state);

    *x = event.xbutton.x_root;
    *y = event.xbutton.y_root;

    XCloseDisplay(display);
}

#endif

bool check_movement(){
    int x1,x2,y1,y2;
    getMouseCoordinates(&x1,&y1);
    #ifdef _WIN32
    Sleep(60000);//in windows this as i concerned this in miliseconds
    #else
    sleep(60);
    #endif
    getMouseCoordinates(&x2,&y2);
    if(x1==x2 && y1==y2){
        return false;
    }else{
        return true;
    }
}
