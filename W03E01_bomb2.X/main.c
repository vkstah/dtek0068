/*
 * File:   main.c
 * Author: Vili Ståhlberg
 * Description: Upgraded version of the W02E01 bomb program. Moved away from using delays
 * to RTC and sleep.
 *
 * Created on 13 November 2021, 13:55
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/sleep.h>


// Global variable used to define whether the program is counting down or not
volatile uint8_t g_running = 1;

// Global variable to allow the superloop to count
volatile uint8_t g_clockticks = 9;

void rtc_init(void)
{
    uint8_t temp;
    //Disable oscillator
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    // Wait for the clock to be released (0 = unstable, unused)
    while (CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm);
    
    // Select external crystal (SEL = 0)
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_SEL_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    
    // Enable oscillator
    temp = CLKCTRL.XOSC32KCTRLA;
    temp |= CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    // Wait for the clock to stabilize
    while (RTC.STATUS > 0);
    
    // Configure RTC module
    // Select 32.768 kHz external oscillator
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    // Enable Periodic Interrupt
    RTC.PITINTCTRL = RTC_PI_bm;
    // Set period to 4096 cycles (1/8 second) and enable PIT function
    RTC.PITCTRLA = RTC_PERIOD_CYC4096_gc | RTC_PITEN_bm;
}

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
    // Variable used to define the actual digit displayed
    uint8_t display;
    // Set PORTC as output
    PORTC.DIRSET = 0xFF;
    // Set PORTA PIN4 as input
    PORTA.DIRCLR = PIN4_bm;
    // Trigger interrupts
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;

    // Initialize RTC
    rtc_init();
    // Set sleep mode to IDLE
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc);
    // Enable interrupts
    sei();
    
    while (1)
    {
        // If global variable g_running equals 1, the program is counting down
        if (g_running == 1)
        {
            // If g_clockticks equals 0, we start to toggle PF5 and the digit display
            if (g_clockticks == 0)
            {
                PORTF.DIRSET = PIN5_bm;
                PORTF.OUTTGL = PIN5_bm;
            }
            display = g_clockticks;
            PORTC.OUT = digits[display];
        }
        // If g_running is not 1, it is not counting down
        else
        {
            // Continue to display the digit even though the countdown is over
            PORTC.OUT = digits[display];
        }
        // Enter sleep mode
        sleep_mode();
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

ISR(RTC_PIT_vect)
{
    // Keep count of the total amount of interrupts via a local static variable
    static uint8_t interrupts = 0;
    // Clear interrupt flag
    RTC.PITINTFLAGS = RTC_PI_bm;
    // Another interrupt has taken place
    interrupts++;
    // If the clock has reached 0
    if (g_clockticks == 0)
    {
        // Do nothing as the clock has reached 0
    }
    // If the clock hasn't reached 0
    else
    {
        // If the amount of interrupts can be divided by 8, 
        // we've counted a second (and the timer goes down by 1 second)
        if ((interrupts % 8) == 0)
        {
            g_clockticks--;
        }
    }
}
