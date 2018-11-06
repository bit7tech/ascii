//----------------------------------------------------------------------------------------------------------------------
// ASCII demo
//----------------------------------------------------------------------------------------------------------------------

#define K_IMPLEMENTATION
#include <kore/kore.h>
#include <kore/kui.h>
#include <kore/kgl.h>

//----------------------------------------------------------------------------------------------------------------------

GLuint gVb;
GLuint gProgram;
bool gOpenGLReady = NO;

void compileShader(GLuint shader, const char* code)
{
    GLuint result = 0;
    GLuint infoLogLength = 0;

    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0)
    {
        String error = stringReserve(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, NULL, error);
        prn("%s", error);
        stringDone(&error);
        abort();
    }
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLuint result = 0;
    int logLength = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        String error = stringReserve(logLength);
        glGetProgramInfoLog(program, logLength, NULL, error);
        prn("%s", error);
        stringDone(&error);
        abort();
    }

    return program;
}

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

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vertexCode =
        "#version 330 core\n"
        "layout(location = 0) in vec3 v;"
        "void main() {"
        "    gl_Position.xyz = v;"
        "    gl_Position.w = 1.0;"
        "}";

    const char* pixelCode =
        "#version 330 core\n"
        "out vec3 colour;"
        "void main() {"
        "    colour = vec3(0, 1, 0);"
        "}";

    compileShader(vertexShader, vertexCode);
    compileShader(fragmentShader, pixelCode);
    gProgram = createProgram(vertexShader, fragmentShader);

    glDetachShader(gProgram, vertexShader);
    glDetachShader(gProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    gOpenGLReady = YES;
}

//----------------------------------------------------------------------------------------------------------------------

void doneOpenGL()
{
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &gVb);
    glDeleteProgram(gProgram);

    gOpenGLReady = NO;
}

//----------------------------------------------------------------------------------------------------------------------

void paint(const Window* wnd)
{
    if (gOpenGLReady)
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(gProgram);
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

