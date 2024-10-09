#ifndef CHIP_8
#define CHIP_8

using namespace std;

// Define key mappings for CHIP-8 keys
extern const int keymap[16];

// Define the CHIP-8 fontset
extern unsigned char chip8Fontset[80];

class chip8Machine {
public:
    unsigned char screen[64 * 32] = {0};
    unsigned char key[16] = {0};
    unsigned char draw_flag = 0;

    int initialize(const char* rom);
    void emulationCycle();
    void updateTimers();

private:
    unsigned char delay_timer = 0;
    unsigned char sound_timer = 0;
    unsigned char memory[4096] = {0};
    unsigned char V[16] = {0};
    unsigned short I = 0;
    unsigned short pc = 0;
    unsigned short stack[16] = {0};
    unsigned short sp = 0;
    unsigned short opcode = 0;

    void loadFontset();
    int loadProgram(const char* rom);
    void executeOpcode(unsigned short opcode);
};

#endif // CHIP_8