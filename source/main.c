#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#ifdef _VITA
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#elif defined _3DS
#include <3ds.h>
#include <sf2d.h>
#endif
uint8_t memory[4096]; // Chip8 Memory (4KB)
uint16_t pc = 0x200; // Program Counter value
unsigned char gfx[64 * 32]; // Gfx buffer
bool drawFlag = false; // flag to determine whether to draw the Gfx in the current loop.
uint8_t V[16]; // V registers
uint16_t stack[16]; // Stack
uint16_t sp; // Stack Pointer
uint16_t I; // Special 'I' register
uint8_t key[16]; // Keypad
uint8_t dt; // Delay Timer
uint16_t opcode; // Opcode value

long rom_size; // Size of chosen Chip8 ROM

void LoadChip8Rom(const char* filePath) { // Loads the chip8 ROM from a specific file path.
	// Clear memory
	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	} // Keep looping until every value in the memory array is set to 0.
	
	// Open chip8 ROM file as read-only.
	FILE* chip8Rom = fopen(filePath, "rb");
	if (chip8Rom == NULL) {
		// printf("Failed to open ROM.");
		return;
	}
	
	// Get file size of the ROM.
	fseek(chip8Rom, 0, SEEK_END);
	rom_size = ftell(chip8Rom);
	rewind(chip8Rom);
	
	// Allocate buffer to store ROM
	char* rom_buffer = (char*) malloc(sizeof(char) * rom_size);
	if (rom_buffer == NULL) {
		// printf("Failed to allocate memory for ROM.");
		return;
	}
	
	// Copy ROM into buffer
	size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, chip8Rom);
	if (result != rom_size) {
		// printf("Failed to copy ROM into buffer.");
		return;
	}
	
	// Copy buffer into memory
	if ((4096-512) > rom_size){
		for (int i = 0; i < rom_size; ++i) {
			memory[i+512] = (uint8_t)rom_buffer[i]; // Load into memory at 0x200
		}
	} else {
		// printf("ROM too large to fit into memory.");
	}
	
	// Increase rom_size variable by 0x200
	rom_size += 0x200;
}

void Chip8EmulationLoop() {
	// Opcode is 2 bytes long
	opcode = memory[pc] << 8 | memory[pc + 1];
	
	switch ( opcode & 0xF000 ) {
		case 0x000:
			switch (opcode & 0x00FF) { // Use the last 2 hex values of the opcode to search for the appropriate function. Same thing is also done for 8XXX, EXXX, and FXXX opcodes.
				case 0x00E0: // 00E0 - CLS
					for (int i = 0; i < 2048; ++i) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
					pc+=2;
					break;
				case 0x00EE: // 00EE - RET
					--sp;
					pc = stack[sp];
					pc += 2;
					break;
				default: break;
			} break;
		case 0x1000: // 1nnn - JP
			pc = opcode & 0x0FFF;
			break;
		case 0x2000: // 2nnn - CALL
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;
		case 0x3000: // 3xkk - SE
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		case 0x4000: // 4xkk - SNE
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
			break;
		case 0x5000: // 5xy0 - SE
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;
		case 0x6000: // 6xkk - LD
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
			break;
		case 0x7000: // 7xkk - ADD
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
			break;
		case 0x8000:
			switch (opcode & 0x000F) {
				case 0x0000: // 8xy0 - LD
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0001: // 8xy1 - OR
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0002: // 8xy2 - AND
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0003: // 8xy3 - XOR
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0004: // 8xy4 - ADD
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
					pc += 2;
					break;
				case 0x0005: // 8xy5 - SUB
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				case 0x0006: // 8xy6 - SHR
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
					break;
				case 0x0007: // 8xy7 - SUBN
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x000E: // 8xyE - SHL
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
					break;
				default: break;
			} break;
		case 0x9000: // 9xy0 - SNE
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;
		case 0xA000: // Annn - LD
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		case 0xB000: // Bnnn - JP
			pc = (opcode & 0x0FFF) + V[0];
			break;
		case 0xC000: // Cxkk - RND
			V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
			pc += 2;
			break;
		case 0xD000: // Dxyn - DRW
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;
 
			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++) {
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++) {
					if((pixel & (0x80 >> xline)) != 0) {
						if(gfx[(x + xline + ((y + yline) * 64))] == 1)
							V[0xF] = 1;                                 
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
			drawFlag = true;
			pc += 2;
			break;
		}
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E: // Ex9E - SKP
					if (key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc +=  4;
					else
						pc += 2;
					break;
				case 0x00A1: // ExA1 - SKNP
					if (key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc +=  4;
					else
						pc += 2;
					break;
				default: break;
			} break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007: // Fx07 - LD
					V[(opcode & 0x0F00) >> 8] = dt;
					pc += 2;
					break;
				case 0x000A: // Fx0A - LD
				{
					bool key_down = false;
					
					for(int i = 0; i < 16; ++i) {
						if(key[i] != 0) {
							V[(opcode & 0x0F00) >> 8] = i;
							key_down = true;
						}
					}
					if(!key_down)
						return;
					pc += 2;
					break;
				}
				case 0x0015: // Fx15 - LD
					dt = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x0018: // Fx18 - LD
					// printf("LD    ST    %01x\n", (opcode & 0x0F00) >> 8);
					// TODO: Implement sound in some way...
					pc += 2;
					break;
				case 0x001E: // Fx1E - ADD
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				case 0x0029: // Fx29 - LD
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
					break;
				case 0x0033: // Fx33 - LD
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
					pc += 2;
					break;
				case 0x0055: // Fx55 - LD
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
						memory[I + i] = V[i];
					}
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;
				case 0x0065: // Fx65 - LD
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
						V[i] = memory[I + i];
					}
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;
				default: break;
			} break;
		default: break;
	}
	
	// Update delay timer
	if (dt > 0)
		--dt;
}

int main() {
#ifdef _VITA
	SceCtrlData pad, old_pad;
	unsigned int keys_down;
	unsigned int keys_up;
	vita2d_pgf *pgf;

	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	pgf = vita2d_load_default_pgf();

	memset(&pad, 0, sizeof(pad));
	old_pad.buttons = 0;
	LoadChip8Rom("ux0:data/VOX8/ROM");
	for (;;) {
#elif defined _3DS
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	LoadChip8Rom("ROM");
	while (aptMainLoop()) {
#endif
		int gfxchar = 0;
#ifdef _VITA
		sceCtrlPeekBufferPositive(0, &pad, 1);
		keys_down = pad.buttons & ~old_pad.buttons;
		keys_up = ~pad.buttons & old_pad.buttons;
		if (keys_down & SCE_CTRL_START)
			break;
		if (keys_down & SCE_CTRL_SQUARE) key[0] = 1;
		else if (keys_up & SCE_CTRL_SQUARE) key[0] = 0;
		if (keys_down & SCE_CTRL_CROSS) key[1] = 1;
		else if (keys_up & SCE_CTRL_CROSS) key[1] = 0;
		if (keys_down & SCE_CTRL_CIRCLE) key[2] = 1;
		else if (keys_up & SCE_CTRL_CIRCLE) key[2] = 0;
		if (keys_down & SCE_CTRL_TRIANGLE) key[3] = 1;
		else if (keys_up & SCE_CTRL_TRIANGLE) key[3] = 0;
		if (keys_down & SCE_CTRL_LEFT) key[4] = 1;
		else if (keys_up & SCE_CTRL_LEFT) key[4] = 0;
		if (keys_down & SCE_CTRL_DOWN) key[5] = 1;
		else if (keys_up & SCE_CTRL_DOWN) key[5] = 0;
		if (keys_down & SCE_CTRL_RIGHT) key[6] = 1;
		else if (keys_up & SCE_CTRL_RIGHT) key[6] = 0;
		if (keys_down & SCE_CTRL_UP) key[7] = 1;
		else if (keys_up & SCE_CTRL_UP) key[7] = 0;
		if (keys_down & SCE_CTRL_L1) key[8] = 1;
		else if (keys_up & SCE_CTRL_L1) key[8] = 0;
		if (keys_down & SCE_CTRL_R1) key[9] = 1;
		else if (keys_up & SCE_CTRL_R1) key[9] = 0;
#elif defined _3DS
		if (hidKeysDown() & KEY_START)
			break;
		if (hidKeysDown() & KEY_CPAD_LEFT) key[0] = 1;
		else if (hidKeysUp() & KEY_CPAD_LEFT) key[0] = 0;
		if (hidKeysDown() & KEY_CPAD_DOWN) key[1] = 1;
		else if (hidKeysUp() & KEY_CPAD_DOWN) key[1] = 0;
		if (hidKeysDown() & KEY_CPAD_RIGHT) key[2] = 1;
		else if (hidKeysUp() & KEY_CPAD_RIGHT) key[2] = 0;
		if (hidKeysDown() & KEY_CPAD_UP) key[3] = 1;
		else if (hidKeysUp() & KEY_CPAD_UP) key[3] = 0;
		if (hidKeysDown() & KEY_DLEFT) key[4] = 1;
		else if (hidKeysUp() & KEY_DLEFT) key[4] = 0;
		if (hidKeysDown() & KEY_DDOWN) key[5] = 1;
		else if (hidKeysUp() & KEY_DDOWN) key[5] = 0;
		if (hidKeysDown() & KEY_DRIGHT) key[6] = 1;
		else if (hidKeysUp() & KEY_DRIGHT) key[6] = 0;
		if (hidKeysDown() & KEY_DUP) key[7] = 1;
		else if (hidKeysUp() & KEY_DUP) key[7] = 0;
		if (hidKeysDown() & KEY_Y) key[8] = 1;
		else if (hidKeysUp() & KEY_Y) key[8] = 0;
		if (hidKeysDown() & KEY_B) key[9] = 1;
		else if (hidKeysUp() & KEY_B) key[9] = 0;
		if (hidKeysDown() & KEY_A) key[0xA] = 1;
		else if (hidKeysUp() & KEY_A) key[0xA] = 0;
		if (hidKeysDown() & KEY_X) key[0xB] = 1;
		else if (hidKeysUp() & KEY_X) key[0xB] = 0;
		if (hidKeysDown() & KEY_L) key[0xC] = 1;
		else if (hidKeysUp() & KEY_L) key[0xC] = 0;
		if (hidKeysDown() & KEY_R) key[0xD] = 1;
		else if (hidKeysUp() & KEY_R) key[0xD] = 0;
		if (hidKeysDown() & KEY_ZL) key[0xE] = 1;
		else if (hidKeysUp() & KEY_ZL) key[0xE] = 0;
		if (hidKeysDown() & KEY_ZR) key[0xF] = 1;
		else if (hidKeysUp() & KEY_ZR) key[0xF] = 0;
#endif
		Chip8EmulationLoop();

		if (drawFlag) {
			drawFlag = false;
#ifdef _VITA
			vita2d_start_drawing();
#elif defined _3DS
			sf2d_start_frame(GFX_TOP, GFX_LEFT);
#endif
			for (int i = 0; i < 32; i++) {
				for (int j = 0; j < 64; j++) {
					unsigned int colour;
					if(gfx[gfxchar] == 1) {
						colour = RGBA8(255, 255, 255, 255);
					} else {
						colour = RGBA8(0, 0, 0, 255);
					}
#ifdef _VITA
					vita2d_draw_rectangle(j*15, i*17, 15, 17, colour);
#elif defined _3DS
					sf2d_draw_rectangle(j*6, i*7, 6, 7, colour);
#endif
					gfxchar++;
				}
			}
#ifdef _VITA
			vita2d_end_drawing();
			vita2d_swap_buffers();
		}
		
		old_pad = pad;
	}
	vita2d_fini();
	vita2d_free_pgf(pgf);
	sceKernelExitProcess(0);
#elif defined _3DS
			sf2d_end_frame();
			sf2d_swapbuffers();
		}
	}
	sf2d_fini();
#endif
	return 0;
}