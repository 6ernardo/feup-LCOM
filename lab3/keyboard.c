#include "keyboard.h"

#define KBC_IN_CMD 0x64
#define KBC_OUT_CMD 0x60
#define KBC_IN_ARG 0x60

#define READ_CMD 0x20
#define WRITE_CMD 0x60

int keyboard_hook_id = 1;
uint8_t scancode;

void (kbc_ih)(){
    read_KBC_output(KBC_OUTBUFFER, &scancode);
}

int (keyboard_subscribe_int)(uint8_t *bit_no){
    *bit_no = BIT(keyboard_hook_id);

    return sys_irqsetpolicy(KEYBOARD_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &keyboard_hook_id);
}

int (keyboard_unsubscribe_int)(){
    return sys_irqrmpolicy(&keyboard_hook_id);
}

int keyboard_restore(){
    uint8_t command;

    if(write_KBC_command(KBC_IN_CMD, READ_CMD) != 0) return 1; // avisa KBC que quer ler command byte
    if(read_KBC_output(KBC_OUT_CMD, &command) != 0) return 1; // Lê command byte para preservar definições

    command = command | BIT(0); // enable interrupt from Keyboard

    if(write_KBC_command(KBC_IN_CMD, WRITE_CMD) != 0) return 1; // avisa KBC que vou escrever command byte
    if(write_KBC_command(KBC_IN_ARG, command) != 0) return 1; // escreve command byte

    return 0;
}
