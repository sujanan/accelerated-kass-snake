#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <raylib.h>

#include "snake.h"

// ========================================================
// Common utils
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
// ========================================================
// Data structures 
// ========================================================
struct vec2vec {
    Vector2 *data;
    int size;
    int n;
};

#define VEC2VEC_SIZE 1024
#define VEC2VEC_SIZE_INC 32

void vec2vecInit(struct vec2vec *v, int size) {
    v->data = malloc(sizeof(Vector2) * size);
    if (!v->data)
        DIE("Memory error");
    v->size = size;
    v->n = 0;
}

void vec2vecResize(struct vec2vec *v, int size) {
    v->data = realloc(v->data, sizeof(Vector2) * size);
    if (!v->data)
        DIE("Memory error");
    v->size = size;
}

void vec2vecSizeInc(struct vec2vec *v, int inc) {
    vec2vecResize(v, v->size + inc);
}

void vec2vecPush(struct vec2vec *v, Vector2 e) {
    if (v->n >= v->size)
        vec2vecSizeInc(v, VEC2VEC_SIZE_INC);
    v->data[v->n] = e;
    v->n++;
}

void vec2vecRemoveAll(struct vec2vec *v) {
    v->n = 0;
}

void vec2vecFree(struct vec2vec *v) {
    free(v->data);
    v->size = 0;
    v->n = 0;
}

// ========================================================
// Contour drawing toolbox
// ========================================================

// Pen tool
// ========================================================
struct pen {
    struct vec2vec points;
    Color col;
};

void penInit(struct pen *pen) {
    vec2vecInit(&pen->points, VEC2VEC_SIZE);
}

void penAddPoint(struct pen *pen, Vector2 pos) {
    vec2vecPush(&pen->points, pos);
}

void penJoinStartEnd(struct pen *pen) {
    if (pen->points.n < 2)
        return;
    vec2vecPush(&pen->points, pen->points.data[0]);
}

int penPointCount(struct pen *pen) {
    return pen->points.n;
}

int penHasStartEndJoined(struct pen *pen) {
    if (pen->points.n < 3)
        return 0;
    Vector2 s = pen->points.data[0];
    Vector2 e = pen->points.data[pen->points.n - 1];
    return s.x == e.x && s.y == e.y;
}

void penDrawLineStrip(struct pen *pen) {
    DrawLineStrip(pen->points.data, pen->points.n, RAYWHITE);
}

void penReset(struct pen *pen) {
    vec2vecRemoveAll(&pen->points);
}

void penFree(struct pen *pen) {
    vec2vecFree(&pen->points);
}

// Toolbox
// ========================================================
enum toolType {
    TOOLBOX_PEN,
    TOOLBOX_NONE
};

struct toolbox {
    struct pen *pen;
    enum toolType inuse;
};

// ========================================================
// Graphic utils
// ========================================================
struct env {
    struct toolbox *box;
    struct contour *con;
    int width;
    int height;
};

static void drawBackgroundTexture(Texture2D tex, int width, int height) {
    DrawTexture(
        tex,
        width / 2 - tex.width / 2,
        height / 2 - tex.height / 2,
        WHITE);
}

static void mouseInput(struct env *env) {
    struct toolbox *box = env->box;
    switch (box->inuse) {
        case TOOLBOX_PEN: {
            struct pen *pen = box->pen;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (penHasStartEndJoined(pen))
                    penReset(pen);
                penAddPoint(pen, GetMousePosition());
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                penJoinStartEnd(pen);
            break;
        }
        case TOOLBOX_NONE: {
            break;
        }
    }
}

static void keyboardInput(struct env *env) {
    struct toolbox *box = env->box;
    if (IsKeyDown(KEY_P))
        box->inuse = TOOLBOX_PEN;
    else if (IsKeyDown(KEY_N))
        box->inuse = TOOLBOX_NONE;
}

static void startMainRenderLoop(Texture2D back, struct env *env) {
    while (!WindowShouldClose()) {
        BeginDrawing();
        // ========================================================
        ClearBackground(RAYWHITE);
        drawBackgroundTexture(back, env->width, env->height);
        mouseInput(env);
        keyboardInput(env);
        penDrawLineStrip(env->box->pen);
        // ========================================================
        EndDrawing();
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>", argv[0]);
        return 1; 
    }

    // load background image
    const char *filename = argv[1];
    Image im = LoadImage(filename);

    // fix heavy CPU usage (Just don't know how yet)
    SetConfigFlags(FLAG_VSYNC_HINT);
    
    // init raylib
    InitWindow(im.width, im.height, "RAYEXP");

    // load background texture
    Texture2D tex = LoadTextureFromImage(im);

    // initialize toolbox
    struct pen pen;
    penInit(&pen);
    struct toolbox box;
    box.inuse = TOOLBOX_NONE;
    box.pen = &pen;

    // initialize contour
    struct contour *con = contourNew();

    // initialize environment
    struct env env;
    env.con = con;
    env.box = &box;
    env.width = im.width;
    env.height = im.height;

    // enter main render loop
    startMainRenderLoop(tex, &env);

    // clear resources
    UnloadTexture(tex);
    penFree(&pen);
    contourFree(con);

    return 0;
}
