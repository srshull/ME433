#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c_master_noint.h"

#define IMU_ADDR 0b1101010 //address of IMU

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



int main() {
    char check;
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
    TRISBbits.TRISB4 = 1; // RB4 is an input (user button)
    TRISAbits.TRISA4 = 0; // RA4 is an output (user LED)
    LATAbits.LATA4 = 0; // RA4 is set high
    ANSELBbits.ANSB2 = 0; // turn off analog in for B2
    ANSELBbits.ANSB3 = 0; // turn off analog in for B3
    
    i2c_master_setup();
    
    T3CONbits.ON=0; // timer 3 off
	T3CONbits.TCKPS=0; // prescaler = 1 
	PR3=47999; // gives us 1kHz frequency
	T3CONbits.ON=1; // timer 3 on
  
	OC1CONbits.OCTSEL=1; // use timer 3 for output compare
	OC1CONbits.OCM=6; // simple PWM mode, no fault pin
	OC1R= 24000; // sets the duty cycle
	OC1RS= 24000; // sets the future duty cycle
    RPA0Rbits.RPA0R = 0b0101; //OC1 on A0
	OC1CONbits.ON=1; // output compare 1 on
    __builtin_enable_interrupts();
    
    i2c_master_start();
    i2c_master_send(IMU_ADDR << 1); 
    i2c_master_send(0x0F); // send address of WHOAMI register
    i2c_master_restart();  // send a RESTART so we can begin reading 
    i2c_master_send((IMU_ADDR << 1) | 1); //1 in LSB indicating read
    check = i2c_master_recv();       // receive a byte from the bus
    i2c_master_ack(1); // send NACK - don't want any more bytes
    i2c_master_stop();// send stop
    if (check == 0b01101001){
        LATAbits.LATA4 = 1; // RA4 is set high
    }
    while(1){
    }
}