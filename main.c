#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include "snake.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ========================================================
// Static data
// ========================================================

// Static data for drawing background texture
// ========================================================
float data_vertex_array[] = {
    // positions     |      colors     |     texture coords
    // =====================================================
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};

unsigned int data_indice_array[] = {
    0, 1, 3,    // first triangle
    1, 2, 3     // second triangle
};

// Utils -- common
// ========================================================
#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

static void errorLog(
        const char *file,
        int line, 
        const char *fmt, 
        ...) {
    va_list ap;
    va_start(ap, fmt);
    if (errno)
        fprintf(stderr, "ERROR [%s %d]: (%s)\n", file, line, strerror(errno));
    else
        fprintf(stderr, "ERROR [%s %d]:\n", file, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
#define ERROR_LOG(...) errorLog(__FILE__, __LINE__, __VA_ARGS__)

static void die(
        const char *file,
        int line, 
        const char *fmt, 
        ...) {
    va_list ap;
    va_start(ap, fmt);
    if (errno)
        fprintf(stderr, "Error [%s %d]: (%s)\n", file, line, strerror(errno));
    else
        fprintf(stderr, "Error [%s %d]:\n", file, line);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(1);
}
#define DIE(...) die(__FILE__, __LINE__, __VA_ARGS__)

static char *readFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) 
        DIE("Could not read file: %s", filename);
    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf)
        DIE("Memory error");
    fread(buf, 1, len, file);
    buf[len] = '\0';
    return buf;
}

// stb_image wrappers
// ========================================================
static unsigned char *loadImageStb(
        const char *filename, 
        int *width, 
        int *height) {

    int n;
    unsigned char *data = stbi_load(filename, 
            width, height, &n, 0);
    if (!data)
        DIE("Could not load image: %s", filename);
    return data;
} 

static void freeImageStb(unsigned char *im) {
    stbi_image_free(im);
}

// ========================================================
// Graphics framework
// ========================================================

#define TEX_SHADER_SRC "./shaders/tex"

// Utils -- graphics
// ========================================================
static void convertScreenCoodsToWorldCoods(
        double xin,
        double yin,
        double *xout,
        double *yout) {
    double model[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    double projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    int view[16];
    glGetIntegerv(GL_VIEWPORT, view);
    double z;
    gluUnProject(xin, yin, 0, model, projection, view, xout, yout, &z);
} 

// Graphics framework common
// ========================================================
void initGraphicsFramework() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void initGladLib() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        DIE("Failed to initialize GLAD");
}

void freeGraphicsFramework() {
    glfwTerminate();
}

// Shader management
// ========================================================
struct shader {
    unsigned int id;
    const char *vert_code;
    const char *frag_code;
};

void shaderLoad(struct shader *sh, const char *filename) {
    char vertname[strlen(filename) + 6];
    char fragname[strlen(filename) + 6];
    snprintf(vertname, sizeof(vertname), "%s%s", filename, ".vert");
    snprintf(fragname, sizeof(fragname), "%s%s", filename, ".frag");
    sh->vert_code = readFile(vertname);
    sh->frag_code = readFile(fragname);
}

void shaderCompile(struct shader *sh) {
    int ok;
    char log[512];
    unsigned int vert = glCreateShader(GL_VERTEX_SHADER);  
    glShaderSource(vert, 1, &sh->vert_code, NULL);
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(vert, sizeof(log), NULL, log);
        ERROR_LOG(
                "Compilation failed (vertex): %s"
                "Code\n"
                "====\n"
                "%s", log, sh->vert_code);
        goto exit;
    }

    unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);  
    glShaderSource(frag, 1, &sh->frag_code, NULL);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(frag, sizeof(log), NULL, log);
        ERROR_LOG(
                "Compilation failed (fragment): %s"
                "Code\n"
                "====\n"
                "%s", log, sh->frag_code);
        goto exit;
    }

    sh->id = glCreateProgram();
    glAttachShader(sh->id, vert);
    glAttachShader(sh->id, frag);
    glLinkProgram(sh->id);
    glGetProgramiv(sh->id, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(sh->id, sizeof(log), NULL, log);
        ERROR_LOG("Compilation_failed (linking): %s", log);
        goto exit;
    }
exit:
    glDeleteShader(vert);
    glDeleteShader(frag);
    if (!ok) exit(1);
}

void shaderUse(struct shader *sh) {
    glUseProgram(sh->id);
}

void shaderSetInt(struct shader *sh, const char *name, int val) {
    glUniform1i(glGetUniformLocation(sh->id, name), val); 
}

void shaderSetFloat(struct shader *sh, const char *name, float val) {
    glUniform1f(glGetUniformLocation(sh->id, name), val); 
}

void shaderFree(struct shader *sh) {
    free((void *) sh->vert_code);
    free((void *) sh->frag_code);
}

// Drawing
// ========================================================
struct painter {
    unsigned int id;
    unsigned int vbo;
    unsigned int vao;
    unsigned int ebo;
    struct shader *sh;
    int n;
    void (*draw_fn)(struct painter *p);
};

void painterInit(struct painter *p, int id) {
    p->id = id;
    glGenVertexArrays(p->id, &p->vao);
    glGenBuffers(p->id, &p->vbo);
    glGenBuffers(p->id, &p->ebo);
}

void painterSetVao(struct painter *p) {
    glBindVertexArray(p->vao);
}

void painterSetVbo(
        struct painter *p,
        float *data,
        int size,
        int buftype) {
    glBindBuffer(GL_ARRAY_BUFFER, p->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, buftype);
}

void painterSetEbo(
        struct painter *p,
        unsigned int *data,
        int size,
        int buftype) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, buftype);
}

void painterSetAtrribVao(
        struct painter *p,
        int ind,
        int size, 
        size_t offset) {
    glVertexAttribPointer(
            ind,
            size,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void *) (offset * sizeof(float)));
    glEnableVertexAttribArray(ind);
}

void painterUpdateVbo(
        struct painter *p,
        float *data,
        int size,
        int offset) {
    glBindBuffer(GL_ARRAY_BUFFER, p->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size * sizeof(float), data);
}

void painterUpdateEbo(
        struct painter *p,
        unsigned int *data,
        int size,
        int offset) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
            offset, size * sizeof(unsigned int), data);
}

void painterDraw(struct painter *p) {
    p->draw_fn(p);
}

// Texture management
// ========================================================
struct texture {
    unsigned int id;
    int wrap;
    int filter;
} static const texture_default = {
    0,
    GL_REPEAT,
    GL_LINEAR
};

void textureGen(struct texture *tex) {
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->filter);
}

void textureLoadData(
        struct texture *tex, 
        unsigned char *data, 
        int width, 
        int height) {
    glTexImage2D(GL_TEXTURE_2D, 
        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

// Texture painter
// ========================================================
static void drawTexture(struct painter *p) {
    shaderUse(p->sh);
    glDrawElements(GL_TRIANGLES, p->n, GL_UNSIGNED_INT, 0);
}

// Pen tool
// ========================================================
struct pen {
    float coods[4];
    int n;
    int use;
    struct painter *painter;
};

struct pen pentool = {
    {0.0, 0.0, 0.0, 0.0},
    0,
    0,
    NULL,
};

static void drawPentoolVert(struct painter *p) {
    shaderUse(p->sh);
    glDrawElements(GL_TRIANGLES, p->n, GL_UNSIGNED_INT, 0);
}

void penUpdatePainter(struct pen *pen) {
    if (pen->n >= 2)
        return;

    int vert_offset = 4 * (pen->n - 1) * sizeof(float);
    int indi_offset = 2 * (pen->n - 1) * sizeof(unsigned int);
    unsigned int indices[] = {pen->n - 1, pen->n};
    painterUpdateVbo(pen->painter,
            pen->coods, sizeof(float) * 4, vert_offset);
    painterUpdateEbo(pen->painter,
            indices, sizeof(unsigned int) * 2, indi_offset);
}

void penDraw(
        struct pen *pen,
        double x,
        double y,
        int btn,
        int action,
        int mods) {
    if (!pen->use)
        return;
    if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        double xo;
        double yo;
        convertScreenCoodsToWorldCoods(x, y, &xo, &yo);
        pen->coods[0] = pen->coods[2];
        pen->coods[1] = pen->coods[3];
        pen->coods[2] = (float) xo;
        pen->coods[3] = (float) yo;
        pen->n += 1;
    } else if (btn == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        
    }
}

// Window management
// ========================================================
static void framebufferSizeCallback(GLFWwindow *window, 
        int width, int height);
static void mouseButtonCallback(GLFWwindow* window,
        int button, int action, int mods);

struct window {
    GLFWwindow *w;
    char *name;
    int width;
    int height;
    struct painter *tex_p;
} const window_default = {
    NULL,
    "AC",
    600,
    800,
    NULL,
};

static void framebufferSizeCallback(
        GLFWwindow *window, 
        int width, 
        int height) {
    glViewport(0, 0, width, height);
}

static void mouseButtonCallback(
        GLFWwindow* win,
        int button,
        int action,
        int mods) {
    double x;
    double y;
    glfwGetCursorPos(win, &x, &y);
    penDraw(&pentool, x, y, button, action, mods);
}

void windowCreate(struct window *win) {
    win->w = glfwCreateWindow(win->width, 
            win->height, win->name, NULL, NULL);
    if (!win->w) {
        ERROR_LOG("Failed to create GLFW window");
        freeGraphicsFramework();
        exit(1);
    }
    glfwMakeContextCurrent(win->w);
}

void windowSetViewportConfig(struct window *win) {
    glViewport(0, 0, win->width, win->height);
    glfwSetFramebufferSizeCallback(win->w, framebufferSizeCallback);
}

void windowSetInputDevices(struct window *win) {
    glfwSetMouseButtonCallback(win->w, mouseButtonCallback);
}

void windowStartRenderLoop(struct window *win) {
    while(!glfwWindowShouldClose(win->w)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        painterDraw(win->tex_p);

        glfwSwapBuffers(win->w);
        glfwPollEvents();    
    }
}

#ifndef SNAKE_STANDALONE
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s [image]\n", argv[0]);
        return 1; 
    }
    const char *filename = argv[1];
    initGraphicsFramework();

    struct window win = window_default;
    unsigned char *imdata = loadImageStb(filename, &win.width, &win.height);

    windowCreate(&win);
    initGladLib();
    windowSetViewportConfig(&win);
    windowSetInputDevices(&win);

    // texture drawing shader
    struct shader tex_sh;
    shaderLoad(&tex_sh, TEX_SHADER_SRC);
    shaderCompile(&tex_sh);

    // texture drawing painter
    struct painter tex_p;
    tex_p.n = ARRAY_LEN(data_indice_array);
    painterInit(&tex_p, 1);
    painterSetVao(&tex_p);
    painterSetVbo(&tex_p,
            data_vertex_array, sizeof(data_vertex_array), GL_STATIC_DRAW);
    painterSetEbo(&tex_p,
            data_indice_array, sizeof(data_indice_array), GL_STATIC_DRAW);
    painterSetAtrribVao(&tex_p, 0, 3, 0);
    painterSetAtrribVao(&tex_p, 1, 3, 3);
    painterSetAtrribVao(&tex_p, 2, 2, 6);
    tex_p.draw_fn = drawTexture;
    tex_p.sh = &tex_sh; 
    win.tex_p = &tex_p;

    // texture
    struct texture tex = texture_default;
    textureGen(&tex);
    textureLoadData(&tex, imdata, win.width, win.height);

    // painter of pentool
    struct painter pen_p;      
    painterInit(&pen_p, 2);
    painterSetVao(&pen_p);
    painterSetVbo(&pen_p, NULL, 1024, GL_DYNAMIC_DRAW);
    painterSetEbo(&pen_p, NULL, 1024, GL_DYNAMIC_DRAW);
    painterSetAtrribVao(&pen_p, 0, 4, 0);
    pentool.painter = &pen_p;
    
    // enable pentool 
    pentool.use = 1;

    // free resources (before entering the render loop)
    freeImageStb(imdata);
    shaderFree(&tex_sh);

    // main render loop
    windowStartRenderLoop(&win);

    freeGraphicsFramework();
    return 0;
}
#endif
