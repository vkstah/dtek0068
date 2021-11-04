/*
 * File:   main.c
 * Author: Vili Ståhlberg
 * Description: The program will simulate a bomb counting down from 9. In order to defuse it,
 * you must "cut" the grounded wire connected to PA4.
 *
 * Created on 04 November 2021, 21:41
 */

#define F_CPU 3333333

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Global function used to define whether the program is counting down or not
volatile uint8_t g_running = 1;

int main(void)
{
    // An array to display the digits (0 to 9), 10th being the blank digit
    uint8_t digits[] =
    {
        0b00111111, 0b00000110,
        0b01011011, 0b01001111,
        0b01100110, 0b01101101,
        0b01111101, 0b00000111,
        0b01111111, 0b01100111,
        0b00000000
    };
    // Set PORTC as output
    PORTC.DIRSET = 0xFF;
    // Set PORTA PIN4 as input
    PORTA.DIRCLR = PIN4_bm;
    // Trigger interrupts
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    // Index is the actual numerical value displayed
    // index = 9 because we start counting down from 9
    uint8_t index = 9;
    
    while (1)
    {
        // If global variable g_running equals 1, the program is counting down
        if (g_running == 1)
        {
            // If the index is 0, we have counted down and the bomb has exploded
            // When this is true, we keep blinking zero
            if (index == 0)
            {
                // Set output to the corresponding digit
                PORTC.OUT = digits[index];
                _delay_ms(1000);
                // Set output to the 10th index of the array, which is the blank digit
                PORTC.OUT = digits[10];
                _delay_ms(1000);
            }
            else
            {
                // Disable interrupts
                cli();
                // Set output to the corresponding digit
                PORTC.OUT = digits[index];
                // Enable interrupts
                sei();
                _delay_ms(1000);
                // Index goes down in decrements of one
                index--;
            }
        }
        // If g_running is not 1, it is not counting down
        else
        {
            // Continue to display the digit even though the countdown is over
            PORTC.OUT = digits[index];
        }
    }
}

// ISR function to handle the interrupt
ISR(PORTA_PORT_vect)
{
    // Clear interrupt flag (0xFF = 1)
    PORTA.INTFLAGS = 0xFF;
    // Set global variable g_running to 0, which stops the countdown
    g_running = 0;
}