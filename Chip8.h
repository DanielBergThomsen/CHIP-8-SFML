#ifndef CHIP8_H

#define CHIP8_H

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;

class Chip8
{
private:
        unsigned short opcode;
        unsigned char memory [4096] = {0};
        unsigned char V[16] = {0};
        unsigned short I;
        unsigned short pc;

        unsigned short stack[16] = {0};
        unsigned short sp;

        unsigned char gfx[64*32] = {0};

        unsigned char delay_timer;
        unsigned char sound_timer;
        static const unsigned SAMPLES = 44100;
        static const unsigned SAMPLE_RATE = 44100;
        static const unsigned AMPLITUDE = 30000;

        sf::Int16 raw[SAMPLES];

        sf::SoundBuffer Buffer;

        sf::Sound Sound;

        const double TWO_PI = 6.28318;
        const double increment = 440./44100;

        unsigned char key[16] = {0};
        sf::Keyboard::Key keyNum[16] =
                {
                        sf::Keyboard::X, sf::Keyboard::Num1, sf::Keyboard::Num2,
                        sf::Keyboard::Num3, sf::Keyboard::Q, sf::Keyboard::W,
                        sf::Keyboard::E, sf::Keyboard::A, sf::Keyboard::S,
                        sf::Keyboard::D, sf::Keyboard::Z, sf::Keyboard::C,
                        sf::Keyboard::Num4, sf::Keyboard::R, sf::Keyboard::F,
                        sf::Keyboard::V
                };

        unsigned char fontset[80] =
                {
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

        bool isOn;
        bool drawFlag;

        ifstream file;

        void fetch();
        void execute();
        void update();

        void RESET();
        void CLEAR();
        void RETURN();

        void JUMP();
        void SUB();
        void SKIP_IF_VX();
        void SKIP_IF_NOT_VX();
        void SKIP_IF_VX_VY();
        void SET_VX();
        void ADD_TO_VX();

        void ARITHMETIC();
        void VX_VY();
        void VX_OR_VY();
        void VX_AND_VY();
        void VX_XOR_VY();
        void ADD_VY_VX();
        void SUB_VY_VX();
        void SHIFT_VX_RIGHT();
        void SET_VX_VY_SUB_VX();
        void SHIFT_VX_LEFT();

        void SKIP_IF_VX_NOT_VY();
        void SET_I();
        void JUMP_ADD_V0();
        void SET_VX_RANDOM();
        void DRAW();

        void SKIP_IF_KEY();
        void SKIP_IF_KEY_VX();
        void SKIP_IF_KEY_NOT_VX();

        void OTHERS();


        void (Chip8::*Chip8Table[16]) () =
                {
                        &Chip8::RESET,          &Chip8::JUMP,
                        &Chip8::SUB,            &Chip8::SKIP_IF_VX,
                        &Chip8::SKIP_IF_NOT_VX, &Chip8::SKIP_IF_VX_VY,
                        &Chip8::SET_VX,         &Chip8::ADD_TO_VX,
                        &Chip8::ARITHMETIC,     &Chip8::SKIP_IF_VX_NOT_VY,
                        &Chip8::SET_I,          &Chip8::JUMP_ADD_V0,
                        &Chip8::SET_VX_RANDOM,  &Chip8::DRAW,
                        &Chip8::SKIP_IF_KEY,    &Chip8::OTHERS
                };

        void (Chip8::*Chip8Reset[15]) () =
                {
                        &Chip8::CLEAR,          NULL,
						NULL,                   NULL,
						NULL,                   NULL,
						NULL,                   NULL,
						NULL,                   NULL,
						NULL,                   NULL,
						NULL,                   NULL,
						&Chip8::RETURN
                };

        void (Chip8::*Chip8Arithmetic[15]) () =
                {
                        &Chip8::VX_VY,             &Chip8::VX_OR_VY,
                        &Chip8::VX_AND_VY,         &Chip8::VX_XOR_VY,
                        &Chip8::ADD_VY_VX,         &Chip8::SUB_VY_VX,
                        &Chip8::SHIFT_VX_RIGHT,    &Chip8::SET_VX_VY_SUB_VX,
                        NULL,                      NULL,
                        NULL,                      NULL,
                        NULL,                      NULL,
                        &Chip8::SHIFT_VX_LEFT
                };

        void (Chip8::*Chip8SkipIfKey[16]) () =
                {
                        NULL,                       &Chip8::SKIP_IF_KEY_NOT_VX,
                        NULL,                       NULL,
                        NULL,                       NULL,
                        NULL,                       NULL,
                        NULL,                       NULL,
                        NULL,                       NULL,
                        NULL,                       NULL,
                        &Chip8::SKIP_IF_KEY_VX,     NULL
                };
public:
        Chip8();
        vector<unsigned char> getGFXArray();
        void emulateCycle();
        bool getChipState();
        bool getDrawFlag();
        void setDrawFlag(bool flag);
        void loadROM(const string& fileName);
        void shutdown();
		void printDebug();
};

#endif
