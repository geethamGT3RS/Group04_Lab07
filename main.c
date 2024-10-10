#include <stdint.h>
#include "tm4c123gh6pm.h"

#define RED_LED     0x02    // PF1
#define BLUE_LED    0x04    // PF2
#define GREEN_LED   0x08    // PF3

#define SW1         0x10    // PF4
#define SW2         0x01    // PF0

void delayMs(int n);

void initGPIO(void) {
    // Enable the clock for Port F
    SYSCTL_RCGCGPIO_R |= 0x20;
    while ((SYSCTL_PRGPIO_R & 0x20) == 0) {} // Allow clock to stabilize

    // Unlock Port F for PF0 (SW2) to modify its settings
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R = 0x1F;  // Allow changes to PF4-0

    // Configure PF1, PF2, PF3 as outputs for the LEDs
    GPIO_PORTF_DIR_R |= RED_LED | BLUE_LED | GREEN_LED;
    GPIO_PORTF_DEN_R |= RED_LED | BLUE_LED | GREEN_LED;

    // Configure PF4 (SW1) and PF0 (SW2) as inputs with pull-up resistors
    GPIO_PORTF_DIR_R &= ~(SW1 | SW2);
    GPIO_PORTF_DEN_R |= SW1 | SW2;
    GPIO_PORTF_PUR_R |= SW1 | SW2;
}

void initUART1(void) {
    // Enable the clock for UART1 and Port B
    SYSCTL_RCGCUART_R |= 0x02;
    SYSCTL_RCGCGPIO_R |= 0x02;
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {} // Allow clock to stabilize

    // Configure PB0 (RX) and PB1 (TX) for UART1
    GPIO_PORTB_AFSEL_R |= 0x03;   // Enable alt function on PB0, PB1
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) | 0x00000011; // Set PB0, PB1 as UART1
    GPIO_PORTB_DEN_R |= 0x03;     // Digital enable for PB0, PB1

    // Configure UART1 for 9600 baud, 8-bit data, odd parity, 1 stop bit
    UART1_CTL_R &= ~0x01;         // Disable UART1
    UART1_IBRD_R = 104;           // Integer part of baud rate divisor
    UART1_FBRD_R = 11;            // Fractional part of baud rate divisor
    UART1_LCRH_R = 0x7A;          // 8-bit, odd parity, 1 stop bit
    UART1_CTL_R |= 0x301;         // Enable UART1, TX, and RX
}

void UART1_Send(uint8_t data) {
    while (UART1_FR_R & 0x20) {}
    UART1_DR_R = data;
}

uint8_t UART1_Receive(void) {
    while (UART1_FR_R & 0x10) {}
    if (UART1_RSR_R & 0x0F) {
        UART1_ECR_R = 0x0F;
        return 0xFF;
    }
    return (uint8_t)(UART1_DR_R & 0xFF);
}

void setLED(uint8_t color) {
    GPIO_PORTF_DATA_R &= ~(RED_LED | BLUE_LED | GREEN_LED);
    GPIO_PORTF_DATA_R |= color;
}

int main(void) {
    initGPIO();
    initUART1();

    while (1) {
        if ((GPIO_PORTF_DATA_R & SW1) == 0) {
            UART1_Send(0xF0);
        } else if ((GPIO_PORTF_DATA_R & SW2) == 0) {
            UART1_Send(0xAA);
        }
        uint8_t receivedData = UART1_Receive();

        if (receivedData == 0xF0) {
            setLED(BLUE_LED);
        } else if (receivedData == 0xAA) {
            setLED(GREEN_LED);
        } else if (receivedData == 0xFF) {
            setLED(RED_LED);
        }

        delayMs(100);
    }
}

void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 3180; j++) {}
    }
}
