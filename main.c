#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <raylib.h>

#include "util.h"
#include "snake.h"

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
    struct snake *snake;
    int width;
    int height;
};

void envInit(
        struct env *env,
        struct toolbox *box,
        char *filename,
        int width,
        int height) {
    
    struct image *im = imageNew();
    struct contour *con = contourNew();
    struct energy *en = energyNew();
    struct snake *snake = snakeNew();

    imageInit(im);
    char ics[512];
    snprintf(ics, sizeof(ics), "%s.ics", strsep(&filename, "."));
    imageRead(im, ics);
    contourInit(con, 1024);
    energyInit(en);
    energyCalculateForce(en, im, 30.0);
    snakeInit(snake, im, con, en, 0.001, 0.4, 100);

    imageFree(im);
    contourFree(con);
    energyFree(en);

    env->snake = snake;
    env->box = box;
    env->width = width;
    env->height = height;
}

void envFree(struct env *env) {
    snakeFree(env->snake);
}

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

static void printUsage(const char *pro_name) {
    fprintf(stderr,
        "Usage: %s [OPTION]... <FILE>\n" \
        "\n"
        "  --log [debug|info|warning|error]    Change the raylib log level\n",
        pro_name
    );
}

int main(int argc, char **argv) {
    char *filename = NULL;

    if (!strcmp(argv[1], "--log")) {
        if (argc != 4) {
            printUsage(argv[0]);
            return 1; 
        }
        filename = argv[3];
        const char *lvl = argv[2];
        if (!strcmp(lvl, "debug"))
            SetTraceLogLevel(LOG_DEBUG);
        else if (!strcmp(lvl, "info"))
            SetTraceLogLevel(LOG_INFO);
        else if (!strcmp(lvl, "warning"))
            SetTraceLogLevel(LOG_WARNING);
        else if (!strcmp(lvl, "error"))
            SetTraceLogLevel(LOG_ERROR);
        else {
            printUsage(argv[0]);
            return 1;         
        } 
    } else {
        if (argc != 2) {
            printUsage(argv[0]);
            return 1;         
        }
        filename = argv[1];
    }

    // load background image
    Image im = LoadImage(filename);

    // initialize toolbox
    struct pen pen;
    penInit(&pen);
    struct toolbox box;
    box.inuse = TOOLBOX_NONE;
    box.pen = &pen;

    // initialize environment
    struct env env;
    envInit(&env, &box, filename, im.width, im.height);

    // fix heavy CPU usage (Just don't know how yet)
    SetConfigFlags(FLAG_VSYNC_HINT);
    
    // init raylib
    InitWindow(im.width, im.height, "RAYEXP");

    // load background texture
    Texture2D tex = LoadTextureFromImage(im);

    // enter main render loop
    startMainRenderLoop(tex, &env);

    // clear resources
    UnloadTexture(tex);
    penFree(&pen);
    envFree(&env);
    CloseWindow();
    return 0;
}
