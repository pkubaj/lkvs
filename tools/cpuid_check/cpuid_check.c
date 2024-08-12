// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2022 Intel Corporation.
/*
 * cpuid_check.c: one CPU ID check tool for script usage
 *
 * Author: Pengfei Xu <Pengfei.xu@intel.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Macro used in check_id, what is it for? */
#define N 32
/* Seems unused */
#define M 40

int usage(char *progname)
{
	printf("%s NUM1 NUM2 NUM3 NUM4 CHAR5 NUM6\n", progname);
	printf("  NUM1: input EAX value in hex\n");
	printf("  NUM2: input EBX value in hex\n");
	printf("  NUM3: input ECX value in hex\n");
	printf("  NUM4: input EDX value in hex\n");
	printf("  CHAR5: a|b|c|d indicates the output matching EAX|EBX|ECX|EDX\n");
	printf("  NUM6: number of decimal digits in CHAR5 output register matching SPEC\n");
	printf("CET SHSTK cpuid check example:\n# %s 7 0 0 0 c 7\n", progname);
	exit(2);
}

/* cpuid checking function with assembly. */
static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
				unsigned int *ecx, unsigned int *edx)
{
/* Outputting values of EAX, EBX, ECX and EDX into eax, ebx, ecx and edx
 * Takes eax and ecx variables as input and makes sure that the output of the first and third operand goes into respectively eax and ecx variables
 * memory: tells the compiler that the assembly code performs memory reads or writes to items other than those listed in the input and output operands (for example, accessing the memory pointed to by one of the input parameters)
 */
	asm volatile("cpuid"
	: "=a" (*eax),
	"=b" (*ebx),
	"=c" (*ecx),
	"=d" (*edx)
	: "0" (*eax), "2" (*ecx)
	: "memory");
}

/* Convert hex to binary mode to display cpuid information.
 * n is a variable holding the value of a register */
int h_to_b(long n)
{
	int i = 0;
	static int a[N];

	for (i = 0; i != N; ++i) {
		a[N - 1 - i] = n % 2;
		n /= 2;
	}
	for (i = 0; i != N; ++i) {
		printf("%d", a[i]);
		if ((i + 1) % 4 == 0)
			printf(" ");
	}
	printf("\n");
	return 0;
}

/* Check that the cpuid target output bits are correct.
 * in my testing, n was always 2 ** 31 - 1 at the start of this function for CPU_XS_BAT_AESNI_CPU_ID_TEST test */
int check_id(long n, int ex_number)
{
	int i = 0;
	static int b[N];
	int bit_n = N - 1 - ex_number;

	/* N is 32 and this loop goes index N-1 to 0, filling the b array with 0's and 1's (so b could be switched to bool)
	 * looking at h_to_b(), the following loop seems to convert hex to binary
	 */
	for (i = 0; i != N; ++i) {
		b[N - 1 - i] = n % 2;
		n /= 2;
	}
	printf("Start with 0, pass: bit set 1, fail: bit set 0\n");
	/* Since n was 2 ** 31 - 1 at the beginning (for CPU_XS_BAT_AESNI_CPU_ID_TEST), the values of the whole array are predermined and are:
	 * b[32 - 1 - 0] = 2147417087 % 2 = 1
	 * b[32 - 1 - 1] = 1073708543 % 2 = 1
	 * b[32 - 1 - 2] = 536854271 % 2 = 1
	 * b[32 - 1 - 3] = 268427135 % 2 = 1
	 * b[32 - 1 - 4] = 134213567 % 2 = 1
	 * b[32 - 1 - 5] = 67106783 % 2 = 1
	 * b[32 - 1 - 6] = 33553391 % 2 = 1
	 * b[32 - 1 - 7] = 16776695 % 2 = 1
	 * b[32 - 1 - 8] = 8388347 % 2 = 1
	 * b[32 - 1 - 9] = 4194173 % 2 = 1
	 * b[32 - 1 - 10] = 2097086 % 2 = 0
	 * b[32 - 1 - 11] = 1048543 % 2 = 1
	 * b[32 - 1 - 12] = 524271 % 2 = 1
	 * b[32 - 1 - 13] = 262135 % 2 = 1
	 * b[32 - 1 - 14] = 131067 % 2 = 1
	 * b[32 - 1 - 15] = 65533 % 2 = 1
	 * b[32 - 1 - 16] = 32766 % 2 = 0
	 * b[32 - 1 - 17] = 16383 % 2 = 1
	 * b[32 - 1 - 18] = 8191 % 2 = 1
	 * b[32 - 1 - 19] = 4095 % 2 = 1
	 * b[32 - 1 - 20] = 2047 % 2 = 1
	 * b[32 - 1 - 21] = 1023 % 2 = 1
	 * b[32 - 1 - 22] = 511 % 2 = 1
	 * b[32 - 1 - 23] = 255 % 2 = 1
	 * b[32 - 1 - 24] = 127 % 2 = 1
	 * b[32 - 1 - 25] = 63 % 2 = 1
	 * b[32 - 1 - 26] = 31 % 2 = 1
	 * b[32 - 1 - 27] = 15 % 2 = 1
	 * b[32 - 1 - 28] = 7 % 2 = 1
	 * b[32 - 1 - 29] = 3 % 2 = 1
	 * b[32 - 1 - 30] = 1 % 2 = 1
	 * b[32 - 1 - 31] = 0 % 2 = 0
	 * This means that the test will always fail when the 6th argument is either 10, 16 or 31. */

	if (b[bit_n] == 1) {
		printf("Order bit:%d, invert order:%d, bit:%d, pass!\n",
		       bit_n, ex_number, b[bit_n]);
	} else {
		printf("Order bit:%d, invert order:%d, bit:%d, fail!\n",
		       bit_n, ex_number, b[bit_n]);
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	int ex_n, test_result = 1;
	char ex = 'e';

	if (argc == 1) {
		usage(argv[0]);
		exit(2);
	} else if (argc == 5) {
		if (sscanf(argv[1], "%x", &eax) != 1)
			usage(argv[0]);
		printf("4 parameters, eax=%d\n", eax);
		if (sscanf(argv[2], "%x", &ebx) != 1)
			usage(argv[0]);
		if (sscanf(argv[3], "%x", &ecx) != 1)
			usage(argv[0]);
		if (sscanf(argv[4], "%x", &edx) != 1)
			usage(argv[0]);
	} else if (argc == 7) {
		if (sscanf(argv[1], "%x", &eax) != 1)
			usage(argv[0]);
		printf("6 parameters, eax=%d\n", eax);
		if (sscanf(argv[2], "%x", &ebx) != 1)
			usage(argv[0]);
		if (sscanf(argv[3], "%x", &ecx) != 1)
			usage(argv[0]);
		if (sscanf(argv[4], "%x", &edx) != 1)
			usage(argv[0]);
		if (sscanf(argv[5], "%c", &ex) != 1)
			usage(argv[0]);
		if (sscanf(argv[6], "%d", &ex_n) != 1)
			usage(argv[0]);
	} else {
		if (sscanf(argv[1], "%x", &eax) != 1)
			usage(argv[0]);
		printf("Just get eax=%d\n", eax);
	}

	printf("cpuid(eax=%08x, ebx=%08x, ecx=%08x, edx=%08x)\n",
	       eax, ebx, ecx, edx);
	printf("cpuid(&eax=%p, &ebx=%p, &ecx=%p, &edx=%p)\n",
	       &eax, &ebx, &ecx, &edx);
	native_cpuid(&eax, &ebx, &ecx, &edx);
	printf("After native_cpuid:\n");
	printf("out:  eax=%08x, ebx=%08x, ecx=%08x,  edx=%08x\n",
	       eax, ebx, ecx, edx);
	printf("cpuid(&eax=%p, &ebx=%p, &ecx=%p, &edx=%p)\n",
	       &eax, &ebx, &ecx, &edx);
	printf("output:\n");
	printf("  eax=%08x    || Binary: ", eax);
	h_to_b(eax);
	printf("  ebx=%08x    || Binary: ", ebx);
	h_to_b(ebx);
	printf("  ecx=%08x    || Binary: ", ecx);
	h_to_b(ecx);
	printf("  edx=%08x    || Binary: ", edx);
	h_to_b(edx);

	printf("Now check cpuid e%cx, bit %d\n", ex, ex_n);
	if (ex == 'a') {
		test_result = check_id(eax, ex_n);
	} else if (ex == 'b') {
		test_result = check_id(ebx, ex_n);
	} else if (ex == 'c') {
		test_result = check_id(ecx, ex_n);
	} else if (ex == 'd') {
		test_result = check_id(edx, ex_n);
	} else {
		printf("No check point, not in a-d, skip.\n");
		test_result = 0;
	}

	printf("Done! Return:%d.\n\n", test_result);
	return test_result;
}
