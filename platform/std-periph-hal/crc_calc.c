#include "crc_calc.h"

const uint32_t poly_CMS300 = 0b10001;
const uint32_t seed = 0b1010;
const uint32_t n = 4;
const uint32_t datasize = 28;// bits

uint32_t nCRC(uint32_t data, uint32_t poly, uint32_t seed, uint32_t datasize, uint32_t n)
{
	data = (data << n) | seed;
	for(uint32_t i = datasize ; i != 0 ; --i)
	{
		// if the current MSB of the data is zero, skip iteration
		if(!(data & bitN(i+n-1)))
			continue;
		data ^= (poly << (i-1));
	}
	return data;
}
uint32_t prepareMessageCMS300(uint32_t data)
{
	
	uint32_t crc = nCRC(data, poly_CMS300, seed, datasize, n);
	
	return ((data << n) | (crc & 0xF));
}
uint32_t checkCRCmessageCMS300(uint32_t msg)
{
	return !(nCRC(msg, poly_CMS300, 0, 32, 0) & 0xF);
}
uint32_t bitN(uint32_t n)
{
	return ((uint32_t) 1) << n;
}
