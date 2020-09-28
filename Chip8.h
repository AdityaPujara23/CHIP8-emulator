#ifndef CHIP_8_H
#define CHIP_8_H

class Chip8 {
    public:
        Chip8(); //chip8 constructor
        ~Chip8(); //chip8 destructor

        bool drawFlag; // Updates Screen: opcode 0x00E0 to clear the screen, 0xDXYN to draw

        bool loadgame(const char * path); // loads a rom file into Chip8
        void emulateCycle(); // Emulate a cycle of Chip8

        unsigned short opcode; // Instructions for Chip8
        unsigned char memory[4096]; // Total memory of Chip8 
        unsigned char registers[16]; // Registers
        unsigned short IR; // Index register
        unsigned short pcounter; // Program counter

        unsigned char graphics[2048]; // Graphics for Chip8 (2048 pixels)
        unsigned char delay_timer; // Delay Timer (delay active when non-zero)
        unsigned char sound_timer; // Sounds Timer for Buzzer (sound active when non-zero)

        unsigned short stack[16]; // Stack for Chip8
        unsigned short stack_pointer; // Stack pointer

        unsigned char keyboard[16]; // Keyboard for Chip8

        void initialize();
};

#endif