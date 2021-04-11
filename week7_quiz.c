#include <stdio.h>

typedef union {
	float f;
	unsigned u;
} bit_float_t;

// same as (float) u?
float bit2float(unsigned u) {
	bit_float_t arg;
	arg.u = u;
	return arg.f;
}

// same as (unsigned) f?
unsigned float2bit(float f) {
	bit_float_t arg;
	arg.f = f;
	return arg.u;
}

// print bit pattern of 4 byte data
void printBit(unsigned* input) {
 	for (int i = 31; i >= 0; i--) {
		printf("%d", (*input >> i) & 1);
		if (i % 8 == 0)
			printf(" ");
	}
	printf("\n");
}

int main(void) {
	// test with 10 and 10.11
	unsigned u = 10;
	float f = 10.11;

	printf("*** 10 ***\n");
	printBit((unsigned*)&((float)u));
	printBit((unsigned*)&(bit2float(u)));

	printf("*** 10.11 ***\n");
	printBit(&((unsigned)f));
	printBit(&(float2bit(f)));
	return 0;
}
