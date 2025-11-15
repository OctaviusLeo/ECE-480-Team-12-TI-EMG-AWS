#ifndef MENU_H
#define MENU_H
#include <stdint.h>
#include <stdbool.h>

/* Start the title/menu loop. */
void menu_start(void);

/* Draw one frame. Return true when a mode was chosen; writes MODE_* to out_mode. */
bool menu_tick(uint8_t *out_mode);

#endif
