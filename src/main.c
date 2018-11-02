//----------------------------------------------------------------------------------------------------------------------
// ASCII demo
//----------------------------------------------------------------------------------------------------------------------

#define K_IMPLEMENTATION
#include <kore/kore.h>
#include <kore/kui.h>
#include <kore/kgl.h>

//----------------------------------------------------------------------------------------------------------------------

void paint(const Window* wnd)
{
    glClearColor(1.0f, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------

int kmain(int argc, char** argv)
{
    glInit();
    Window mainWindow;
    windowInit(&mainWindow);
    mainWindow.title = stringMake("ASCII demo");
    mainWindow.bounds.size.cx = 800;
    mainWindow.bounds.size.cy = 600;
    mainWindow.resizeable = YES;
    mainWindow.opengl = YES;
    mainWindow.paintFunc = &paint;
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
                run = NO;
                break;

            case K_EVENT_INPUT:
                if (ev.input.down && ev.input.alt && (ev.input.key == VK_RETURN))
                {
                    mainWindow.fullscreen = !mainWindow.fullscreen;
                }

                if (ev.input.down && ev.input.key == VK_ESCAPE)
                {
                    windowDone(&mainWindow);
                }
                break;

            default:
                break;
            }

            windowApply(&mainWindow);
        }
    }
    return 0;
}

