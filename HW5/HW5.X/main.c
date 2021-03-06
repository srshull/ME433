#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "ILI9163C.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV by 24 to get 96 MHz
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL by 2 to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID =  0x0000// some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

/*
void drawCharacter(unsigned short x, unsigned short y, char character){
    int i=0;
    int j;
    for (i=0; i<5; i++){
        for (j=0; j<8; j++){
            if (ASCII[character-0x20][i] & 1<<j){
                LCD_drawPixel(x+i,y+j,BLACK);
            }
            else {
                LCD_drawPixel(x+i,y+j,WHITE);
            }
        }
    }
}

void writeString(unsigned short x, unsigned short y, char* message){
    int i = 0;
    while(message[i]){drawCharacter(x+6*i,y,message[i]); i++;}
    
}
*/

int main() {
    int i=0;
    int x=1;
    int y=1;
    int leet=15189;
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1; // RB4 is an input
    TRISAbits.TRISA4 = 0; // RA4 is an output
    LATAbits.LATA4 = 1; // RA4 is set high
    SPI1_init();
    LCD_init();
    LCD_clearScreen(WHITE);
    __builtin_enable_interrupts();
  
    char message[50];
  
    sprintf(message, "It Works! %d!", leet);
    //while(message[i]){drawCharacter(28+6*i,32,message[i]); i++;}
    LCD_writeString(1,1,message);
    LCD_writeString(1,10,message);
    while(1) {
    }
}