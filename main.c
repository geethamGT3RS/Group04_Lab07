#include <stdint.h>
#include "tm4c123gh6pm.h"

void initUART1(void) {
    // Enable clock for UART1 and Port F
    SYSCTL_RCGCUART_R |= (1 << 1);
    SYSCTL_RCGCGPIO_R |= (1 << 1);

    // Wait for peripherals to be ready
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}

    // Configure Port B pins 0 and 1 for UART1
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) | 0x00000011;
    GPIO_PORTB_DEN_R |= 0x03;

    // Configure UART1
    UART1_CTL_R &= ~0x01;             // Disable UART1 during setup
    UART1_IBRD_R = 104;               // Integer portion for baud rate (9600 baud, 16 MHz clock)
    UART1_FBRD_R = 11;                // Fractional portion for baud rate
    UART1_LCRH_R = 0x70;              // 8-bit, no parity, 1-stop bit
    UART1_LCRH_R |= (1 << 7);         // Enable odd parity
    UART1_CTL_R = 0x301;
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= 0x11;
    GPIO_PORTF_DIR_R &= ~0x11;
    GPIO_PORTF_DEN_R |= 0x11;
    GPIO_PORTF_PUR_R |= 0x11;
}

void UART1_Transmit(uint8_t data) {
    while ((UART1_FR_R & 0x20) != 0);
    UART1_DR_R = data;
}

void checkSwitches(void) {
    if ((GPIO_PORTF_DATA_R & 0x10) == 0) {
        UART1_Transmit(0xF0);
    }
    if ((GPIO_PORTF_DATA_R & 0x01) == 0) {
        UART1_Transmit(0xAA);
    }
}

int main(void) {
    initUART1();

    while (1) {
        checkSwitches();
    }
}
