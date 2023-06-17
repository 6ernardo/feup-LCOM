#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <lcom/lcf.h>

#include "kbc.h"

#define KEYBOARD_IRQ 1
#define KBC_STATUS_REG 0x64
#define KBC_OUTBUFFER 0x60

void (kbc_ih)();

int (keyboard_subscribe_int)(uint8_t *bit_no);

int (keyboard_unsubscribe_int)();

int keyboard_restore();

#endif /* _KEYBOARD_H_ */
