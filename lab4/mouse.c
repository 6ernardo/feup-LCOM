#include "mouse.h"

int mouse_hook_id = 2;
uint8_t byte = 0;
uint8_t array[3];
uint8_t scancode;

struct packet pp;

void (mouse_ih)(){
    read_KBC_output(KBC_OUTBUFFER, &scancode, 1);
}

int (mouse_subscribe_int)(uint8_t *bit_no){
    *bit_no= BIT(mouse_hook_id);

    return sys_irqsetpolicy(MOUSE_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &mouse_hook_id);
}

int (mouse_unsubscribe_int)(){
    return sys_irqrmpolicy(&mouse_hook_id);
}

void (mouse_assemble_array)(){
    if(byte == 0 && (scancode & BIT(3))) { //Byte 1
        array[byte]=scancode;
        byte++;
    }
    else if(byte>0){ //Byte 2 or 3
        array[byte]=scancode;
        byte++;
    }
}

void (mouse_assemble_packet)(){
    
    for(int i=0; i<3; i++){
        pp.bytes[i]=array[i];
    }

    pp.rb = array[0] & BIT(1);
    pp.mb = array[0] & BIT(2);
    pp.lb = array[0] & BIT(0);
    pp.delta_x = (array[0] & BIT(5)) ? (0xFF00 | array[1]) : array[1]; // verifica sinal do deslocamento (positivo ou negativo) 
    pp.delta_y = (array[0] & BIT(4)) ? (0xFF00 | array[2]) : array[2]; // e ajusta o valor em conformidade
    pp.x_ov = array[0] & BIT(6);
    pp.y_ov = array[0] & BIT(7);
}

int (mouse_write)(uint8_t command){
    uint8_t response;

    for(int i=0; i<10; i++){
        if(write_KBC_command(0x64, 0xD4) != 0) return 1;
        if(write_KBC_command(0x60, command) != 0) return 1;
        tickdelay(micros_to_ticks(DELAY_US));
        if(util_sys_inb(0x60, &response) != 0) return 1;
        if(response == 0xFA) return 1; //success
    }

    return 0;
}