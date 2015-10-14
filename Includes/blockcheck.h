#ifndef CHECKBLOCK_H
#define CHECKBLOCK_H

#include <stdbool.h> 

unsigned char cols3[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};  // Blue
unsigned char rows3[] = {0xC0, 0xC0, 0xC0, 0xE0, 0xC0, 0xC0, 0xE0, 0xF8};  // Blue
unsigned char blk_row[] = {0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00}; //Red
	
bool rightDotExists(unsigned char cur_val, unsigned char cur_sel)
{
	for(unsigned char i = 0; i < sizeof(cols3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			if(i + 1 != sizeof(cols3))
			{
				if((rows3[i + 1] & cur_val) == cur_val) return true;
				else return false;
			}
			else return false;
		}
	}
	return false;
}

bool leftDotExists(unsigned char cur_val, unsigned char cur_sel)
{
	for(unsigned char i = 0; i < sizeof(cols3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			if(i != 0)
			{
				if((rows3[i - 1] & cur_val) == cur_val) return true;
				else return false;
			}
			else return false;
		}
	}
	return false;
}

bool topDotExists(unsigned char cur_val, unsigned char cur_sel)
{
	unsigned char tempvar = 0x00;
	for(unsigned char i = 0; i < sizeof(rows3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			tempvar = (cur_val >> 1) & 0x7F;
			if((rows3[i] & tempvar) == tempvar) return true;
			else return false;
		}
	}
	return false;
}

bool bottomDotExists(unsigned char cur_val, unsigned char cur_sel)
{
	unsigned char tempvar = 0x00;
	for(unsigned char i = 0; i < sizeof(rows3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			tempvar = (cur_val << 1) & 0xFE;
			if((rows3[i] & tempvar) == tempvar) return true;
			else return false;
		}
	}
	return false;
}

bool rightBlock(unsigned char cur_val, unsigned char cur_sel)
{
	for(unsigned char i = 0; i < sizeof(cols3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			if(i + 1 != sizeof(cols3))
			{
				if((blk_row[i + 1] & cur_val) == cur_val) return true;
				else return false;
			}
			else return false;
		}
	}
	return false;
}

bool leftBlock(unsigned char cur_val, unsigned char cur_sel)
{
	for(unsigned char i = 0; i < sizeof(cols3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			if(i != 0)
			{
				if((blk_row[i - 1] & cur_val) == cur_val) return true;
				else return false;
			}
			else return false;
		}
	}
	return false;
}

bool bottomBlock(unsigned char cur_val, unsigned char cur_sel)
{
	unsigned char tempvar = 0x00;
	for(unsigned char i = 0; i < sizeof(rows3); i++)
	{
		if(cols3[i] == cur_sel)
		{
			tempvar = (cur_val << 1) & 0xFE;
			if((blk_row[i] & tempvar) == tempvar) return true;
			else return false;
		}
	}
	return false;
}

unsigned char findIndex(unsigned char col)
{
	unsigned char i = 0;
	for(i = 0; i < sizeof(cols3); i++)
	{
		if(col == cols3[i]) return i;
	}
	i = 9; //if not found
	return i;
}

unsigned char findTopRow(unsigned char cur_val)
{
	unsigned char i;
	unsigned char tval = 0x01;
	for(i = 0; i < 8; i++)
	{
		if((tval & cur_val) == tval) return tval;
		else tval = (tval << 1) & 0xFE;
	}
	return 0x80;	//Not found
}

#endif