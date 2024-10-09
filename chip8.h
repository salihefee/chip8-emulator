#ifndef CHIP_8
#define CHIP_8

using namespace std;

// Define key mappings for CHIP-8 keys
extern const int keymap[16];

// Define the CHIP-8 fontset
extern unsigned char chip8Fontset[80];

class chip8Machine {
public:
    unsigned char screen[64 * 32];
    unsigned char key[16];
    unsigned char draw_flag;

    int initialize(char* rom);
    void emulationCycle();
    void updateTimers();

private:
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char memory[4096];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned short stack[16];
    unsigned short sp;
    unsigned short opcode;

    void loadFontset();
    int loadProgram(char* rom);
    void executeOpcode(unsigned short opcode);
};

#endif // CHIP_8