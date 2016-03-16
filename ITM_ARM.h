#ifndef _ITM_ARM_H_
#define _ITM_ARM_H_

#define ITM_Port8(n)   (*((volatile unsigned char *)(0xE0000000+4*n)))
	
void itmPrintln(volatile char *string);
void itmPrint(volatile char *string);
void itmPrintlnInt(volatile int integer);
void itmPrintInt(volatile int integer);
	
#endif
