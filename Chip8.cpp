#include "Chip8.h"

Chip8::Chip8()
{
        // Initilize
        pc = 0x200; // Programs for Chip-8 always start at address 0x200 forward
        opcode = 0;
        I = 0;
        sp = 0;
        srand(time(NULL));

        // Load fontset (0x50 and forward)
        for(int i = 0; i < 80; ++i)
        {
                memory[i] = fontset[i];
        }

        isOn = true;
        drawFlag = false;

        // Set up sound to continously play a sine wave
        double x = 0;
        for (unsigned i = 0; i < SAMPLES; i++)
        {
                raw[i] = AMPLITUDE * sin(x*TWO_PI);
                x += increment;
        }

        if (!Buffer.loadFromSamples(raw, SAMPLES, 1, SAMPLE_RATE))
        {
                std::cerr << "Loading failed!" << std::endl;
        }

        Sound.setBuffer(Buffer);
        Sound.setLoop(true);
}

void Chip8::fetch()
{
        // Fetch opcode from memory
        opcode = memory[pc] << 8 | memory[pc + 1];
}

void Chip8::execute()
{
	(this->*Chip8Table[opcode >> 12])(); 
}

void Chip8::update()
{
        // Start by checking state of keyboard
        for(int i = 0; i < 16; ++i)
        {
                key[i] = sf::Keyboard::isKeyPressed(keyNum[i]);
        }

        // Update timers
        if(delay_timer > 0)
                --delay_timer;

        if(sound_timer > 0)
        { // Chip-8 defines a single beep to be played whenever the sound timer is non-zero
                if(Sound.getStatus() != sf::Sound::Status::Playing)
                { // Only start playing sound if sound isn't already playing
                        Sound.play();
                }
                --sound_timer;
        } else
        {
                Sound.stop();
        }
}

vector<unsigned char>Chip8::getGFXArray()
{ // Returns state of VRAM for graphical output
        vector<unsigned char> v(begin(gfx), end(gfx));
        return v;
}

void Chip8::RESET()
{
        (this->*Chip8Reset[opcode & 0x000F])();
}

void Chip8::CLEAR() // 0x00E0: Clear display
{
        for(int i = 0; i < 64 * 32; ++i)
        {
                gfx[i] = 0;
        }
        drawFlag = true;
        pc += 2;
}

void Chip8::RETURN() // 0x00EE: Return from subroutine
{
        --sp;
        pc = stack[sp];
        pc += 2;
}

void Chip8::JUMP() // 0x1NNN: Jump to address NNN
{
        pc = (opcode & 0x0FFF);
}

void Chip8::SUB() // 0x2NNN: Call subroutine at NNN
{
        stack[sp] = pc;
        ++sp;
        pc = (opcode & 0x0FFF);
}

void Chip8::SKIP_IF_VX() // 0x3XNN: Skip next instruction if VX equals NN
{
        if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
                pc += 4;
        } else
        {
                pc += 2;
        }
}

void Chip8::SKIP_IF_NOT_VX() // 0x4XNN: Skip next instruction if VX does not equal NN
{
        if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
                pc += 4;
        } else
        {
                pc += 2;
        }
}

void Chip8::SKIP_IF_VX_VY() // 0x5XY0: Skip next instruction if VX = VY
{
        if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
                pc += 4;
        } else
        {
                pc += 2;
        }
}

void Chip8::SET_VX() // 0x6XNN: Sets VX to NN
{
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        pc += 2;
}

void Chip8::ADD_TO_VX() // 0x7XNN: Adds NN to VX
{
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        pc += 2;
}

void Chip8::ARITHMETIC()
{
        (this->*Chip8Arithmetic[opcode & 0x000F])();
}

void Chip8::VX_VY() // 0x8XY0: Assigns VX to VY
{
        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
        pc += 2;
}

void Chip8::VX_OR_VY() // 0x8XY1: Bitwise or between VX and VY. Reset VF
{
        V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
        V[0xF] = 0;
}

void Chip8::VX_AND_VY() // 0x8XY2: Bitwise and between VX and VY. Reset VF
{
        V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
        pc += 2;
}

void Chip8::VX_XOR_VY() // 0x8XY3: Bitwise XOR between VX and VY. Reset VF
{
        V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
        pc += 2;
}

void Chip8::ADD_VY_VX() // 0x8XY4: Adds VY to VX. Carry flag is set (VF)
{
        if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
        {
                V[0xF] = 0x01; // 1 is carry by def
        }else
        {
                V[0xF] = 0x00; // No carry necessary
        }
        V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
        pc += 2;
}

void Chip8::SUB_VY_VX() // 0x8XY5: Subtract VY from VX. Borrow flag set (VF)
{
        if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
        {
                V[0xF] = 0x00;
        }else
        {
                V[0xF] = 0x01;
        }
        V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
        pc += 2;
}

void Chip8::SHIFT_VX_RIGHT () // 0x8XY6: Shift VX right by 1. VF is set to least significant bit before shift.
{
        V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1; //Is the bit reached????
        V[(opcode & 0x0F00) >> 8] >>= 1;
        pc += 2;
}

void Chip8::SET_VX_VY_SUB_VX() // 0x8XY7: Set VX to VY - VX. Borrow flag set (VF)
{
        if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
        {
                V[0xF] = 0x00;
        }else
        {
                V[0xF] = 0x01;
        }
        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
        pc += 2;
}

void Chip8::SHIFT_VX_LEFT() // 0x8XYE: Shifts VX left by 1. VF is set to most significant bit before shift.
{
        V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
        V[(opcode & 0x0F00) >> 8] <<= 1;
        pc += 2;
}

void Chip8::SKIP_IF_VX_NOT_VY() // 0x9XY0: Skip next instruction if VX != VY.
{
        if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        {
                pc += 4;
        }else
        {
                pc += 2;
        }
}

void Chip8::SET_I() // 0xANNN: Sets the 16-bit register to addess NNN
{
        I = (opcode & 0x0FFF);
        pc += 2;
}

void Chip8::JUMP_ADD_V0() // 0xBNNN: Jump to address NNN + V0
{
        pc = (opcode & 0x0FFF + V[0]);
}

void Chip8::SET_VX_RANDOM() // 0xCXNN: Set VX to the result of AND on a random number AND NN.
{
        unsigned char rnd = (unsigned char) rand();

        V[(opcode & 0x0F00) >> 8] = (rnd & (opcode & 0x00FF));
        pc += 2;
}

void Chip8::DRAW() // 0xDXYN: Draw sprite at VX, VY with width 8 pixels and height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
{
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; ++yline)
        {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; ++xline)
                {
                        if((pixel & (0x80 >> xline)) != 0)
                        {
                                if(gfx[x + xline + ((y + yline) * 64)] == 1)
                                {
                                        V[0xF] = 1;
                                }
                                gfx[x + xline + ((y + yline) * 64)] ^= 1;
                        }
                }
        }

        drawFlag = true;
        pc += 2;
}

void Chip8::SKIP_IF_KEY()
{
	(this->*Chip8SkipIfKey[opcode && 0x00FF])(); 
}

void Chip8::SKIP_IF_KEY_NOT_VX() // 0xEXA1: Skip next instruction if key stored in VX isn't pressed
{
        if(key[V[(opcode & 0x0F00) >> 8]] == 0)
        {
                pc += 4;
        }else
        {
                pc += 2;
        }
}

void Chip8::SKIP_IF_KEY_VX() // 0xEX9E: Skip next instruction if key stored in VX is pressed
{
        if(key[V[(opcode & 0x0F00) >> 8]] != 0)
        {
                pc += 4;
        }else
        {
                pc += 2;
        }
}

void Chip8::OTHERS()
{
	
                switch(opcode & 0x00FF)
                {
                case 0x0015: // 0xFX15: Set delay timer to VX
                {
                        delay_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                }
                break;

                case 0x0007: // 0xFX07: Set VX to value of delay timer
                {
                        V[(opcode & 0x0F00) >> 8] = delay_timer;
                        pc += 2;
                }
                break;

                case 0x000A: // 0xFX0A: Wait for key press, then store in VX.
                {
                        bool keepGoing = true;
                        while(keepGoing)
                        {
                        for(int i = 0; i < 16; ++i)
                        {
                                if(key[i] != sf::Keyboard::isKeyPressed(keyNum[i]))
                                {
                                        key[i] = sf::Keyboard::isKeyPressed(keyNum[i]);
                                        V[opcode & 0x0F00 >> 8] = key[i];
                                        keepGoing = false;
                                }
                        }
                        }
                        pc += 2;
                }
                break;

                case 0x0018: // 0xFX18: Set the sound timer to VX.
                {
                        sound_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                }
                break;

                case 0x001E: // 0xFX1E: Add VX to I-register
                {
                        if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
                                V[0xF] = 1;
                        else
                                V[0xF] = 0;
                        I += V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                }
                break;

                case 0x0029: // 0xFX29: Set I to the location of the sprite for the character in VX. Character 0-F (in hex) are represented by a 4x5 font.
                {
                        I = V[(opcode & 0x0F00) >> 8] * 5;
                        pc += 2;
                }
                break;

                case 0x0033: // 0xFX33: Store BCD representation of VX, with the most significant of three digits at the address in I, the middle digit at I + 1, and the least significant digit at I + 2.
                {
                        //This is some weird stuff, I didn't write this myself
                        memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                        memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                        memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                        pc += 2;
                }
                break;

                case 0x0055: // 0xFX55: Store V0 to VX in memory starting at address I.
                {
                        for(int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        {
                                memory[I + i] = V[i];
                        }
                        // On the original interpreter, when the operation is done, I = I + X + 1.
                        I += ((opcode & 0x0F00) >> 8) + 1;
                        pc += 2;
                }
                break;
                case 0x0065: // 0xFX65: Fills V0 to VX with values from memory starting at address I.
                {
                        for(int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        {
                                V[i] = memory[I + i];
                        }

                        I += ((opcode & 0x0F00) >> 8) + 1;
                        pc += 2;
                }
                break;
                default:
                {
                        cout << "Unkown opcode: " << hex << opcode << endl;
                        isOn = false;
                }
                break;
                }
}

void Chip8::emulateCycle()
{ // Emulates one cycle

        // Fetch instruction
        fetch();

        // Decode and execute instruction
        execute();

        // Update timers and keyboard
        update();
}

bool Chip8::getChipState()
{ // Returns whether machine is supposed to be on
        return isOn;
}

bool Chip8::getDrawFlag()
{ // Returns true if a change in VRAM is made
        return drawFlag;
}

void Chip8::setDrawFlag(bool flag)
{ // Set the drawflag to have update in graphical output
        drawFlag = flag;
}

void Chip8::loadROM(const string& fileName)
{ // Loads rom from directory provided
        file.open(fileName, ios::binary); // Open file in binary mode to read ROM
        file.seekg(0, ios::beg); // Start from beginning of file
        unsigned char* byte;
        char* readP;
        char read = 'F';
        for(int i = 512; i < sizeof(memory)/sizeof(*memory); ++i)
        { // Go through memory starting at 0x200 and fill out program
                file.get(read); // Read one signed character
                readP = &read; // Now have a pointer point to this character
                byte = reinterpret_cast<unsigned char*>(readP); // Finally we tell the compiler to simply reinterpret this signed pointer char to an unsigned one instead
                memory[i] = *byte; // Fill memory with instruction

                if(file.eof())
                { // End of file flag set, no need to continue iterating
                        return;
                }
        }
}

void Chip8::shutdown()
{
  isOn = false;
}

void Chip8::printDebug()
{
	cout << "OP: " << hex << opcode << endl;
	cout << "I: " << hex << I << endl;
	cout << "PC: " << hex << pc << endl;
	cout << "SP: " << hex << sp << endl << endl;
}
