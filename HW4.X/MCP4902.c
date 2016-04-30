#include<xc.h>           // processor SFR definitions

#define CS LATBbits.LATB15

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}
void initSPI1(){
    SDI1Rbits.SDI1R = 0b0000; // SDI1 on pin A1
    RPB13Rbits.RPB13R = 0b0011; // SD01 on pin B13
    TRISBbits.TRISB15 = 0; // RB15 is an output (SS pin)
    CS = 1; 
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x3;            // baud rate to 10 MHz [SPI4BRG = (80000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 4

}

void setVoltage(char channel, unsigned char voltage){
    unsigned short data = voltage;
    if (channel == 0) {//0 is channel A
        data = (data << 4) | 0x7000;
    }
    else if (channel == 1){ // 1 is channel B
        data = (data << 4) | 0xF000;
    }
    CS = 0;
    spi_io((data & 0xFF00) >> 8 ); // most significant byte of data
    spi_io(data & 0x00FF);
    CS = 1;
}

