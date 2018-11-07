//----------------------------------------------------------------------------------------------------------------------
// ASCII demo
//----------------------------------------------------------------------------------------------------------------------

#define K_IMPLEMENTATION
#include <kore/kore.h>
#include <kore/kui.h>
#include <kore/kgl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//----------------------------------------------------------------------------------------------------------------------

GLuint gVb;
GLuint gProgram;
GLuint gFontTex;
GLuint gForeTex;
GLuint gBackTex;
GLuint gAsciiTex;
u32* gForeImage;
u32* gBackImage;
u32* gAsciiImage;
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

//----------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------

GLuint loadTexture(const char* fileName)
{
    Data file = dataLoad(fileName);
    GLuint textureID = 0;

    if (file.bytes)
    {
        int width, height, bpp;
        u32* image = (u32*)stbi_load_from_memory(file.bytes, (int)file.size, &width, &height, &bpp, 4);
        dataUnload(file);

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 160, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        stbi_image_free(image);
    }
    return textureID;
}

//----------------------------------------------------------------------------------------------------------------------

GLuint createDynamicTexture(int width, int height, u32** outImage)
{
    u32* image = K_ALLOC_CLEAR(width * height * sizeof(u32));
    for (int i = 0; i < width; ++i) image[i] = 0xffff00ff;

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, image);

    *outImage = image;
    return texId;
}

//----------------------------------------------------------------------------------------------------------------------

void updateDynamicTexture(GLuint texId, u32* image, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

//----------------------------------------------------------------------------------------------------------------------

void destroyDynamicTexture(u32* image, int width, int height, GLuint id)
{
    glDeleteTextures(1, &id);
    K_FREE(image, width * height * sizeof(u32));
}

//----------------------------------------------------------------------------------------------------------------------

#define GL_BUFFER_OFFSET(x) ((void *)(x))

void glMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        pr("ERROR: ");
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        pr("DEPRECATED BEHAVIOUR: ");
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        pr("UNDEFINED BEHAVIOUR: ");
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        pr("PORTABILITY: ");
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        pr("PERFORMANCE: ");
        break;

    case GL_DEBUG_TYPE_OTHER:
        pr("OTHER: ");
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:    pr("[HIGH] ");          break;
    case GL_DEBUG_SEVERITY_MEDIUM:  pr("[MED] ");           break;
    case GL_DEBUG_SEVERITY_LOW:     pr("[LOW] ");           break;
    }

    prn("%s", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH) K_BREAK();
}

void initOpenGL()
{
    static const GLfloat buffer[] = {
        //  X       Y       TX      TY
        // Top left
            -1.0f,  -1.0f,  0.0f,   0.0f,
            1.0f,   -1.0f,  1.0f,   0.0f,
            -1.0f,  1.0f,   0.0f,   1.0f,

        // Bottom right
            -1.0f,  1.0f,   0.0f,   1.0f,
            1.0f,   -1.0f,  1.0f,   0.0f,
            1.0f,   1.0f,   1.0f,   1.0f,
    };

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glMessage, 0);

    glGenBuffers(1, &gVb);
    glBindBuffer(GL_ARRAY_BUFFER, gVb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), GL_BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), GL_BUFFER_OFFSET(2 * sizeof(float)));

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    Data vertexCode = dataLoad("ascii.vs");
    Data pixelCode = dataLoad("ascii.fs");

    compileShader(vertexShader, vertexCode.bytes);
    compileShader(fragmentShader, pixelCode.bytes);
    gProgram = createProgram(vertexShader, fragmentShader);

    glDetachShader(gProgram, vertexShader);
    glDetachShader(gProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    dataUnload(vertexCode);
    dataUnload(pixelCode);

    // Set up textures
    gFontTex = loadTexture("font_10_16.png");
    gForeTex = createDynamicTexture(800, 600, &gForeImage);
    //glBindTexture(GL_TEXTURE_2D, gFontTex);

    gOpenGLReady = YES;
}

//----------------------------------------------------------------------------------------------------------------------

void doneOpenGL()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, &gVb);
    glDeleteProgram(gProgram);
    glDeleteTextures(1, &gFontTex);
    destroyDynamicTexture(gForeImage, 800, 600, gForeTex);

    gOpenGLReady = NO;
}

//----------------------------------------------------------------------------------------------------------------------

void paint(const Window* wnd)
{
    if (gOpenGLReady)
    {
        // Initialise drawing
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set uniforms
        //GLint uFontTex = glGetUniformLocation(gProgram, "fontTex");
//         GLint uResolution = glGetUniformLocation(gProgram, "uResolution");
//         glProgramUniform2f(gProgram, uResolution, (float)wnd->bounds.size.cx, (float)wnd->bounds.size.cy);

        glUseProgram(gProgram);

        for (int i = 0; i < 800; ++i) gForeImage[i] = 0xff00ff00;
        updateDynamicTexture(gForeTex, gForeImage, 800, 600);

        glDrawArrays(GL_TRIANGLES, 0, 6);
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

