#pragma once

struct interrupt {
    unsigned char master;
    union{
        unsigned char enable; //0xFFFF
        struct {
            unsigned int EVBlank : 1;
            unsigned int ELCD : 1;
            unsigned int ETimer : 1;
            unsigned int ESerial : 1;
            unsigned int EJoypad : 1;
        };
    };
    union{
        unsigned char flag; //0xFFFF
        struct {
            unsigned int FVBlank : 1;
            unsigned int FLCD : 1;
            unsigned int FTimer : 1;
            unsigned int FSerial : 1;
            unsigned int FJoypad : 1;
        };
    };
    //0xFF0F
}extern interrupt;


struct timer {
    unsigned char DIV; // 0xFF04 always incrementing, rest on write
    unsigned char TIMA; // 0xFF05 resets to TMA on oveflow
    unsigned char TMA; // 0xFF06 timer modulo
    union {
        unsigned char TAC; // 0xFF07 timer control
        struct {
            unsigned int Speed : 2;
            unsigned int Enable : 1;
        };
    };
    
}extern timer;


struct joypad {

    union {
        unsigned char keys;
        struct {

            unsigned int P10 : 1; //Right or A
            unsigned int P11 : 1; //Left or B
            unsigned int P12 : 1; // Up or Select
            unsigned int P13 : 1; //Down or Start
            unsigned int P14 : 1; //Action buttons
            unsigned int P15 : 1; //Direction buttons

        };
    };
    
}extern joypad;



extern unsigned char JoypadState;

