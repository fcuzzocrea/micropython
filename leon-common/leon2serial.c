#include <bsp.h>
#include <leon.h>

void output_a_character_to_leon2_port0(unsigned char c)
{
    /* wait for UART to be free, then write */
    while(!(LEON_REG.UART_Status_1 & LEON_REG_UART_STATUS_THE))
        ;
    LEON_REG.UART_Channel_1 = (unsigned int) c;
}
