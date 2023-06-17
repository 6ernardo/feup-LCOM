#include <lcom/lcf.h>

#include "kbc.h"

//lê status register
int read_KBC_status(uint8_t *status){
    return util_sys_inb(KBC_STATUS_REG, status);
}

//lê out buffer
int read_KBC_output(uint8_t port, uint8_t *output, uint8_t mouse){
    uint8_t status;

    //vai tentar ler status e output buffer 10 vezes, em intervalos de 20ms (delay_us)
    for(int attempts = 10; attempts > 0; attempts--){
        if(read_KBC_status(&status) != 0) return 1;

        if((status & BIT(0)) != 0){ //out buffer está cheio, pode ler
            if(util_sys_inb(port, output) != 0) return 1; //ler de port

            if((status & BIT(7)) != 0) return 1; //parity error
            if((status & BIT(6)) != 0) return 1; //timeout error

            if(mouse && !(status & BIT(5))) return 1; //esperava mouse
            if(!mouse && (status & BIT(5))) return 1; //esperava keyboard

            return 0;
        }

        tickdelay(micros_to_ticks(DELAY_US));
    }

    return 1;
}

int write_KBC_command(uint8_t port, uint8_t command){
    uint8_t status;

    for(int attempts = 10; attempts > 0; attempts--){
        if(read_KBC_status(&status) != 0) return 1; //lê estado para verificar se input buffer está cheio

        if(!(status & BIT(1))){ //se não está cheio
            if(sys_outb(port, command) != 0) return 1; //escreve comando na porta
            
            return 0;
        }

        tickdelay(micros_to_ticks(DELAY_US));
    }

    return 1;
}
