#include "raylib.h"
#include "fluid.h"

int main() {
    Fluid f(1000.0f, 100, 50, 0.02f); // wider than tall, decent resolution

    // wind tunnel scene: inflow on the left, open on the right, walls top/bottom
    float inVel = 2.0f;
    for (int i = 0; i < f.numX; i++) {
        for (int j = 0; j < f.numY; j++) {
            float sVal = 1.0f;
            if (i == 0 || j == 0 || j == f.numY - 1)
                sVal = 0.0f; // solid: left wall, top, bottom
            f.s[f.idx(i, j)] = sVal;
        }
    }

    int pipeH = (int)(0.1f * f.numY);
    int minJ = f.numY / 2 - pipeH / 2;
    int maxJ = f.numY / 2 + pipeH / 2;

    int cellSize = 10; // smaller now that the grid is bigger
    int screenWidth = f.numX * cellSize;
    int screenHeight = f.numY * cellSize;

    InitWindow(screenWidth, screenHeight, "Wind tunnel: smoke advection");
    SetTargetFPS(60);

    float dt = 1.0f / 60.0f;
    float gravity = 0.0f; // wind tunnel scene has no gravity
    int numIters = 40;
    float overRelaxation = 1.9f;

    while (!WindowShouldClose()) {
        // reapply inflow velocity and smoke source every frame, otherwise
        // they get advected away after the first step and the screen
        // fades to white
        for (int j = minJ; j < maxJ; j++) {
            f.u[f.idx(1, j)] = inVel;
            f.m[f.idx(1, j)] = 0.0f; // dark stripe against white background
        }

        f.simulate(dt, gravity, numIters, overRelaxation);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < f.numX; i++) {
            for (int j = 0; j < f.numY; j++) {
                float density = f.m[f.idx(i, j)];
                unsigned char shade = (unsigned char)(255 * density);
                Color c = { shade, shade, shade, 255 };

                int x = i * cellSize;
                int y = (f.numY - 1 - j) * cellSize;

                DrawRectangle(x, y, cellSize, cellSize, c);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}