// Main loop + rendering

#define SDL_MAIN_HANDLED

#include "chip8.h"
#include "keymap.h"

#include <SDL.h>
#include <fstream>


int main(int argc, char** argv)
{
    Chip8::CPU cpu;

    // When running from command line we can pass a rom directly
    if (argc > 1) {
        uint8_t fileBuffer[Chip8::memorySize];
        uint16_t fileSize = 0;

        for (std::ifstream f(argv[1], std::ios::binary); f.good();) {
            fileBuffer[fileSize++] = f.get();
        }

        cpu.LoadProgram(fileBuffer, fileSize);
    } else {
        // TODO: ROM Menu?
        return -1;
    }

	SDL_Init(SDL_INIT_EVERYTHING);
	// Create an SDL screen
	SDL_Window*   window   = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Chip8::screenWidth * 8, Chip8::screenHeight * 8, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture*  texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, Chip8::screenWidth, Chip8::screenHeight);

	uint8_t screenData[Chip8::screenSizeBytes];
	uint32_t screenTexture[Chip8::screenSizePixels];

	bool quit = false;
	// TODO: Map keyboard
	while (!quit) {
		// Run CPU
		cpu.ExecuteStep();

		// Receive input events
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent)) {
			switch (sdlEvent.type) {
			case SDL_QUIT: {
				quit = true;
			} break;
			case SDL_KEYDOWN: {
				if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				} else {
					for (int i = 0; i < 16; ++i) {
						if (sdlEvent.key.keysym.sym == Keymap::keymap[i]) {
							cpu.SetKeyDown(i);
						}
					}
				}
			} break;
			case SDL_KEYUP: {
				for (int i = 0; i < 16; ++i) {
					if (sdlEvent.key.keysym.sym == Keymap::keymap[i]) {
						cpu.SetKeyUp(i);
					}
				}
			}
			default: break;
			}
		}
		
		if (cpu.GetDrawFlag()) {
			// Get the screen data from memory
			cpu.GetScreen(screenData, Chip8::screenSizeBytes);
			// Convert screen data to ARGB8888 texture
			uint32_t white = 0xFFFFFFFF;
			for (int pix = 0, byte = 0; pix < Chip8::screenSizePixels; ++pix) {
				if (screenData[pix / 8] & (0x80 >> (pix % 8))) {
					screenTexture[pix] = white;
				}
				else {
					screenTexture[pix] = 0;
				}
			}
			// Render
			SDL_UpdateTexture(texture, nullptr, screenTexture, Chip8::screenWidth * 4);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
			SDL_RenderPresent(renderer);
			cpu.ClearDrawFlag();
			SDL_Delay(20);
		}
	}

	SDL_Quit();
    return 0;
}
