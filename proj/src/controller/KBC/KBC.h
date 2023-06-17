#ifndef _LCOM_KBC_H_
#define _LCOM_KBC_H_

#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include "controller/mouse/i8042.h"

int (read_KBC_status)(uint8_t *status);

int (write_KBC_command)(uint8_t port, uint8_t commandByte);

int (read_KBC_output)(uint8_t port, uint8_t *output, uint8_t mouse);

#endif
