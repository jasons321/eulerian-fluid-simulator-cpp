#include "raylib.h"
#include "Fluid.h"
#include <cstdio>

int main() {
    Fluid f(1000.0f, 10, 10, 0.1f);

    // tank scene: solid border, fluid interior
    for (int i = 0; i < f.numX; i++) {
        for (int j = 0; j < f.numY; j++) {
            float sVal = 1.0f; // fluid
            if (i == 0 || i == f.numX - 1 || j == 0 || j == f.numY - 1)
                sVal = 0.0f; // solid border
            f.s[f.idx(i, j)] = sVal;
        }
    }

    int cellSize = 40; // pixels per grid cell
    int screenWidth = f.numX * cellSize;
    int screenHeight = f.numY * cellSize;

    InitWindow(screenWidth, screenHeight, "Phase 4: tank scene");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < f.numX; i++) {
            for (int j = 0; j < f.numY; j++) {
                Color c = (f.s[f.idx(i, j)] == 0.0f) ? BLACK : WHITE;

                int x = i * cellSize;
                int y = (f.numY - 1 - j) * cellSize; // flip Y: higher j draws higher on screen

                DrawRectangle(x, y, cellSize, cellSize, c);
                DrawRectangleLines(x, y, cellSize, cellSize, LIGHTGRAY);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}