//----------------------------------------------------------------------------------------------------------------------
// ASCII demo
//----------------------------------------------------------------------------------------------------------------------

#define K_IMPLEMENTATION
#include <kore/kore.h>
#include <kore/kui.h>
#include <kore/kgl.h>

//----------------------------------------------------------------------------------------------------------------------

GLuint gVb;
bool gOpenGLReady = NO;

void initOpenGL()
{
    static const GLfloat buffer[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        0.0f, 1.0f,
    };

    glGenBuffers(1, &gVb);
    glBindBuffer(GL_ARRAY_BUFFER, gVb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    gOpenGLReady = YES;
}

//----------------------------------------------------------------------------------------------------------------------

void doneOpenGL()
{
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &gVb);

    gOpenGLReady = NO;
}

//----------------------------------------------------------------------------------------------------------------------

void paint(const Window* wnd)
{
    if (gOpenGLReady)
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int kmain(int argc, char** argv)
{
    Window mainWindow;
    windowInit(&mainWindow);
    mainWindow.title = stringMake("ASCII demo");
    mainWindow.bounds.size.cx = 800;
    mainWindow.bounds.size.cy = 600;
    mainWindow.resizeable = YES;
    mainWindow.opengl = YES;
    mainWindow.paintFunc = &paint;
    windowApply(&mainWindow);
    initOpenGL();

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

    doneOpenGL();
    return 0;
}

