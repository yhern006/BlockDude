/*
 * Wednesday537pm.c
 *
 * Created: 3/19/2014 5:37:46 PM
 *  Author: Yvette
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <UCR/io.c>
#include <UCR/blockcheck.h>
#include "timer.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdbool.h>                                                     

//--------Shared Variables----------------------------------------------------
unsigned char reset, up, down = 0;
unsigned char game_off = 0x01;
unsigned char column_val1 = 0x20; // sets the pattern displayed on columns
unsigned char column_sel1 = 0xBF; // grounds column to display pattern
unsigned char topRed, redexists, redIndex = 0x00;
unsigned char tempa, tempa1 = 0x00;
unsigned char tempb, tempb1 = 0xFF;
unsigned char doorCol = 0xFE;
unsigned char doorRow = 0x04;
unsigned char rblock = 0xFF;
unsigned char rwblck = 0x00;
unsigned char row = 0x18;
unsigned char col = 0xEF;
unsigned short x_downLimit = 731;
unsigned short x_upLimit = 745;
unsigned short y_downLimit = 730;
unsigned short y_upLimit = 745;
unsigned short z_downLimit = 801;
unsigned short z_upLimit = 809;
unsigned short x, y, z, x_prev, y_prev, z_prev;
unsigned char backup_red[] = {0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};

//--------End Shared Variables------------------------------------------------


void ADC_init(void)
{
	ADMUX |= (1 << REFS0);   //AVcc with external capacitor at AREF
	ADCSRA |= (1 << ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
	//ENABLE ADC, PRESCALER 128
}

unsigned short ADC_read(unsigned char ch)
{
	ch = ch & 0b00000111;		  // channel must be b/w 0 to 7
	ADMUX = (ADMUX & 0xf8) | ch;  //Clear last 3 bits of ADMUX, OR with ch (selecting channel)
	ADCSRA |= (1 << ADSC);        //START CONVERSION
	while((ADCSRA) & (1 << ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}

unsigned short equilibrium(unsigned char x, unsigned char y, unsigned char z)
{
	if(x <= x_upLimit && x >= x_downLimit)
	{
		if(y <= y_upLimit && y >= y_downLimit)
		{
			if(z <= z_upLimit && z >= z_downLimit) return 1;
			else return 0;
		}
	}
	return 0;
}

void transmit_data(unsigned char data, unsigned char regNum)
{
	unsigned char i;
	for (i = 0; i < 8 ; ++i)
	{
		if(regNum == 1)		// Blue
		{
			PORTB = 0x80;
			PORTB |= (((data >> i) & 0x01) << 4) & 0xF0;
			PORTB |= 0x20;
		}
		else if(regNum == 2)	// Green
		{
			PORTD = 0x80;
			PORTD |= (((data >> i) & 0x01) << 4) & 0xF0;
			PORTD |= 0x20;
		}
		else if(regNum == 3)	// Red
		{
			PORTD = 0x08;
			PORTD |= ((data >> i) & 0x01);
			PORTD |= 0x02;
		}
		else		// inputs (used to be PORTA)
		{
			PORTC = 0x08;
			PORTC |= ((data >> i) & 0x01);
			PORTC |= 0x02;
		}
	}
	
	if(regNum == 1)
	{
		PORTB |= 0x40;
		PORTB = 0x00;
	}
	else if(regNum == 2 || regNum == 3)
	{
		if(regNum == 2) PORTD |= 0x40;
		else PORTD |= 0x04;
		PORTD = 0x00;
	}
	else
	{
		PORTC |= 0x04;
		PORTC = 0x00;
	}
}

// Ground Bricks
enum SM3_States {SM3_s0};
int SMTick3(int SM3_State)
{
	static unsigned char index1 = 0;
	static unsigned char column_val2 = 0xE0;
	static unsigned char column_sel2 = 0x7F;
	unsigned char tempDoorRow = 0x00;

	switch(SM3_State)
	{
		case -1:
			SM3_State = SM3_s0;
			break;
		case SM3_s0:	 // stay
			SM3_State = SM3_s0;
			break;
		default:
			SM3_State = SM3_s0;
			break;
	}
	switch(SM3_State)
	{
		case SM3_s0:
			if(!game_off)
			{
				if(index1 >= sizeof(cols3)) index1 = 0;
				transmit_data(0x00, 4);
				column_sel2 = cols3[index1];
				column_val2 = rows3[index1++];
				transmit_data(0xFF, 2);  //Turn green off
				transmit_data(0xFF, 3);  //Turn red off			
				transmit_data(column_sel2, 1); // Blue
				if(column_sel2 == doorCol) tempDoorRow = doorRow;
				transmit_data(column_val2 | tempDoorRow, 4);
			}
			break;
		default:
			break;
	}
	return SM3_State;
}


// Little Dude [GREEN]
enum SM1_States {SM1_State_s0} ;
int SMTick1(int state1)
{
	switch (state1)
	{
		case -1:
			state1 = SM1_State_s0;
			break;
		case SM1_State_s0:   // stay
			state1 = SM1_State_s0;
			break;
		default:
			state1 = SM1_State_s0;
			break;
	}

	switch(state1)
	{
		case SM1_State_s0:
			if(!game_off && !reset)
			{
				tempa1 = column_val1;
				tempb1 = column_sel1;
				transmit_data(0x00, 4);
				transmit_data(0xFF, 1);		//Turn Blue off
				transmit_data(0xFF, 3);		//Turn Red off
				transmit_data(tempb1, 2);	//Green On
				transmit_data(tempa1, 4);
			}
			else if(reset)
			{
				column_val1 = 0x20;
				column_sel1 = 0xBF;
				transmit_data(0x00, 4);
				transmit_data(0xFF, 1);		//Turn Blue off
				transmit_data(0xFF, 3);		//Turn Red off
				transmit_data(column_sel1, 2);	//Green On
				transmit_data(column_val1, 4);
			}
			break;
		default:
			break;
	}
	return state1;
}

//Displays Red Blocks
enum SM2_States {SM2_s0};
int SMTick2(int state2)
{
	static unsigned char index3 = 0;
	unsigned char tempDoorRow = 0x00;
	switch(state2)
	{
		case -1:
			state2 = SM2_s0;
			break;
		case SM2_s0:
			state2 = SM2_s0;
			break;
		default:
			state2 = SM2_s0;
			break;
	}
	switch(state2)
	{
		case SM2_s0:
			if(!game_off)
			{
				if(index3 >= sizeof(cols3)) index3 = 0;
				transmit_data(0x00, 4);
				rblock = cols3[index3];
				rwblck = blk_row[index3++];			
				transmit_data(0xFF, 2);  //Turn green off
				transmit_data(0xFF, 1);  //Turn blue off
				if(rblock == doorCol) tempDoorRow = doorRow;
				transmit_data(rblock, 3); // Red On
				transmit_data(rwblck | tempDoorRow, 4);
			}
			else
			{
				index3 = 0;
				for(unsigned char i = 0; i < sizeof(blk_row); i++)
					blk_row[i] = backup_red[i];
			}
			break;
		default:
			break;
	}
	return state2;
}

// Accelerometer
enum SM4_States {SM4_Init, SM4_downLeft, SM4_downRight, SM4_Equilibrium, SM4_Wait, 
				 SM4_Left, SM4_Right, SM4_upRight, SM4_upLeft} ;
int SMTick4(int state4)
{
	static unsigned char indx2, counter, counter3 = 0;
	unsigned char topRow = 0x00;
	z_prev = z;
	x_prev = x;
	y_prev = y;
	
	x = ADC_read(0);      //READ ADC VALUE FROM PA.0
	y = ADC_read(1);      //READ ADC VALUE FROM PA.1
	z = ADC_read(2);      //READ ADC VALUE FROM PA.2

	switch(state4) // Transitions
	{
		case -1:
			state4 = SM4_Init;
			break;
		case SM4_Init:
			state4 = SM4_Equilibrium;
			break;
		case SM4_Equilibrium:
			if(counter < 50 && !reset) state4 = SM4_Equilibrium;
			else if(reset) state4 = SM4_Init;
			else state4 = SM4_Wait;
			break;
		case SM4_Wait:
			if(reset) state4 = SM4_Init;
			else if(!equilibrium(x, y, z) && counter3 >= 70)
			{
				if(x < x_downLimit && y < y_downLimit && z < z_downLimit) state4 = SM4_downLeft;
				else if(x > x_upLimit && y < y_downLimit && z < z_downLimit) state4 = SM4_downRight;
				else if(x < x_downLimit && x <= (x_upLimit - 4) && y <= y_upLimit && y >= y_downLimit)
					state4 = SM4_Left;
				else if(x > (x_upLimit - 4) && y <= (y_upLimit - 4) && y >= y_downLimit) 
					state4 = SM4_Right;
				else if(x > x_upLimit && y > y_upLimit && z < z_upLimit) state4 = SM4_upRight;
				else if(x < x_downLimit && y > y_upLimit && z < z_upLimit) state4 = SM4_upLeft;
				else state4 = SM4_Equilibrium;
			}
			else state4 = SM4_Wait;
			break;
		case SM4_downLeft:
			state4 = SM4_Init;
			break;
		case SM4_downRight:
			state4 = SM4_Init;
			break;
		case SM4_Left:
			state4 = SM4_Init;
			break;
		case SM4_Right:
			state4 = SM4_Init;
			break;
		case SM4_upRight:
			state4 = SM4_Init;
			break;
		case SM4_upLeft:
			state4 = SM4_Init;
			break;
		default:
			state4 = SM4_Init;
			break;
	}

	switch(state4)
	{
		case SM4_Init:
			counter = 0;
			counter3 = 0;
			break;
		case SM4_Equilibrium:
			counter++;
			break;
		case SM4_Wait:
			counter = 0;
			counter3++;
			break;
		case SM4_downLeft:
			if((column_sel1 | 0x7F) != 0x7F)
			{
				if(!leftDotExists(column_val1, column_sel1))
				{
					if(redexists && !leftBlock(column_val1, column_sel1))	//put down
					{
						redIndex = findIndex(column_sel1);
						if(blk_row[redIndex - 1] > column_val1)
							topRow = findTopRow(blk_row[redIndex - 1]);
						else
							topRow = findTopRow(rows3[redIndex - 1]);
						
						blk_row[redIndex - 1] |= (topRow >> 1) & 0x7F;
						blk_row[redIndex] &= ~(column_val1 >> 1) & 0x7F;
						redexists = 0x00;
					}
					else if(!redexists && leftBlock(column_val1, column_sel1))		//pick up block
					{
						redIndex = findIndex(column_sel1);
						topRed = blk_row[redIndex - 1] & column_val1;    //gets row of left
						blk_row[redIndex - 1] &= ~topRed;
						blk_row[redIndex] |= (column_val1 >> 1) & 0x7F;
						redexists = 0x01;
					}
				}
				y
			}
			break;
		case SM4_downRight:
			if((column_sel1 | 0xFE) != 0xFE)
			{
				if(!rightDotExists(column_val1, column_sel1))
				{
					if(redexists && !rightBlock(column_val1, column_sel1)) //putting down
					{
						redIndex = findIndex(column_sel1);
						if(blk_row[redIndex + 1] > blk_row[redIndex])
						{
							blk_row[redIndex] |= (column_sel1 >> 1) & 0x7F;
							blk_row[redexists + 1] &= ~((column_val1 >> 1) & 0x7F);
						}
						else //no red blocks next column
						{
							topRow = findTopRow(rows3[redIndex + 1]);
							blk_row[redIndex + 1] = (topRow >> 1) & 0x7F;
							blk_row[redIndex] &= ~((column_val1 >> 1) & 0x7F);
						}
						redexists = 0x00;
					}
					else if(!redexists && rightBlock(column_val1, column_sel1) && !rightBlock(column_val1 >> 1, column_sel1))
					{
						redIndex = findIndex(column_sel1);
						blk_row[redIndex + 1] &= ~(column_val1);
						blk_row[redIndex] |= ((column_val1 >> 1) & 0x7F);
						redexists = 0x01;
					}
				}
			}
			counter3 = 0;
			break;
		case SM4_Left:
			if((column_sel1 | 0x7F) != 0x7F)
			{
				if(bottomDotExists(column_val1, (column_sel1 << 1) | 0x01) || bottomBlock(column_val1, (column_sel1 << 1) | 0x01))
				{
					if(!leftDotExists(column_val1, column_sel1) && !leftBlock(column_val1, column_sel1))
					{
						column_sel1 = (column_sel1 << 1) | 0x01;
						if(redexists)
						{
							redIndex = findIndex(column_sel1);
							blk_row[redIndex] |= ((column_val1 >> 1) & 0x7F);
							blk_row[redIndex + 1] &= ~((column_val1 >> 1) & 0x7F);
						}
					}
				}
				else //need to move downleft
				{
					indx2 = findIndex(column_sel1);
					if(blk_row[indx2 - 1] > column_val1)
					{
						topRow = findTopRow(blk_row[indx2 - 1]);
						if(redexists)
						{
							blk_row[redIndex - 1] |= (topRow >> 2);
							blk_row[redIndex] &= ~((column_val1 >> 1) & 0x7F);
						}
						column_sel1 = (column_sel1 << 1) | 0x01;
						column_val1 = (topRow >> 1) & 0x7F;
					}
					else	//no red blocks
					{
						topRow = findTopRow(rows3[indx2 - 1]);
						if(redexists)
						{
							blk_row[indx2] &= ~((column_val1 >> 1) & 0x7F);
							column_val1 = (topRow >> 1) & 0x7F;
							blk_row[indx2 - 1] |= ((column_val1 >> 1) & 0x7F);
						}
						else column_val1 = (topRow >> 1) & 0x7F;
						column_sel1 = (column_sel1 << 1) | 0x01;		
					}
				}
			}
			counter3 = 0;
			break;
		case SM4_Right:
			if((column_sel1 | 0xFE) != 0xFE)
			{
				if(bottomDotExists(column_val1, (column_sel1 >> 1) | 0x80) || bottomBlock(column_val1, (column_sel1 >> 1) | 0x80))
				{
					if(!rightDotExists(column_val1, column_sel1) && !rightBlock(column_val1, column_sel1))
					{
						column_sel1 = (column_sel1 >> 1) | 0x80;
						if(redexists)
						{
							redIndex = findIndex(column_sel1);
							blk_row[redIndex] |= (column_val1 >> 1) & 0x7F;
							blk_row[redIndex - 1] &= ~((column_val1 >> 1) & 0x7F);
						}
					}
				}
				else //need to move down right
				{
					indx2 = findIndex(column_sel1);
					if(blk_row[indx2 + 1] > column_val1)
					{
						topRow = findTopRow(blk_row[indx2 + 1]) - 1;
						if(redexists) blk_row[redIndex + 1] |= (column_val1 << topRow);
						blk_row[redIndex] &= ~((column_val1 >> 1) & 0x7F);
					}
					else	//no red blocks
					{
						if(redexists)
						{
							topRow = findTopRow(rows3[indx2 + 1]);
							blk_row[indx2] &= ~((column_val1 >> 1) & 0x7F);
							column_val1 = (topRow >> 1) & 0x7F;
							column_sel1 = (column_sel1 >> 1) | 0x80;
							blk_row[indx2 + 1] = ((column_val1 >> 1) & 0x7F);
						}
						else
						{
							topRow = findTopRow(rows3[indx2 + 1]);
							column_val1 = (topRow >> 1) & 0x7F;
							column_sel1 = (column_sel1 >> 1) | 0x80;
						}
					}
				}
			}
			counter3 = 0;
			break;		
		case SM4_upRight:
			if((column_sel1 | 0xFE) != 0xFE)
			{
				indx2 = findIndex(column_sel1);
				unsigned char shiftedval = (column_val1 >> 1) & 0x7F;
				if(rightDotExists(column_val1, column_sel1) || rightBlock(column_val1, column_sel1))
				{
					if(!rightBlock(shiftedval, column_sel1))
					{
						if(!rightDotExists(shiftedval, column_sel1))
						{
							column_sel1 = (column_sel1 >> 1) | 0x80;
							column_val1 = (column_val1 >> 1) & 0x7F;
							if(redexists)
							{
								blk_row[indx2] &= ~(column_val1);
								blk_row[indx2 + 1] |= (column_val1 >> 1) & 0x7F;
							}
						}
					}
				}
			}
			counter3 = 0;
			break;
		case SM4_upLeft:
			if((column_sel1 | 0x7F) != 0x7F)
			{
				indx2 = findIndex(column_sel1);
				if(leftDotExists(column_val1, column_sel1) || leftBlock(column_val1, column_sel1))
				{
					if(blk_row[indx2 - 1] != ((column_val1 >> 1) & 0x7F) && cols3[indx2 - 1] != ((column_val1 >> 1) & 0x7F))
					{
						column_sel1 = (column_sel1 << 1) | 0x01;
						column_val1 = (column_val1 >> 1) & 0x7F;
						if(redexists)
						{
							blk_row[indx2] &= ~(column_val1);
							blk_row[indx2 - 1] |= (column_val1 >> 1) & 0x7F;
						}
					}
				}
			}
			counter3 = 0;
			break;
		default:
			break;
		tempa1 = column_val1;
		tempb1 = column_sel1;
	}
	return state4;
}

// RESET
enum SM5_States {SM5_Wait, SM5_Reset, SM5_On};
int SMTick5(int state5)  // Period of 1 ms
{
	unsigned char tempButton = ~PINA;
	switch(state5) // Transitions
	{
		case -1:
			state5 = SM5_Wait;
			break;
		case SM5_Wait:   // Off / Waiting for input
			if((tempButton & 0x10) == 0x10) state5 = SM5_On;
			else state5 = SM5_Wait;
			break;
		case SM5_Reset: // On / Reset
			state5 = SM5_On;
			break;
		case SM5_On:
			if((tempButton & 0x10) == 0x10) state5 = SM5_Reset;
			else state5 = SM5_On;
			break;
		default:
			state5 = SM5_Wait;
			break;
	}
	switch(state5)
	{
		case SM5_Wait:
			reset = 0x00;
			game_off = 0x01;
			break;
		case SM5_Reset: // On / Reset
			reset = 0x01;
			game_off = 0x01;
			redexists = 0x00;
			LCD_DisplayString(17, "Level 1         ");
			break;
		case SM5_On:
			game_off = 0x00;
			reset = 0x00;
			break;
		default:
			break;
	}
	return state5;
}

// Checking win
enum SM6_States {SM6_Wait, SM6_Win};
int SMTick6(int state6)
{
	switch(state6)
	{
		case -1:
			state6 = SM6_Wait;
			break;
		case SM6_Wait:
			if(column_sel1 == doorCol && column_val1 == doorRow)
				state6 = SM6_Win;
			else state6 = SM6_Wait;
			break;
		case SM6_Win:
			if(reset) state6 = SM6_Wait;
			else state6 = SM6_Win;
			break;
		default:
			state6 = SM6_Wait;
			break;
	}
	switch(state6)
	{
		case SM6_Wait:
			break;
		case SM6_Win:
			column_val1 = 0x20;
			column_sel1 = 0xBF;
			transmit_data(0x00, 4);
			transmit_data(0xFF, 1);
			transmit_data(0xFF, 3);
			transmit_data(0x00, 2);
			transmit_data(0xFF, 4);
			game_off = 0x01;
			LCD_DisplayString(25, "Complete");
			break;
		default:
			break;
	}
	return state6;
}

int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	x = ADC_read(0);      //READ ADC VALUE FROM PA.0
	y = ADC_read(1);      //READ ADC VALUE FROM PA.1
	z = ADC_read(2);      //READ ADC VALUE FROM PA.2
		
	ADC_init();
	LCD_init();
	LCD_ClearScreen();
	LCD_DisplayString(1, "Welcome!");
	LCD_DisplayString(17, "Level 1");

	unsigned long SMTick1_calc = 1;		// Little Dude
	unsigned long SMTick2_calc = 1;		// Red Block
	unsigned long SMTick3_calc = 1;		// Blue Bricks
	unsigned long SMTick4_calc = 2;		// Accelerometer
	unsigned long SMTick5_calc = 10;	// LCD
	unsigned long SMTick6_calc = 1;     // Check Win

	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick2_calc, SMTick1_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);
	tmpGCD = findGCD(tmpGCD, SMTick5_calc);
	tmpGCD = findGCD(tmpGCD, SMTick6_calc);

	unsigned long int GCD = tmpGCD;

	unsigned long SMTick2_period = SMTick2_calc/GCD;
	unsigned long SMTick1_period = SMTick1_calc/GCD;
	unsigned long SMTick3_period = SMTick3_calc/GCD;
	unsigned long SMTick4_period = SMTick4_calc/GCD;
	unsigned long SMTick5_period = SMTick5_calc/GCD;
	unsigned long SMTick6_period = SMTick6_calc/GCD;

	static task task1, task2, task3, task4, task5, task6;
	task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SMTick1;//Function pointer for the tick.

	task2.state = -1;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &SMTick2;//Function pointer for the tick.

	task3.state = -1;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period;//Task current elapsed time.
	task3.TickFct = &SMTick3;//Function pointer for the tick.
	
	task4.state = -1;//Task initial state.
	task4.period = SMTick4_period;//Task Period.
	task4.elapsedTime = SMTick4_period;//Task current elapsed time.
	task4.TickFct = &SMTick4;//Function pointer for the tick.
	
	task5.state = -1;//Task initial state.
	task5.period = SMTick5_period;//Task Period.
	task5.elapsedTime = SMTick5_period;//Task current elapsed time.
	task5.TickFct = &SMTick5;//Function pointer for the tick.
	
	task6.state = -1;//Task initial state.
	task6.period = SMTick6_period;//Task Period.
	task6.elapsedTime = SMTick6_period;//Task current elapsed time.
	task6.TickFct = &SMTick6;//Function pointer for the tick.

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}