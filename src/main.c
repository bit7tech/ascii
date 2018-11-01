//----------------------------------------------------------------------------------------------------------------------
// ASCII demo
//----------------------------------------------------------------------------------------------------------------------

#define K_IMPLEMENTATION
#include <kore/kore.h>
#include <kore/kui.h>

int kmain(int argc, char** argv)
{
    Window mainWindow;
    windowInit(&mainWindow);
    mainWindow.title = stringMake("ASCII demo");
    mainWindow.bounds.size.cx = 800;
    mainWindow.bounds.size.cy = 600;
    windowApply(&mainWindow);

    WindowEvent ev;
    bool run = YES;
    while (run)
    {
        while (windowPoll(&ev))
        {
            windowUpdate(&mainWindow);

            switch (ev.type)
            {
            case K_EVENT_QUIT:
                if (ev.handle == mainWindow.handle)
                {
                    run = NO;
                }
                break;

            default:
                break;
            }

            windowApply(&mainWindow);
        }
    }

    windowDone(&mainWindow);
    return 0;
}

