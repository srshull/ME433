#ifndef MCP4902_H    /* Guard against multiple inclusion */
#define MCP4902_H

//sends a character over SPI1
unsigned char spi_io(unsigned char o);

//initializes SPI1
void initSPI1();

/*sets a voltage on the DAC
 * INPUTS:
 * channel: 0 for A, 1 for B
 * voltage: a value between 0-255
 */
void setVoltage(char channel, unsigned char voltage);


#endif


