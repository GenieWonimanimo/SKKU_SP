#include "sfp.h"
#include <stdlib.h>
#define MAX 	65504
#define MIN 	-65504
#define POS_INF 31744
#define NEG_INF 64512
#define BIAS	15

sfp int2sfp(int input){
	// if input is special value
	if (input > MAX)
		return POS_INF;
	if (input < MIN)
		return NEG_INF;
	// input is not denormalized value because input is int
	sfp res = 0; // 0 00000 0000000000, initial state
	// set sign bit
	if (input < 0) {
		res |= 1 << 15;
		input *= -1;
	}
	// set Mantissa
	int M = 0;
	int cnt = 0;
	while (input > 0) {
		M |= ((input % 2) << cnt++);
		input /= 2;
	}
	M = M & ~(1 << (--cnt)); // delete (cnt - 1)th bit (assume there is implied 1)
	res |= M << (10 - cnt);
	// set Exponent
	int exp = cnt + BIAS; // cnt is E(= exp - BIAS)
	res |= exp << 10;
	return res;
}

int sfp2int(sfp input){

}

sfp float2sfp(float input){
	// if input is special value
	if (input > MAX)
		return POS_INF;
	if (input < MIN)
		return NEG_INF;

	sfp res = 0; // 0 00000 0000000000
}

float sfp2float(sfp input){
}

sfp sfp_add(sfp a, sfp b){
	
}

sfp sfp_mul(sfp a, sfp b){
	
}

char* sfp2bits(sfp result){
	char* bits = (char*)malloc(17);
	for (int i = 15; i >= 0; i--) {
		bits[15 - i] = (result & (1 << i)) + '0';
	}
	bits[16] = '\0';
	return bits;
}


