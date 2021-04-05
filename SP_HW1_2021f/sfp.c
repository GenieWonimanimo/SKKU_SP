#include "sfp.h"
#include <stdlib.h>
#define MAX 		65504
#define MIN 		-65504
#define D_MAX		0.000060
#define D_MIN		-0.000060
#define POS_INF 	31744
#define NEG_INF 	64512
#define NAN			0xffff
#define TMAX		0x7fffffff
#define TMIN		-2147483648
#define BIAS		15

double Pow(double a, int b) {
	if (b == 0)
		return 1;
    double res = Pow(a, b / 2);
    if (b % 2 == 0)
        return res * res;
    return res * res * (b > 0 ? a : (1 / a));
}

sfp int2sfp(int input){
	// if input is special value
	if (input > MAX)
		return POS_INF;
	if (input < MIN)
		return NEG_INF;
	if (!(MIN <= input && input <= MAX))
		return NAN;
	// if input is denormalized value
	if (input == 0)
		return 0;
	sfp res = 0; // 0 00000 0000000000, initial state
	// set sign bit
	if (input < 0) {
		res |= 1 << 15;
		input *= -1;
	}
	// get cnt(how many times input can divided by 2)
	int M = input;
	int cnt = 0;
	while (input > 0) {
		cnt++;
		input /= 2;
	}
	// set Exponent
	int exp = (cnt - 1) + BIAS; // cnt - 1 is E(= exp - BIAS)
	res |= exp << 10;
	// set Mantissa
	if (cnt > 11) {
		M >>= (cnt - 11);
		cnt = 11;
	}
	M = M & ~(1 << (--cnt)); // delete (cnt - 1)th bit (assume there is implied 1)
	res |= M << (10 - cnt);
	return res;
}

int sfp2int(sfp input){
	// if sfp is special value
	if (input == POS_INF)
		return TMAX;
	if (input == NEG_INF || input == NAN)
		return TMIN;

	// get sign, if sign bit is 1, res is negative int
	int s = 1;
	if (((input >> 15) & 1) == 1) {
		s = -1;
		input = input & ~(1 << 15);
	}
	// get Mantissa
	double M = 0;
	double tmp = 0.5;
	for (int i = 9; i >= 0; i--) {
		M += ((input >> i) & 1) * tmp;
		tmp /= 2;
	}
	M += 1;
	// get Exponent
	int E = (input >> 10) - BIAS; // E = exp - BIAS
	return s * M * Pow(2, E);
}

sfp float2sfp(float input){
	// if input is special value
	if (input > MAX)
		return POS_INF;
	if (input < MIN)
		return NEG_INF;
	if (!(MIN <= input && input <= MAX))
		return NAN;

	sfp res = 0; // 0 00000 0000000000
	int* pf = (int*)&input;
	// set sign bit
	if (input < 0) {
		res |= 1 << 15;
		input *= -1;
	}
	// set Exponent
	int E = 1 - BIAS; // set denormalized case as default
	if (input > D_MAX) { // if input is normalized value
		E = (*pf >> 23) - 127;
		int exp = E + BIAS;
		res |= exp << 10;
	}
	// set Mantissa
	int M = 1;
	double fracVal = input / Pow(2, E) - (input > D_MAX ? 1 : 0);
	for (int i = 0; i < 10; i++) {
		M <<= 1;
		fracVal *= 2;
		if (fracVal >= 1) {
			M |= 1;
			fracVal -= 1;
		}
	}
	M = M & ~(1 << 10);
	res |= M;
	return res;
}

float sfp2float(sfp input){
	// if sfp is special value
	if (input == POS_INF)
		return TMAX;
	if (input == NEG_INF || input == NAN)
		return TMIN;
	// get sign, if sign bit is 1, res is negative int
	int s = 1;
	if (((input >> 15) & 1) == 1) {
		s = -1;
		input = input & ~(1 << 15);
	}
	// get Mantissa
	double M = 0;
	double tmp = 0.5;
	for (int i = 9; i >= 0; i--) {
		M += ((input >> i) & 1) * tmp;
		tmp /= 2;
	}
	M += (input <= 1023) ? 0 : 1; // if input is denormalized value, 1023 == 0 00000 1111111111
	// get Exponent
	int E;
	if (input <= 1023) // if input is denormalized value, E = 1 - BIAS
		E = 1 - BIAS;
	else // else, E = exp - BIAS
		E = (input >> 10) - BIAS;
	return s * M * Pow(2, E);
}

// shift n bit to right side.(round to even)
sfp shift_rte(unsigned long long m, int n) {
	if (((m >> (n - 1)) & 1) == 1) {
		if (((m >> n) & 1) == 1) 
			m += (1 << (n - 1));
		else {
			for (int i = 0; i < n - 1; i++) {
				if (((m >> i) & 1) == 1) {
					m += (1 << (n - 1));
					break;
				}
			}
		}
	}
	m >>= n;
	return m;
}

sfp sfp_add(sfp a, sfp b){
	// if a or b is special value
	if (a == NAN || b == NAN)
		return NAN;
	if (a == POS_INF || b == POS_INF) {
		if (a == NEG_INF || b == NEG_INF)
			return NAN;
		return POS_INF;
	}
	if (a == NEG_INF || b == NEG_INF)
		return NEG_INF;
	// get sign, exponent, mantissa of a, b
	int s1 = (a >> 15) & 1;
	int s2 = (b >> 15) & 1;
	int exp1 = (a & ~(1 << 15)) >> 10;
	int exp2 = (b & ~(1 << 15)) >> 10;
	int E1 = 1 - BIAS;
	int E2 = 1 - BIAS;
	sfp m1_ = a;
	m1_ <<= 6;
	m1_ >>= 6;
	sfp m2_ = b;
	m2_ <<= 6;
	m2_ >>= 6;
	if (exp1 != 0) {
		E1 = exp1 - BIAS;
		m1_ |= (1 << 10);
	}
	if (exp2 != 0) {
		E2 = exp2 - BIAS;
		m2_ |= (1 << 10);
	}
	unsigned long long m1 = (unsigned long long)m1_ << 30;
	unsigned long long m2 = (unsigned long long)m2_ << 30;
	// normalize
	if (E1 < E2) {
		m1 >>= E2 - E1;
		E1 = E2;
	}
	else if (E2 < E1) {
		m2 >>= E1 - E2;
		E2 = E1;
	}
	sfp res = 0;
	long long resS, resM, resE;
	// if a and b has same sign bit
	if (s1 == s2) {
		resS = s1;
		resM = shift_rte(m1 + m2, 30);
		resE = E1;
		// normalize
		if (((resM >> 11) & 1) == 1) {
			resM = shift_rte(resM, 1);
			resE++;
			if (((resM >> 11) & 1) == 1) {
				resM >>= 1;
				resE++;
			}
		}
	}
	// if a and b has different sign bit
	else {
		resS = (m1 > m2) ? 0 : 1;
		resM = (m1 > m2) ? m1 - m2 : m2 - m1;
		resM = shift_rte(resM, 30);
		resE = E1;
		if (resM == 0)
			return 0;
		// normalize
		while (((resM >> 10) & 1) != 1) {
			resM <<= 1;
			resE--;
		}
	}
	// specialized value
	if (resE > 15)
		return (resS == 0 ? POS_INF : NEG_INF);
	// denoramlized value
	else if (resE <= 1 - BIAS) {
		resM = shift_rte(resM, (1 - BIAS) - resE);
		res |= resS << 15;
		res |= resM;
	}
	// normalized value
	else {
		resM = resM & ~(1 << 10);
		int resExp = resE + BIAS;
		res |= resS << 15;
		res |= resM;
		res |= resExp << 10;
	}
	return res;
}

sfp sfp_mul(sfp a, sfp b){
	// if a or b is special value
	if (a == NAN || b == NAN)
		return NAN;
	if (a == POS_INF || a == NEG_INF || b == POS_INF || b == NEG_INF) {
		if (a == 0 || b == 0)
			return NAN;
		if (((a >> 15) & 1) == ((b >> 15) & 1))
			return POS_INF;
		return NEG_INF;	
	}
	if (a == 0 || b == 0)
		return 0;
	// get sign, exponent, mantissa of a, b
	int s1 = (a >> 15) & 1;
	int s2 = (b >> 15) & 1;
	int exp1 = (a & ~(1 << 15)) >> 10;
	int exp2 = (b & ~(1 << 15)) >> 10;
	int E1 = 1 - BIAS;
	int E2 = 1 - BIAS;
	sfp m1 = a;
	m1 <<= 6;
	m1 >>= 6;
	sfp m2 = b;
	m2 <<= 6;
	m2 >>= 6;
	if (exp1 != 0) { // if a is normalized value
		E1 = exp1 - BIAS;
		m1 |= (1 << 10); // there is implied 1
	}
	else { // denormalized value
		while (((m1 >> 10) & 1) == 0) { // make m1 1.xxxx
			m1 <<= 1;
			E1--;
		}
	}
	if (exp2 != 0) { // if b is normalized value
		E2 = exp2 - BIAS;
		m2 |= (1 << 10); // there is implied 1
	}
	else { // denormalized value
		while (((m2 >> 10) & 1) == 0) { // make m2 1.xxxx
			m2 <<= 1;
			E2--;
		}
	}
	// multiply Mantissa and add Exponent
	int res = 0;
	int resS = (s1 == s2) ? 0 : 1;
	unsigned long long resM = (unsigned long long)m1 * m2;
	int resE = E1 + E2;
	// normalize
	resM = shift_rte(resM, 10);
	if (((resM >> 12) & 1) == 1) {
		resM = shift_rte(resM, 1);
		resE++;
		if (((resM >> 12) & 1) == 1) {
			resM >>= 1;
			resE++;
		}
	}
	if (((resM >> 11) & 1) == 1) {
		resM = shift_rte(resM, 1);
		resE++;
		if (((resM >> 11) & 1) == 1) {
			resM >>= 1;
			resE++;
		}
	}
	// if result exceeds the range of sfp
	if (resE > 15)
		return (resS == 0 ? POS_INF : NEG_INF);
	// try to represent a * b as denormalized value
	while (resE < -14 && resM > 0) {
		resM >>= 1;
		resE++;
	}
	// too small to represent
	if (resE < -14)
		return 0;
	// a * b is can be represented as denorm or norm
	res |= resS << 15;
	// if a * b is denormalized value
	if (resE == -14 && ((resM >> 10) & 1) == 0) {
		res |= resM;
		return res;
	}
	// a * b is normalized value
	resM = resM & ~(1 << 10);
	res |= resM;
	int resExp = resE + BIAS;
	res |= resExp << 10;
	return res;
}

char* sfp2bits(sfp result){
	char* bits = (char*)malloc(17);
	for (int i = 15; i >= 0; i--) {
		bits[15 - i] = ((result >> i) & 1) + '0';
	}
	bits[16] = '\0';
	return bits;
}


