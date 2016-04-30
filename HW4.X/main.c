#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include "MCP4902.h"
#include "i2c_master_noint.h"

#define EXPANDER_ADDR 0b0100000 //address of pin expander

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


// function prototypes
void setExpander(char pin, char level);
char readAllPins(void);
int abval(int val);

//set a pin high (1) or low (0) on the pin expander. Pins must already be
// configured as outputs.
void setExpander(char pin, char level){
    char currentLevels;
    currentLevels = readAllPins();
    i2c_master_start();    
    i2c_master_send(EXPANDER_ADDR << 1);
    i2c_master_send(0x0A); // send address of OLAT register
    if (level == 1){
        i2c_master_send(1 << pin | currentLevels); // make pin high
    }
    else if (level == 0){
        i2c_master_send(~(1 << pin) & currentLevels); // make pin low
    }
    i2c_master_stop();
      
}

char readAllPins(void){
    char levels;
    i2c_master_start();    
    i2c_master_send(EXPANDER_ADDR << 1); 
    i2c_master_send(0x09); // send address of GPIO register
    i2c_master_restart();  // send a RESTART so we can begin reading 
    i2c_master_send((EXPANDER_ADDR << 1) | 1); //1 in LSB indicating read
    levels = i2c_master_recv();       // receive a byte from the bus
    i2c_master_ack(1); // send NACK - don't want any more bytes
    i2c_master_stop();// send stop
    return levels;
}



 int abval(int val) //basic absolute value function I found online
 { 
     return (val<0 ? (-val) : val);
 } 

int main() {
    char val;
    int i;
    int j=0;
    char pinStates;
 
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
    LATAbits.LATA4 = 1; // RA4 is set high
    initSPI1(); 
    ANSELBbits.ANSB2 = 0; // turn off analog in for B2
    ANSELBbits.ANSB3 = 0; // turn off analog in for B3
    
    i2c_master_setup();
    __builtin_enable_interrupts();
    
    i2c_master_start();                     // Begin the start sequence
    i2c_master_send(EXPANDER_ADDR << 1); // send device address shifted left by 1 bit
                                        // to indicate a write
    i2c_master_send(0x00); // send address of IODIR register
    i2c_master_send(0xF0); // configure GP0-3 as outputs and GP4-7 as inputs
    i2c_master_stop();
   
    while(1){
        
        
        for (i=0; i<200; i++){
            
            pinStates = readAllPins();
            if (pinStates & 0b10000000) {
                setExpander(0, 1);
            }
            else setExpander(0, 0);
            
            val=2.55*(100-abval(i%200-100));
            setVoltage(0,val); //set voltage on channel B
            setVoltage(1,(127+127*sin(j/12.56637)));
            j=j+1;;
            if(j==79){
                j=0;  
            }
            _CP0_SET_COUNT(0);
            while (_CP0_GET_COUNT() < 24000){} // wait for 1ms
            
        }
    }
}