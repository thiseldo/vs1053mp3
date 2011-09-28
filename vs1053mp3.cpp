/***********************************************************************
 ***********************************************************************/
#include <spi.h>
#include "digitalWriteFast.h"
#include "vs1053mp3.h"

#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)


// Default constructor
vs1053mp3::vs1053mp3( void ) {
}

void vs1053mp3::init( ) {
  pinMode(MP3_DREQ, INPUT);
  digitalWriteFast( MP3_DREQ, HIGH );
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is 1.0x after power up. 
  //Therefore, max SPI speed is 1.75MHz. We will use 1MHz to be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV16); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
//  SPI.setClockDivider(SPI_CLOCK_DIV4); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF); //Throw a dummy byte at the bus

  //Initialize VS1053 chip 
  digitalWriteFast(MP3_XCS, HIGH); //Deselect Control
  digitalWriteFast(MP3_XDCS, HIGH); //Deselect Data

  setVolume(20, 20); //Set initial volume (20 = -10dB)

  //Now that we have the VS1053 up and running, increase the VS1053 internal clock multiplier and up our SPI rate
  writeRegister(SCI_CLOCKF, 0x60, 0x00); //Set multiplier to 3.0x

  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is now 3x.
  //Therefore, max SPI speed is 5MHz. 4MHz will be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV4); //Set SPI bus speed to 4MHz (16MHz / 4 = 4MHz)

}

static void sendSPI(byte data) {
    SPDR = data;
    while (!(SPSR&(1<<SPIF)))
        ;
}
 
//Write to VS10xx register
//SCI: Data transfers are always 16bit. When a new SCI operation comes in 
//DREQ goes low. We then have to wait for DREQ to go high again.
//XCS should be low for the full duration of operation.
void vs1053mp3::writeRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte){
  while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
  digitalWriteFast(MP3_XCS, LOW); //Select control

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x02); //Write instruction
  SPI.transfer(addressbyte);
  SPI.transfer(highbyte);
  SPI.transfer(lowbyte);
  while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  digitalWriteFast(MP3_XCS, HIGH); //Deselect Control
}

//Read the 16-bit value of a VS10xx register
unsigned int vs1053mp3::readRegister (unsigned char addressbyte){
  digitalWriteFast(MP3_XDCS, HIGH);
  while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
  digitalWriteFast(MP3_XCS, LOW); //Select control

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x03); //Read instruction
  SPI.transfer(addressbyte);

  char response1 = SPI.transfer(0xFF); //Read the first byte
  while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  char response2 = SPI.transfer(0xFF); //Read the second byte
  while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete

  digitalWriteFast(MP3_XCS, HIGH); //Deselect Control

  int resultvalue = response1 << 8;
  resultvalue |= response2;
  return resultvalue;
}

//Set VS10xx Volume Register
void vs1053mp3::setVolume(unsigned char leftchannel, unsigned char rightchannel){
  writeRegister(SCI_VOL, leftchannel, rightchannel);
}

void vs1053mp3::playBuffer( unsigned char *buf, int buflen ) {
    unsigned char *p;
    int dlen = buflen;
    p = buf; // Point "p" to the beginning of array

    while(dlen>0) {
      while(!digitalReadFast(MP3_DREQ)) { 
        //DREQ is low while the receive buffer is full
        //You can do something else here, the bus is free...
        //Maybe set the volume or whatever...
      	digitalWriteFast(MP3_XDCS, HIGH); //Deselect Data
      }

      //Once DREQ is released (high) we can now send 32 bytes of data
      digitalWriteFast(MP3_XDCS, LOW); //Select Data
    //  output_low( 
//      for( int i = 0; i<32 && dlen >0; i++ ) {
        //SPI.transfer(*p++); // Send SPI byte
    	SPDR = *p++;
    	while (!(SPSR&(1<<SPIF))) ;
        //sendSPI(*p++); // Send SPI byte
      	dlen--;
//      }
//      while(!digitalReadFast(MP3_DREQ)) ; //Wait for DREQ to go high indicating transfer is complete
      //digitalWriteFast(MP3_XDCS, HIGH); //Deselect Data
    }
    digitalWriteFast(MP3_XDCS, HIGH); //Deselect Data
}

// Send 2048 bytes of 0 to clear buffer and terminate stream
void vs1053mp3::closeStream() {
    digitalWriteFast(MP3_XDCS, LOW); //Select Data
    for (int i = 0 ; i < 2048 ; i++) {
      while(!digitalRead(MP3_DREQ)); //If we ever see DREQ low, then we wait here
    	SPDR = 0;
    	while (!(SPSR&(1<<SPIF))) ;
    }
    while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating transfer is complete
    digitalWrite(MP3_XDCS, HIGH); //Deselect Data


}

// Thats all folks!
