#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include "Chip8.h"


using namespace std;

//Sprites for Chip8 (binary represenation of the desired picture)
unsigned char chip8_Sprites[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //Sprite for '0'
    0x20, 0x60, 0x20, 0x20, 0x70, //Sprite for '1'
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //Sprite for '2'
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //Sprite for '3'
    0x90, 0x90, 0xF0, 0x10, 0x10, //Sprite for '4'
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //Sprite for '5'
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //Sprite for '6'
    0xF0, 0x10, 0x20, 0x40, 0x40, //Sprite for '7'
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //Sprite for '8'
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //Sprite for '9'
    0xF0, 0x90, 0xF0, 0x90, 0x90, //Sprite for 'A'
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //Sprite for 'B'
    0xF0, 0x80, 0x80, 0x80, 0xF0, //Sprite for 'C'
    0xE0, 0x90, 0x90, 0x90, 0xE0, //Sprite for 'D'
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //Sprite for 'E'
    0xF0, 0x80, 0xF0, 0x80, 0x80  //Sprite for 'F'
};

// implementation of the default Chip8 constructor
Chip8::Chip8() {
}
// implementation of Chip8 initialize function
void Chip8::initialize()
{   
    //Reseting Chip8
    pcounter = 0x200;    // Set program counter to 0x200
    opcode = 0;          // Reset op code
    IR = 0;              // Reset I
    stack_pointer = 0;   // Reset stack pointer

    // Clearing the display
    for (int i = 0; i < 2048; i++) {
        graphics[i] = 0;
    }

    // Clearing the stack, keypad, and registers
    for (int i = 0; i < 16; ++i) {
        stack[i] = 0;
        keyboard[i] = 0;
        registers[i] = 0;
    }

    // Clearing memory
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    // Load font set into memory
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_Sprites[i];
    }

    // Reseting timers
    delay_timer = 0;
    sound_timer = 0;

}
// Executing Opcode (Emulating one cycle of Chip8)
void Chip8::emulateCycle()
{
  // Creating opcode from memory
  opcode = memory[pcounter] << 8 | memory[pcounter + 1]; //opcode is 2 bytes : combining instructions

  // Decoding opcode based on standard Chip8 instructions from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#00E0
    switch(opcode & 0xF000){
        case 0x0000:
            switch (opcode & 0x000F) {
                // 00E0 - Clear screen: CLS
                case 0x0000:
                    for (int i = 0; i < 2048; ++i) {
                        graphics[i] = 0;
                    }
                    drawFlag = true;
                    pcounter+=2;
                    break;
                // 00EE - Set program counter to address of top of stack
                case 0x000E:
                    --stack_pointer; // subtract 1 from stack pointer
                    pcounter = stack[stack_pointer];
                    pcounter += 2;
                    break;
                default:
                    printf("\nUnknown Opcode: %.4X\n", opcode);
                    exit(3);
            }
            break;
        // 1NNN - Jump to location address NNN
        case 0x1000:
            pcounter = opcode & 0x0FFF; // set program counter to NNN
            break;

        // 2NNN -Increment stack pointer, current PC on top of stack, PC set to NNN
        case 0x2000:
            stack[stack_pointer] = pcounter;
            ++stack_pointer;
            pcounter = opcode & 0x0FFF; // set program counter to NNN
            break;

        // 3XNN - Skips the next instruction if VX equals NN.
        case 0x3000:
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) // comparing register Vx to NN
                pcounter += 4;
            else
                pcounter += 2;
            break;

        // 4XNN - Skips the next instruction if VX does not equal NN.
        case 0x4000:
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) // comparing register Vx to NN
                pcounter += 4;
            else
                pcounter += 2;
            break;

        // 5XY0 - Skips the next instruction if VX equals VY.
        case 0x5000:
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) // comparing register Vx to Vy
                pcounter += 4;
            else
                pcounter += 2;
            break;

        // 6XNN - Sets VX to NN.
        case 0x6000:
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF; // setting Vx to NN
            pcounter += 2; // incrementing program counter
            break;

        // 7XNN - Adds NN to VX.
        case 0x7000:
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF; //Vx = Vx + NN
            pcounter += 2; // incrementing program counter
            break;

        // 8XY_
        case 0x8000:
            switch (opcode & 0x000F) {

                // 8XY0 - Stores value of Vy in register Vx
                case 0x0000:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4]; // Vx = Vy
                    pcounter += 2; // increments program counter
                    break;

                // 8XY1 - Sets VX to (VX OR VY).
                case 0x0001:
                    registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4]; // Vx = Vy/Vx
                    pcounter += 2; // increments program counter
                    break;

                // 8XY2 - Sets VX to (VX AND VY).
                case 0x0002:
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4]; 
                    pcounter += 2; // increments program counter
                    break;

                // 8XY3 - Sets VX to (VX XOR VY).
                case 0x0003:
                    registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4]; //Bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx
                    pcounter += 2; // increments program counter
                    break;

                // 8XY4 - Adds VY to VX. VF is set to 1 when result is greater than 8 bits,
                // else VF = 0
                case 0x0004:
                    registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4]; 
                    if(registers[(opcode & 0x00F0) >> 4] > (0xFF - registers[(opcode & 0x0F00) >> 8]))
                        registers[0xF] = 1; //carry
                    else
                        registers[0xF] = 0;
                    pcounter += 2; // increments program counter
                    break;

                // 8XY5 - VY is subtracted from VX. VF is set to 1 when
                // VX is greater than VY, and 0 when it is not.
                case 0x0005:
                    if(registers[(opcode & 0x00F0) >> 4] > registers[(opcode & 0x0F00) >> 8])
                        registers[0xF] = 0; // VX < VY
                    else
                        registers[0xF] = 1; // VX > VY
                    registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4]; // VX = VX - VY
                    pcounter += 2; // increments program counter
                    break;

                // 0x8XY6 - VF is set to the value of the least significant bit of VX. 
                // Shifts VX right by one. 
                case 0x0006:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x1;
                    registers[(opcode & 0x0F00) >> 8] >>= 1;
                    pcounter += 2; // increments program counter
                    break;

                // 0x8XY7: Sets VX to VY minus VX. VF is set to 1 when VY > VX
                // and 0 when it is not.
                case 0x0007:
                    if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4])	// VY-VX
                        registers[0xF] = 0; // there is a borrow
                    else
                        registers[0xF] = 1;
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
                    pcounter += 2;
                    break;

                // 0x8XYE: Shifts VX left by one. VF is set to the value of
                // the most significant bit of VX before the shift.
                case 0x000E:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] >> 7;
                    registers[(opcode & 0x0F00) >> 8] <<= 1;
                    pcounter += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // 9XY0 - Skips the next instruction if VX isn't equal to VY.
        case 0x9000:
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4])
                pcounter += 4;
            else
                pcounter += 2;
            break;

        // ANNN - Sets index register to the address NNN.
        case 0xA000:
            IR = opcode & 0x0FFF;
            pcounter += 2;
            break;

        // BNNN - Jumps to the address NNN plus V0.
        case 0xB000:
            pcounter = (opcode & 0x0FFF) + registers[0];
            break;

        // CXNN - Sets VX to a random number, masked by NN.
        case 0xC000:
            registers[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            pcounter += 2;
            break;

        // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8
        // pixels and a height of N pixels.
        // Each row of 8 pixels is read as bit-coded starting from memory
        // VF is set to 1 if any screen pixels are flipped from set to unset
        // when the sprite is drawn, else to 0.
        case 0xD000:
        {
            unsigned short x = registers[(opcode & 0x0F00) >> 8];
            unsigned short y = registers[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            registers[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[IR + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(graphics[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            registers[0xF] = 1;
                        }
                        graphics[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pcounter += 2;
        }
            break;

        // EX__
        case 0xE000:

            switch (opcode & 0x00FF) {
                // EX9E - Skips the next instruction if the key stored
                // in VX is pressed.
                case 0x009E:
                    if (keyboard[registers[(opcode & 0x0F00) >> 8]] != 0)
                        pcounter +=  4;
                    else
                        pcounter += 2;
                    break;

                // EXA1 - Skips the next instruction if the key stored
                // in VX isn't pressed.
                case 0x00A1:
                    if (keyboard[registers[(opcode & 0x0F00) >> 8]] == 0)
                        pcounter +=  4;
                    else
                        pcounter += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // FX__
        case 0xF000:
            switch(opcode & 0x00FF)
            {
                // FX07 - Sets VX to the value of the delay timer
                case 0x0007:
                    registers[(opcode & 0x0F00) >> 8] = delay_timer;
                    pcounter += 2;
                    break;

                // FX0A - A key press is awaited, and then stored in VX
                case 0x000A:
                {
                    bool key_pressed = false;

                    for(int i = 0; i < 16; ++i)
                    {
                        if(keyboard[i] != 0)
                        {
                            registers[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if(!key_pressed)
                        return;

                    pcounter += 2;
                }
                    break;

                // FX15 - Sets the delay timer to VX
                case 0x0015:
                    delay_timer = registers[(opcode & 0x0F00) >> 8];
                    pcounter += 2;
                    break;

                // FX18 - Sets the sound timer to VX
                case 0x0018:
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    pcounter += 2;
                    break;

                // FX1E - Adds VX to IR, stores in IR
                case 0x001E:
                    // VF is set to 1 when range overflow (I+VX>0xFFF), and 0
                    // when there isn't.
                    if(IR + registers[(opcode & 0x0F00) >> 8] > 0xFFF)
                        registers[0xF] = 1;
                    else
                        registers[0xF] = 0;
                    IR += registers[(opcode & 0x0F00) >> 8];
                    pcounter += 2;
                    break;

                // FX29 - Sets IR to the location of the sprite for the
                // character in VX. Characters 0-F (in hexadecimal) are
                // represented by a 4x5 font
                case 0x0029:
                    IR = registers[(opcode & 0x0F00) >> 8] * 0x5;
                    pcounter += 2; // increments program counter
                    break;

                // FX33 - Stores the Binary-coded decimal representation of VX
                // at the addresses IR, IR plus 1, and IR plus 2
                case 0x0033:
                    memory[IR]     = registers[(opcode & 0x0F00) >> 8] / 100;
                    memory[IR + 1] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[IR + 2] = registers[(opcode & 0x0F00) >> 8] % 10;
                    pcounter += 2; // increments program counter
                    break;

                // FX55 - Stores V0 to VX in memory starting at address IR
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[IR + i] = registers[i];
                    pcounter += 2; // increments program counter
                    break;
                // FX65 - Stores V0 to VX in memory starting at address IR
                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        registers[i] = memory[IR + i];
                    pcounter += 2; // increments program counter
                    break;

                default:
                    printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;

        default:
            printf("\nUnimplemented op code: %.4X\n", opcode);
            exit(3);
    }
  // Updating timers
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        if(sound_timer == 1) {
        --sound_timer;
        }
    }
}

// implementation of Chip8 load ROM function
bool Chip8::loadgame(const char *path) {
    // Initialise
    initialize();
    // Updating user of the file path being used
    printf("Loading ROM: %s\n", path);

    // Open ROM file
    FILE* rom = fopen(path, "rb");
    if (rom == NULL) {
        // standard error stream
        std::cerr << "Failed to open ROM" << std::endl;
        return false;
    }

    // Get file size to allocate buffer
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    // Allocate memory buffer to store rom
    char* rom_buffer = (char*) malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL) {
        // standard error stream
        std::cerr << "Failed to allocate memory for ROM" << std::endl;
        return false;
    }

    // Copy ROM into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
    if (result != rom_size) {
        // standard error stream
        std::cerr << "Failed to read ROM" << std::endl;
        return false;
    }

    // Copy memory buffer to Chip8 memory
    if ((4096-512) > rom_size){
        for (int i = 0; i < rom_size; ++i) {
            memory[i + 512] = (uint8_t)rom_buffer[i];   // Load into memory starting at 0x200 (=512)
        }
    }
    else {
        // standard error stream
        std::cerr << "ROM too large to fit in memory" << std::endl;
        return false;
    }

    // Clean up after ROM
    fclose(rom);
    free(rom_buffer);

    return true;
}

// implementation of the default Chip8 destructor
Chip8::~Chip8() {
}

