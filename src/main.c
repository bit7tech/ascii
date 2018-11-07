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
int gFontWidth = 0;
int gFontHeight = 0;
int gImageWidth = 0;
int gImageHeight = 0;

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

GLuint loadFontTexture(const char* fileName)
{
    Data file = dataLoad(fileName);
    GLuint textureID = 0;

    if (file.bytes)
    {
        int width, height, bpp;
        u32* image = (u32*)stbi_load_from_memory(file.bytes, (int)file.size, &width, &height, &bpp, 4);
        dataUnload(file);
        gFontWidth = width / 16;
        gFontHeight = height / 16;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    *outImage = image;
    return texId;
}

//----------------------------------------------------------------------------------------------------------------------

void resizeDynamicTexture(GLuint id, int oldWidth, int oldHeight, int newWidth, int newHeight, u32** outImage)
{
    *outImage = K_REALLOC(*outImage, oldWidth * oldHeight * sizeof(u32), newWidth * newHeight * sizeof(u32));
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, *outImage);
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

void fillTexture(u32* image, GLuint id, u32 colour, int width, int height)
{
    int count = width * height;
    for (int i = 0; i < count; ++i) image[i] = colour;
    updateDynamicTexture(id, image, width, height);
}

void initOpenGL(int width, int height)
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

    glUseProgram(gProgram);

    // Set up textures
    gFontTex = loadFontTexture("font_10_16.png");
    int cw = width / gFontWidth;
    int ch = height / gFontHeight;
    gForeTex = createDynamicTexture(cw, ch, &gForeImage);
    gBackTex = createDynamicTexture(cw, ch, &gBackImage);
    gAsciiTex = createDynamicTexture(cw, ch, &gAsciiImage);
    gImageWidth = cw;
    gImageHeight = ch;

    GLuint loc;

    // Bind shader variable "fontTex" to texture unit 0, then bind our texture to texture unit 0.
    loc = glGetUniformLocation(gProgram, "fontTex");
    glUniform1i(loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFontTex);

    // Bind shader variable "foreTex" to texture unit 1, then bind our texture to texture unit 1.
    loc = glGetUniformLocation(gProgram, "foreTex");
    glUniform1i(loc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gForeTex);

    // Bind shader variable "fontTex" to texture unit 2, then bind our texture to texture unit 2.
    loc = glGetUniformLocation(gProgram, "backTex");
    glUniform1i(loc, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBackTex);

    // Bind shader variable "fontTex" to texture unit 3, then bind our texture to texture unit 3.
    loc = glGetUniformLocation(gProgram, "asciiTex");
    glUniform1i(loc, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gAsciiTex);

    // Let's do some example initialisation.
    fillTexture(gForeImage, gForeTex, 0xff0000ff, cw, ch);
    fillTexture(gBackImage, gBackTex, 0xff000000, cw, ch);
    fillTexture(gAsciiImage, gAsciiTex, 0x4c4c4c4c, cw, ch);
    for (int i = 0; i < cw; ++i) gAsciiImage[i] = i;
    updateDynamicTexture(gAsciiTex, gAsciiImage, cw, ch);

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

    int cw = 800 / 10;
    int ch = 600 / 16;
    destroyDynamicTexture(gForeImage, cw, ch, gForeTex);
    destroyDynamicTexture(gBackImage, cw, ch, gBackTex);
    destroyDynamicTexture(gAsciiImage, cw, ch, gAsciiTex);

    gOpenGLReady = NO;
}

//----------------------------------------------------------------------------------------------------------------------

void onSize(const Window* wnd, int width, int height)
{
    if (gOpenGLReady)
    {
        int cw = width / gFontWidth;
        int ch = height / gFontHeight;

        resizeDynamicTexture(gForeTex, gImageWidth, gImageHeight, cw, ch, &gForeImage);
        resizeDynamicTexture(gBackTex, gImageWidth, gImageHeight, cw, ch, &gBackImage);
        resizeDynamicTexture(gAsciiTex, gImageWidth, gImageHeight, cw, ch, &gAsciiImage);

        fillTexture(gForeImage, gForeTex, 0xff0000ff, cw, ch);
        fillTexture(gBackImage, gBackTex, 0xff000000, cw, ch);
        fillTexture(gAsciiImage, gAsciiTex, 0x4c4c4c4c, cw, ch);
        for (int i = 0; i < cw; ++i) gAsciiImage[i] = i;
        updateDynamicTexture(gAsciiTex, gAsciiImage, cw, ch);

        gImageWidth = cw;
        gImageHeight = ch;
    }
}

//----------------------------------------------------------------------------------------------------------------------

void onPaint(const Window* wnd)
{
    if (gOpenGLReady)
    {
        // Initialise drawing
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set uniforms
        GLint uFontRes = glGetUniformLocation(gProgram, "uFontRes");
        glProgramUniform2f(gProgram, uFontRes, (float)gFontWidth, (float)gFontHeight);
        GLint uResolution = glGetUniformLocation(gProgram, "uResolution");
        glProgramUniform2f(gProgram, uResolution, (float)wnd->bounds.size.cx, (float)wnd->bounds.size.cy);

        glUseProgram(gProgram);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int kmain(int argc, char** argv)
{
    const int width = 800;
    const int height = 600;

    Window mainWindow;
    windowInit(&mainWindow);
    mainWindow.title = stringMake("ASCII demo");
    mainWindow.bounds.size.cx = width;
    mainWindow.bounds.size.cy = height;
    mainWindow.resizeable = YES;
    mainWindow.opengl = YES;
    mainWindow.paintFunc = &onPaint;
    mainWindow.sizeFunc = &onSize;
    windowApply(&mainWindow);
    initOpenGL(width, height);

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

