#ifndef KEYMAP_H
#define KEYMAP_H

#include <SDL.h>

// Mapping for the CHIP-8 hex keyboard

/*
| 1 | 2 | 3 | C |   | 1 | 2 | 3 | 4 |
| 4 | 5 | 6 | D |   | Q | W | E | R |
| 7 | 8 | 9 | E |   | A | S | D | F |
| A | 0 | B | F |   | Z | X | C | V |
*/

namespace Keymap {
	// Ordered so index == keyvalue
	uint8_t keymap[16] = {
		SDLK_x, SDLK_1, SDLK_2, SDLK_3,
		SDLK_q, SDLK_w, SDLK_e, SDLK_a,
		SDLK_s, SDLK_d, SDLK_z, SDLK_c,
		SDLK_4, SDLK_r, SDLK_f, SDLK_v
	};
}

#endif // KEYMAP_H
