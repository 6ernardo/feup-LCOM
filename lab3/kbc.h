#ifndef _KBC_H_
#define _KBC_H_

#include <lcom/lcf.h>

#include "keyboard.h"

#define KBC_STATUS_REG 0x64
#define DELAY_US 20000

int read_KBC_status(uint8_t *status);

int read_KBC_output(uint8_t port, uint8_t *output);

int write_KBC_command(uint8_t port, uint8_t command);

#endif /* _KBC_H_ */
