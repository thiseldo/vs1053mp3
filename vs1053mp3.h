/*
* vs1053mp3.h
* VS1053 MP3 decoder library header
*/

#ifndef _VS1053MP3_H
#define _VS1053MP3_H

/* Any #defines */
#define MP3_XCS 9 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS 2 //Data Chip Select / BSYNC Pin
#define MP3_DREQ 3 //Data Request Pin: Player asks for more data

//VS10xx SCI Registers
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F


class vs1053mp3 {
  private:

  public:
  	vs1053mp3();
  	
	void init( );
	void writeRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte);
	unsigned int readRegister (unsigned char addressbyte);
	void setVolume(unsigned char leftchannel, unsigned char rightchannel);
	void playBuffer( unsigned char *p, int dlen );
	void closeStream();
};

#endif //_VS1053MP3_H

