#ifndef _LCD_SHIFT_REG_
#define _LCD_SHIFT_REG_

void transmit_data_lcd(unsigned char data)
{	
	unsigned char i;
	for (i = 0; i < 8 ; ++i)
	{
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		//PORTC = 0x80;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		//PORTC |= (((green >> i) & 0x01) << 4) & 0xF0;
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x02;
		//PORTC |= 0x20;
	}

	PORTB |= 0x04;
	PORTB = 0x00;
}

#endif