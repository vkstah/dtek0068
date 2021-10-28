/*
 * File:   main.c
 * Author: dtek
 *
 * Created on 28 October 2021, 19:13
 */


#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    
    PORTF.DIR |= PIN5_bm;
    PORTF.DIR &= ~PIN6_bm;
    PORTF.PIN6CTRL |= PORT_PULLUPEN_bm;
    
    while (1) {
        if (PORTF.IN & PIN6_bm) {
            PORTF.OUT |= PIN5_bm;
        }
        else {
            PORTF.OUT &= ~PIN5_bm;
        }
    }
}
