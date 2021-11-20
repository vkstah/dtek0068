/*
 * File:   main.c
 * Author: Vili Ståhlberg
 * Description: W04E01 exercise program to play the Dino-game available
 * in the Chrome browser.
 *
 * Created on 20 November 2021, 16:23
 */


#include <avr/io.h>
#include <util/delay.h>

uint16_t ldr_read()
{
    // Set reference voltage (2.5V)
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc;
    // Switch from default channel to AIN8 (PE0)
    ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
    // No pull-up or invert, disable input buffer
    PORTE.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    // Start the conversion
    ADC0.COMMAND = ADC_STCONV_bm;
    // Wait for the bit to get set
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
    {
        ;
    }
    // Return the result
    return ADC0.RES;
}

uint16_t trimpot_read()
{
    // Set reference voltage (VDD)
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
    // Switch from default channel to AIN14 (PF4)
    ADC0.MUXPOS = ADC_MUXPOS_AIN14_gc;
    // No pull-up or invert, disable input buffer
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // Enable ADC
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    // Start the conversion
    ADC0.COMMAND = ADC_STCONV_bm;
    // Wait for the bit to get set)
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
    {
        ;
    }
    // Return the result
    return ADC0.RES;
}

int main(void) {
    /*
     * Array to display digits (0-9) and letter A to signal value 10, 
     * 11th index is a blank display
     */
    uint16_t digits[] =
    {
        0b00111111, 0b00000110,
        0b01011011, 0b01001111,
        0b01100110, 0b01101101,
        0b01111101, 0b00000111,
        0b01111111, 0b01100111,
        0b01110111, 0b00000000
    };
    // Set port C as output
    PORTC.DIRSET = 0xFF;
    // Set PE0 as input (LDR)
    PORTE.DIRCLR = PIN0_bm;
    // Set PF4 as input (POT)
    PORTF.DIRCLR = PIN4_bm;
    // Variable for storing LDR ADC result
    uint16_t ldr_result;
    // Variable for storing trimpot ADC result;
    uint16_t trimpot_result;
    
    // Route TCA0 PWM waveform to PORTB
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc;
    // Set PB2 as digital output
    PORTB.DIRSET = PIN2_bm;
    // Set TCA0 prescaler value to 16
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
    // Set single-slope PWM generation mode
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    // Set PWM period to 20ms
    TCA0.SINGLE.PERBUF = 0x1046;
    // Set initial arm position to neutral (0 degrees)
    TCA0.SINGLE.CMP2BUF = 0x0138;
    // Enable compare channel 2
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
    // Enable TCA0
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    
    while (1)
    {
        // Set the arm to a neutral position
        TCA0.SINGLE.CMP2BUF = 0x0138;
        // Read the ADC result for LDR
        ldr_result = ldr_read();
        // Read the ADC result for trimpot
        trimpot_result = trimpot_read();
        // Display the trimpot reading's value divided by 100
        PORTC.OUT = digits[trimpot_result/100];
        // If the LDR's value is larger (indicating a cactus)
        if (trimpot_result < ldr_result)
        {
            // Move the arm to press the spacebar
            TCA0.SINGLE.CMP2BUF = 0x00D0;
            // Wait 100ms
            _delay_ms(100);
        }
    }
}
