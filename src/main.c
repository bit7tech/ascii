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
u32* gFontImage;
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

GLuint loadTexture(const char* fileName, u32** outImage)
{
    Data file = dataLoad(fileName);
    GLuint textureID = 0;

    if (file.bytes)
    {
        int width, height, bpp;
        *outImage = (u32*)stbi_load_from_memory(file.bytes, (int)file.size, &width, &height, &bpp, 4);
        dataUnload(file);

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 160, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, *outImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    return textureID;
}

//----------------------------------------------------------------------------------------------------------------------

#define GL_BUFFER_OFFSET(x) ((void *)(x))

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

    glGenBuffers(1, &gVb);
    glBindBuffer(GL_ARRAY_BUFFER, gVb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), GL_BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), GL_BUFFER_OFFSET(2 * sizeof(float)));

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vertexCode =
        "#version 330 core\n"
        "layout(location = 0) in vec2 v;"
        "layout(location = 1) in vec2 t;"
        "out vec2 texCoord;"
        "void main() {"
        "    vec2 vv = vec2(v.x, -v.y);"
        "    gl_Position.xy = vv;"
        "    gl_Position.zw = vec2(0.0, 1.0);"
        "    texCoord = t;"
        "}";

    const char* pixelCode =
        "#version 330 core\n"
        "in vec2 texCoord;"
        "out vec3 colour;"
        "uniform sampler2D fontText;"
        "void main() {"
        "    colour = texture(fontText, texCoord).rgb;"
        "}";

    compileShader(vertexShader, vertexCode);
    compileShader(fragmentShader, pixelCode);
    gProgram = createProgram(vertexShader, fragmentShader);

    glDetachShader(gProgram, vertexShader);
    glDetachShader(gProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up textures
    gFontTex = loadTexture("font_10_16.png", &gFontImage);

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
    stbi_image_free(gFontImage);

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
        GLint uFontTex = glGetUniformLocation(gProgram, "fontTex");
//         GLint uResolution = glGetUniformLocation(gProgram, "uResolution");
//         glProgramUniform2f(gProgram, uResolution, (float)wnd->bounds.size.cx, (float)wnd->bounds.size.cy);

        glUseProgram(gProgram);

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

