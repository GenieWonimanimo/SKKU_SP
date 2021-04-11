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
	float fu1 = (float)u;
	float fu2 = bit2float(u);
	printBit((unsigned*)&fu1);
	printBit((unsigned*)&fu2);

	printf("*** 10.11 ***\n");
	unsigned uf1 = (unsigned)f;
	unsigned uf2 = float2bit(f);
	printBit(&uf1);
	printBit(&uf2);
	return 0;
}
