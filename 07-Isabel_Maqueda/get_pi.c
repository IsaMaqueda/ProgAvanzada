/*
	Program to approximate pi by using the infinite series
		pi = 4 - (4/3) + (4/5) - (4/7) + (4/9) - ...
	
	Based on a program proposed in:
		http://www.dreamincode.net/forums/topic/99268-calculate-the-value-of-pi-using-infinite-series/

	Isabel Maqueda Rolon
    A01652906
    24/10/2019
*/

#include "get_pi.h"

double computePI(unsigned long int iterations)
{
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;
    unsigned long int counter = 0;

    for (counter = 0; counter<iterations; counter++)
    {
        result += sign * (4.0/divisor);
        sign *= -1;
        divisor += 2;
    }

    return result;
}
