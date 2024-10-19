#include "tm4c123gh6pm_header.h"
#include <stdint.h>
#include <stdlib.h>
volatile int check = 0;
volatile int start = 0;
void Delay(unsigned long counter);
int UART5_Receiver(void);
void UART5_Transmitter(int data);
void GPIO_INT_HANDLER(void)
{
    int sw1, sw2;
    int temp_x = 500;
    while (temp_x > 0)
    {
        temp_x = temp_x - 1;
    }
    sw1 = GPIO_PORTF_DATA_R & 0x10;
    sw2 = GPIO_PORTF_DATA_R & 0x01;
    if (sw1 == 0 && sw2 != 0)
    {
        UART5_Transmitter(0xF0);
    }
    if (sw1 != 0 && sw2 == 0)
    {
        UART5_Transmitter(0xAA);
    }
    GPIO_PORTF_ICR_R = 0x11;
}

int main(void)
{
    SYSCTL_RCGCUART_R |= 0x20; /* enable clock to UART5 */
    SYSCTL_RCGCGPIO_R |= 0x10; /* enable clock to PORTE for PE4/Rx and PE5/Tx */
    Delay(1);

    /* UART5 initialization */
    UART5_CTL_R = 0;     /* disable UART5 */
    UART5_IBRD_R = 104;  /* for 9600 baud rate, integer part = 104 */
    UART5_FBRD_R = 11;   /* for 9600 baud rate, fractional part = 11 */
    UART5_CC_R = 0;      /* use system clock */
    UART5_LCRH_R = 0x62; /* data length 8-bit, no parity, no FIFO */
    UART5_CTL_R = 0x301; /* enable UART5, Rx and Tx */

    /* UART5 Tx (PE5) and Rx (PE4) configuration */
    GPIO_PORTE_DEN_R = 0x30;        /* enable digital for PE4 and PE5 */
    GPIO_PORTE_AFSEL_R = 0x30;      /* enable alternate function for PE4 and PE5 */
    GPIO_PORTE_AMSEL_R = 0;         /* disable analog function for PE4 and PE5 */
    GPIO_PORTE_PCTL_R = 0x00110000; /* configure PE4 and PE5 for UART5 */

    SYSCTL_RCGC2_R |= 0x00000020;   /* enable clock to GPIOF */
    GPIO_PORTF_LOCK_R = 0x4C4F434B; /* unlock commit register */
    GPIO_PORTF_CR_R = 0x1F;         /* make PORTF0 configurable */
    GPIO_PORTF_DEN_R = 0x1F;        /* set PORTF pins 4 pin */
    GPIO_PORTF_DIR_R = 0x0E;        /* set PORTF4 pin as input user switch pin */
    GPIO_PORTF_PUR_R = 0x11;        /* PORTF4 is pulled up */
    GPIO_PORTF_IS_R = 0x00;         // Edge-sensitive interrupts
    GPIO_PORTF_IBE_R = 0x00;        // Interrupt on single edge
    GPIO_PORTF_IEV_R = 0x00;        // Falling edge triggers interrupt
    GPIO_PORTF_IM_R = 0x11;         // Unmask interrupts for PF4 and PF0
    NVIC_EN0_R = 0x40000000;        // Enable IRQ30 (GPIO Port F interrupt)
    Delay(1);
    int v = UART5_DR_R;
    while (1)
    {
        int rx;

        rx = UART5_Receiver();

        if ((UART5_RSR_R & (1 << 1)) != 0 && start > 1)
        {
            GPIO_PORTF_DATA_R = 0x02;
        }
        else if (rx == 0xAA)
        {
            GPIO_PORTF_DATA_R = 0x08;
        }
        else if (rx == 0xF0)
        {
            GPIO_PORTF_DATA_R = 0x04;
        }
    }
}

void UART5_Transmitter(int data)
{
    while ((UART5_FR_R & (1 << 5)) != 0)
        ;
    UART5_DR_R = data;
    check = check + 1;
}

int UART5_Receiver(void)
{
    int data;
    while ((UART5_FR_R & (1 << 4)) != 0)
        ;
    data = UART5_DR_R;
    start = start + 1;
    return data;
}
void Delay(unsigned long counter)
{
    unsigned long i = 0;
    for (i = 0; i < counter; i++)
        ;
}
