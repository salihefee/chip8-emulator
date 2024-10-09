#include "chip8.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>

// Define the CHIP-8 fontset
unsigned char chip8Fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int chip8Machine::initialize(char* rom) {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    fill(begin(screen), end(screen), 0);
    fill(begin(V), end(V), 0);
    fill(begin(stack), end(stack), 0);
    fill(begin(memory), end(memory), 0);

    int loadSuccess = loadProgram(rom);
    if (!loadSuccess) {
        cerr << "Error: ROM file " << rom << " could not be loaded." << endl;
        return 0;
    }
    loadFontset();

    delay_timer = 0;
    sound_timer = 0;
    draw_flag = 0;

    // Seed the random number generator with the current epoch time
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    return 1;
}

void chip8Machine::emulationCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];
    cout << hex << opcode << endl;
    executeOpcode(opcode);
}

void chip8Machine::updateTimers() {
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        sound_timer--;
        // Optionally, handle sound here if sound_timer reaches zero (I don't even know what this counter is for)
    }
}

void chip8Machine::loadFontset() {
    for (unsigned short charptr = 0; charptr < 80; charptr++) {
        memory[charptr] = chip8Fontset[charptr];
    }
}

int chip8Machine::loadProgram(char* rom) {
    ifstream program;
    program.open(rom, ios::binary);

    if (!program.is_open())
        return 0;

    program.seekg(0, ios::end);
    streampos file_size = program.tellg();
    program.seekg(0, ios::beg);
    size_t size = static_cast<size_t>(file_size);

    vector<char> buffer(size);
    program.read(buffer.data(), size);

    for (unsigned short program_ptr = 0x200; program_ptr < 0x200 + size; program_ptr++) {
        memory[program_ptr] = buffer[program_ptr - 0x200];
    }

    return 1;
}

void chip8Machine::executeOpcode(unsigned short opcode) {
    unsigned char X = (opcode & 0x0F00) >> 8;
    unsigned char Y = (opcode & 0x00F0) >> 4;
    unsigned char NN = (opcode & 0x00FF);
    // Decode opcode
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x000F) {
        case 0x0000: // Clear the screen        
            fill(begin(screen), end(screen), 0);
            draw_flag = 1;
            pc += 2;
            break;

        case 0x000E: // Return from subroutine          
            pc = stack[--sp];
            pc += 2;
            break;

        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;

    case 0x2000: // Call subroutine at NNN
        stack[sp] = pc;
        sp++;
        pc = opcode & 0x0FFF;
        break;

    case 0x1000: // Jump to address at NNN
        pc = opcode & 0x0FFF;
        break;

    case 0x3000: {// Skip the next instruction if VX equals NN 
        if (V[X] == NN) {
            pc += 2;
        }
        pc += 2;
        break;
    }

    case 0x4000: { // Skip the next instruction if VX does not equal NN
        if (V[X] != NN)
            pc += 2;
        pc += 2;
        break;
    }

    case 0x5000: // Skip the next instruction if VX equals VY
        if (V[X] == V[Y])
            pc += 2;
        pc += 2;
        break;

    case 0x6000: { // Sets VX to NN
        V[X] = NN;
        pc += 2;
        break;
    }

    case 0x7000: { // Adds NN to VX (carry flag is not changed)
        V[X] += NN;
        pc += 2;
        break;
    }

    case 0x8000:
        switch (opcode & 0x000F) {
        case 0x0000: // Sets VX to the value of VY
            V[X] = V[Y];
            pc += 2;
            break;

        case 0x0001: // Sets VX to VX or VY. (bitwise OR operation)
            V[X] |= V[Y];
            pc += 2;
            break;

        case 0x0002: // Sets VX to VX and VY. (bitwise AND operation)
            V[X] &= V[Y];
            pc += 2;
            break;

        case 0x0003: // Sets VX to VX xor VY
            V[X] ^= V[Y];
            pc += 2;
            break;

        case 0x0004: // Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not
            if (V[Y] > (0xFF - V[X]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[X] += V[Y];
            pc += 2;
            break;

        case 0x0005: // VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not.
            if (V[Y] > V[X])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[X] -= V[Y];
            pc += 2;
            break;

        case 0x0006: // Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF
            V[0xF] = V[X] & 0x01;
            V[X] >>= 1;
            pc += 2;
            break;

        case 0x0007: // Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not.
            if (V[X] > V[Y])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[X] = V[Y] - V[X];
            pc += 2;
            break;

        case 0x000E: // Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset
            V[0xF] = V[X] & 0x80;
            V[X] <<= 1;
            pc += 2;
            break;
        }
        break;

    case 0x9000: // Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block)
        if (V[X] != V[Y])
            pc += 2;
        pc += 2;
        break;

    case 0xA000: // Sets I to the address NNN
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: // Jumps to the address NNN plus V0
        pc = V[0] + (opcode & 0x0FFF);
        break;

    case 0xC000: // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
        V[X] = (rand() % 256) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: { // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each x of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.[24]
        V[0xF] = 0;
        unsigned char pixel;
        unsigned int index = V[Y] * 64 + V[X];
        unsigned short height = opcode & 0x000F;

        for (unsigned char yOffset = 0; yOffset < height; yOffset++) {
            pixel = memory[I + yOffset];
            for (unsigned char xOffset = 0; xOffset < 8; xOffset++) {
                if ((pixel & (0x80 >> xOffset)) != 0) {
                    unsigned int screenIndex = ((V[Y] + yOffset) % 32) * 64 + ((V[X] + xOffset) % 64);

                    if (screen[screenIndex] == 1) {
                        V[0xF] = 1;
                    }

                    screen[screenIndex] ^= 1;
                }
            }
        }

        draw_flag = 1;

        pc += 2;
        break;
    }

    case 0xE000:
        switch (opcode & 0x000F) {
        case 0x000E: // Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block)
            if (key[V[X]]) {
                pc += 2;
            }
            pc += 2;
            break;

        case 0x0001: // Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block)
            if (!key[V[X]]) {
                pc += 2;
            }
            pc += 2;
            break;
        }

        break;

    case 0xF000:
        switch (opcode & 0x00FF) {
        case 0x0007: // Sets VX to the value of the delay timer
            V[X] = delay_timer;
            pc += 2;
            break;

        case 0x000A: {// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)
            int key_pressed = 0;

            for (int i = 0; i < 16; i++) {
                if (key[i]) {
                    V[X] = i;
                    key_pressed = 1;
                    break;
                }
            }

            if (!key_pressed) {
                return;
            }

            pc += 2;
            break;
        }

        case 0x0015: // Sets the delay timer to VX
            delay_timer = V[X];
            pc += 2;
            break;

        case 0x0018: // Sets the sound timer to VX
            sound_timer = V[X];
            pc += 2;
            break;

        case 0x001E: // Adds VX to I. VF is not affected
            I += V[X];
            pc += 2;
            break;

        case 0x0029: // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
            I = V[X] * 5;
            pc += 2;
            break;

        case 0x0033: // Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
            memory[I] = V[X] / 100;
            memory[I + 1] = (V[X] / 10) % 10;
            memory[I + 2] = (V[X] % 100) % 10;
            pc += 2;
            break;

        case 0x0055: // Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
            for (int index = 0; index < X + 1; index++) {
                memory[I + index] = V[index];
            }
            pc += 2;
            break;

        case 0x0065: // Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified
            for (int index = 0; index < X + 1; index++) {
                V[index] = memory[I + index];
            }
            pc += 2;
            break;

        }
        break;
    }
}