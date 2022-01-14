/* 
 * File:   lcd.c
 * Author: ae063157
 *
 * Created on 16 novembre 2021, 14:35
 */


#define FCY 32000000
#include <libpic30.h>

#include <xc.h>
//#include "lcd.h"
#include "rtcc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libpic30.h>
#include <stdint.h>
#include <string.h>
#include "rtcc.h"

#define LCD_EN PORTDbits.RD4
#define LCD_RS PORTBbits.RB15

//-------RTCC DATE&Time

static RTCC_DATETIME time;
static uint8_t RTCC_DecToBCD (uint8_t value);
static uint8_t RTCC_BCDToDec (uint8_t value);

void RTCC_BuildTimeGet(RTCC_DATETIME *dateTime)
{
    uint8_t weekday;
    uint8_t month;
    uint8_t y;
    uint8_t dateTable[] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

    dateTime->second =  (((__TIME__[6]) & 0x0f) << 4) | ((__TIME__[7]) & 0x0f);
    dateTime->minute =  (((__TIME__[3]) & 0x0f) << 4) | ((__TIME__[4]) & 0x0f);
    dateTime->hour = (((__TIME__[0]) & 0x0f) << 4) | ((__TIME__[1]) & 0x0f);
    dateTime->day =  (((__DATE__[4]) & 0x0f) << 4) | ((__DATE__[5]) & 0x0f);
    dateTime->year = (((__DATE__[9]) & 0x0f) << 4) | ((__DATE__[10]) & 0x0f);

    //Set the month
    switch(__DATE__[0])
    {
        case 'J':
            //January, June, or July
            switch(__DATE__[1])
            {
                case 'a':
                    //January
                    month = 0x01;
                    break;
                case 'u':
                    switch(__DATE__[2])
                    {
                        case 'n':
                            //June
                            month = 0x06;
                            break;
                        case 'l':
                            //July
                            month = 0x07;
                            break;
                    }
                    break;
            }
            break;
        case 'F':
            month = 0x02;
            break;
        case 'M':
            //March,May
            switch(__DATE__[2])
            {
                case 'r':
                    //March
                    month = 0x03;
                    break;
                case 'y':
                    //May
                    month = 0x05;
                    break;
            }
            break;
        case 'A':
            //April, August
            switch(__DATE__[1])
            {
                case 'p':
                    //April
                    month = 0x04;
                    break;
                case 'u':
                    //August
                    month = 0x08;
                    break;
            }
            break;
        case 'S':
            month = 0x09;
            break;
        case 'O':
            month = 0x10;
            break;
        case 'N':
            month = 0x11;
            break;
        case 'D':
            month = 0x12;
            break;
    }

    dateTime->month = month;

    // Start with weekday = 6.  This value is valid for this algorithm for this century.
    weekday = 6;
    // y = year
    y = ((dateTime->year >> 4) * 10) + (dateTime->year & 0x0f);
    // Weekday = base day + year + x number of leap days
    weekday += y + (y / 4);
    // If the current year is a leap year but it's not March, subtract 1 from the date
    if (((y % 4) == 0) && (month < 3))
    {
        weekday -= 1;
    }
    // Add an offset based on the current month
    weekday += dateTable[month - 1];
    // Add the current day in the month
    weekday += ((dateTime->day >> 4) * 10) + (dateTime->day & 0x0f);
    weekday = weekday % 7;

    dateTime->weekday = weekday;
    
    if (!dateTime->bcdFormat)
    {
        dateTime->year = RTCC_BCDToDec (dateTime->year);
        dateTime->month = RTCC_BCDToDec (dateTime->month);
        dateTime->day = RTCC_BCDToDec (dateTime->day);
        dateTime->weekday = RTCC_BCDToDec (dateTime->weekday);
        dateTime->hour = RTCC_BCDToDec (dateTime->hour);
        dateTime->minute = RTCC_BCDToDec (dateTime->minute);
        dateTime->second = RTCC_BCDToDec (dateTime->second);
    }
}

void RTCC_Initialize (RTCC_DATETIME * value)
{    
   // Turn on the secondary oscillator
   __builtin_write_OSCCONL(0x02);

   RTCCON1L = 0x0000;
   
    asm volatile("push w7");
    asm volatile("push w8");
    asm volatile("disi #5");
    asm volatile("mov #0x55, w7");
    asm volatile("mov w7, _NVMKEY");
    asm volatile("mov #0xAA, w8");
    asm volatile("mov w8, _NVMKEY");
    asm volatile("bclr _RTCCON1L, #11");
    asm volatile("pop w8");
    asm volatile("pop w7");

   // Set it to the correct time
   if (value->bcdFormat)
   {
       DATEH = (value->year << 8) | ((uint16_t)(value->month));
       DATEL = (((uint16_t)(value->day)) << 8 ) | ((uint16_t)(value->weekday));
       TIMEH = (((uint16_t)(value->hour)) << 8 ) | ((uint16_t)(value->minute));
       TIMEL = (((uint16_t)(value->second)) << 8);
   }
   else
   {
       DATEH = (((uint16_t)RTCC_DecToBCD(value->year)) << 8) | ((uint16_t)RTCC_DecToBCD(value->month));
       DATEL = (((uint16_t)RTCC_DecToBCD(value->day)) << 8) | ((uint16_t)RTCC_DecToBCD(value->weekday));
       TIMEH = (((uint16_t)RTCC_DecToBCD(value->hour)) << 8) | ((uint16_t)RTCC_DecToBCD(value->minute));
       TIMEL = (((uint16_t)RTCC_DecToBCD(value->second)) << 8);
   }
    
   // Enable RTCC, clear WRLOCK, Timestamping enabled
   RTCCON1L = 0x8001;
}

void RTCC_TimeGet (RTCC_DATETIME * value)
{
    uint16_t registerValue;
    
    /* Cause timestamp event. */
    RTCSTATLbits.TSAEVT = 1;
    
    while(RTCSTATLbits.TSAEVT != 1) { }
            
    registerValue = TSADATEH;
    value->month = registerValue & 0x00FF;
    value->year = registerValue >> 8;
    
    registerValue = TSADATEL;
    value->weekday = registerValue & 0xFF;
    value->day = registerValue >> 8;

    registerValue = TSATIMEH;
    value->minute = registerValue & 0xFF;
    value->hour = registerValue >> 8;
    
    registerValue = TSATIMEL;
    value->second = registerValue >> 8;

    if (!value->bcdFormat)
    {
        value->year = RTCC_BCDToDec (value->year);
        value->month = RTCC_BCDToDec (value->month);
        value->day = RTCC_BCDToDec (value->day);
        value->weekday = RTCC_BCDToDec (value->weekday);
        value->hour = RTCC_BCDToDec (value->hour);
        value->minute = RTCC_BCDToDec (value->minute);
        value->second = RTCC_BCDToDec (value->second);
    }
    
    RTCSTATLbits.TSAEVT = 0;
}

// Note : value must be < 100
static uint8_t RTCC_DecToBCD (uint8_t value)
{
    return (((value / 10)) << 4) | (value % 10);
}

static uint8_t RTCC_BCDToDec (uint8_t value)
{
    return ((value >> 4) * 10) + (value & 0x0F);
}

void inline __attribute__((deprecated)) BSP_RTCC_Initialize (BSP_RTCC_DATETIME * value)
{
    RTCC_Initialize(value);
}

void inline __attribute__((deprecated)) BSP_RTCC_TimeGet (BSP_RTCC_DATETIME * value)
{
    RTCC_TimeGet(value);
}

//---------------------

void delay(){
    unsigned int i;
    for(i=0;i<5000;i++);
}

//unsigned char ligne1[]={'T','e','m','p','e','r','a','t','u','r','e',' ','r','e',' ',' '};
//unsigned char ligne2[]={'e','s','t',' ','d','e',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'};

void sendCommand(unsigned char command){
    LCD_RS = 0;
    delay();
    LCD_EN = 1;
    delay();
    PORTE = command;
    delay();
    LCD_EN = 0;
    delay();
}

void sendData(unsigned char data){
    LCD_RS = 1;
    delay();
    LCD_EN = 1;
    delay();
    PORTE = data;
    delay();
    LCD_EN = 0;
    delay();
}

char ligne1[16];
char ligne2[16];

int main(int argc, char** argv){
    
    

    unsigned char i, s;
    TRISE = 0x00;
    TRISDbits.TRISD4 = 0;
    TRISBbits.TRISB15 = 0;
    
    for(i=0;i<16;i++){
        ligne1[i] = ' ';
    }
    for(i=0;i<16;i++){
        ligne2[i] = ' ';
    }
    
    int tempsensor = 8;
    TRISBbits.TRISB4 = 1;    //mettre en input RB4
    ANSBbits.ANSB4 = 1;      //mettre en analogue RB4 -> AN4 
    
    AD1CON1 = 0;
    AD1CON1bits.MODE12 = 0; 
    AD1CON1bits.FORM = 0;
    AD1CON1bits.SSRC = 0b0111; 
    AD1CON1bits.ASAM = 1; 
    AD1CON2 = 0; 
    AD1CON3 = 0;
    AD1CON3bits.ADRC = 1; 
    AD1CON3bits.SAMC = 2; 
    AD1CON3bits.ADCS = 2; 
    AD1CON5 = 0;
    AD1CHS = 0;
    AD1CHSbits.CH0SA = 4;
    AD1CON1bits.ADON = 1; 
    
    
    sendCommand(0x38);
    sendCommand(0x0E);
    sendCommand(0x01);
    
    while(1){
        
        time.bcdFormat = false;
        RTCC_BuildTimeGet( &time );
        RTCC_Initialize( &time );
        RTCC_TimeGet( &time );
        
        tempsensor = ADC1BUF0;
        double coeff = 0.004882;
        double tempamb = ((tempsensor*coeff)*125)/5;
        
        sprintf(ligne1, "temp: %.1f C    ", tempamb);
        
        
        
        sprintf(ligne2, "time:%02d/%02d %02d:%02d",time.day,time.month,time.hour,time.minute);
        
        
        sendCommand(0x80);
        for(i=0;i<16;i++){
            s = ligne1[i];
            sendData(s);
        }
        delay();
        ////
        //sprintf(ligne2, "time : ");
        sendCommand(0xc0);
        for(i=0;i<16;i++){
            s = ligne2[i];
            sendData(s);
        }
        delay();
           
        
    }
    return (EXIT_SUCCESS);
}
 
