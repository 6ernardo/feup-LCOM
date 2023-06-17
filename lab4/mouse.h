#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <lcom/lcf.h>

#include "kbc.h"

#define MOUSE_IRQ 12
#define KBC_OUTBUFFER 0x60

void (mouse_ih)();

int (mouse_subscribe_int)(uint8_t *bit_no);

int (mouse_unsubscribe_int)();

void (mouse_assemble_array)();

void (mouse_assemble_packet)();

int (mouse_write)(uint8_t command);

#endif /* _MOUSE_H_ */