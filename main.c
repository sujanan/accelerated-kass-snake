#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <raylib.h>

// ========================================================
// Graphic utils
// ========================================================
static void drawBackgroundTexture(Texture2D tex, int width, int height) {
    DrawTexture(
        tex,
        width / 2 - tex.width / 2,
        height / 2 - tex.height / 2,
        WHITE);
}

static void startMainRenderLoop(
        Texture2D back,
        int width,
        int height) {

    while (!WindowShouldClose()) {
        BeginDrawing();
        // ========================================================
        ClearBackground(RAYWHITE);
        drawBackgroundTexture(back, width, height);
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

    // enter main render loop
    startMainRenderLoop(tex, im.width, im.height);

    // clear resources
    UnloadTexture(tex);

    return 0;
}
