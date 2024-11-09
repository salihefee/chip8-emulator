#include <iostream>
#include "chip8.h"
#include "raylib.h"

using namespace std;

chip8Machine chip8;

// Key mappings for CHIP-8 keys
const int keymap[16] = {
    KEY_X,    // 0
    KEY_ONE,  // 1
    KEY_TWO,  // 2
    KEY_THREE,// 3
    KEY_Q,    // 4
    KEY_W,    // 5
    KEY_E,    // 6
    KEY_A,    // 7
    KEY_S,    // 8
    KEY_D,    // 9
    KEY_Z,    // A
    KEY_C,    // B
    KEY_FOUR, // C
    KEY_R,    // D
    KEY_F,    // E
    KEY_V,     // F
};

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <path to ROM file>\n";
        return 1;
    }

    if (!chip8.initialize(argv[1])) {
        return 1;
    }

    constexpr int screenWidth = 1024;
    constexpr int screenHeight = 512;
    constexpr int scale = screenHeight / 32;
    InitWindow(screenWidth, screenHeight, "CHIP-8 Emulator");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_F1)) {
            chip8.initialize(argv[1]);
        }

        chip8.emulationCycle();

        PollInputEvents();

        for (int i = 0; i < 16; i++) {
            chip8.key[i] = IsKeyDown(keymap[i]);
        }

        if (chip8.draw_flag) {
            chip8.draw_flag = 0;
            BeginDrawing();
            ClearBackground(BLACK);
            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    if (chip8.screen[y * 64 + x] == 1) {
                        DrawRectangle(x * scale, y * scale, scale, scale, WHITE);
                    }
                }
            }
            EndDrawing();
        }

    }


    CloseWindow();

    return 0;
}

