#define FCY 32000000
//main juste pour allumer des leds

#include <libpic30.h>

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include "config.h"

/*
 * 
 */
 int main(int argc, char** argv) {
     TRISA = 0;
     
     int i;
     for ( i=0; i<10; i++){
        LATA = 0xFF;
        __delay_ms(10);
        LATA = 0x00;
        __delay_ms(10);
     }
     
     int j;
     for ( j=0; j<10; j++){
        LATA = 0b10101010;
        __delay_ms(10);
        LATA = 0b01010101;
        __delay_ms(10);
     }
     
     int k = 0;
     
     while(k<5){
        LATA = 0b10000000;
        __delay_ms(5);
        LATA = 0b01000000;
        __delay_ms(5);
        LATA = 0b00100000;
        __delay_ms(5);
        LATA = 0b00010000;
        __delay_ms(5);
        LATA = 0b00001000;
        __delay_ms(5);
        LATA = 0b00000100;
        __delay_ms(5);
        LATA = 0b00000010;
        __delay_ms(5);
        LATA = 0b00000001;
        __delay_ms(5);
        LATA = 0b11111111;
        __delay_ms(100);
        k++;
     }
     
     while(1){
         LATA = 0x00;
     };
    
    return (EXIT_SUCCESS);
}

