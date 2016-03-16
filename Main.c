#include <LPC17xx.h>
#include <UART_LPC17xx.h>
#include <cmsis_os.h>
#include "ITM_ARM.h"

#define IER_RBR 1U << 0
#define IER_THRE 1U << 1
#define IER_RLS 1U << 2

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;
ARM_USART_STATUS Driver_USART1_STATUS;
USART_TRANSFER_INFO Driver_USART1_INFO;

void uart0Initialize(void) {
        Driver_USART0.Initialize(NULL);
        Driver_USART0.PowerControl(ARM_POWER_FULL);
        Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS| ARM_USART_DATA_BITS_8| ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,115200);
        Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
        Driver_USART0.Control(ARM_USART_CONTROL_RX,1);      

				NVIC_EnableIRQ(UART0_IRQn);
				LPC_UART0->IER = IER_RBR | IER_THRE |IER_RLS; //Enable Interrupt
}

void uart0UnInitialize(void) {
	Driver_USART0.Uninitialize();
}

void uart1Initialize(void) {
        Driver_USART1.Initialize(NULL);
        Driver_USART1.PowerControl(ARM_POWER_FULL);
        Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS| ARM_USART_DATA_BITS_8| ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,115200);
        Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
        Driver_USART1.Control(ARM_USART_CONTROL_RX,1);  

				NVIC_EnableIRQ(UART1_IRQn);
				LPC_UART1->IER = IER_RBR | IER_THRE |IER_RLS; //Enable Interrupt
}

void uart1UnInitialize(void) {
	Driver_USART1.Uninitialize();
}

//void UART0_IRQHandler(void) {
//	if ((LPC_UART0->IIR & 0x0000000F) == 0x02) {
//		itmPrintln("inside Interrupt");
//	}
//}

int main() {
			
	SystemCoreClockUpdate ();
	SysTick_Config(SystemCoreClock/1000);
	
//	LPC_PINCON->PINSEL1 = 0x00;
//	LPC_PINCON->PINMODE1 = 0x00;
//	LPC_PINCON->PINMODE_OD1 = 0x00;
//	LPC_GPIO1->FIODIR2 = 0xFF;
//	LPC_GPIO1->FIOSET2 = 0XFF;
//	LPC_GPIO1->FIOCLR2 = 0XFF;
//	LPC_GPIO1->FIOSET2 = 0XFF;
	
//	LPC_SC->PCONP = 0x10;
//	LPC_SC->PCLKSEL0 = 0x00;  //PCLK_peripheral = CCLK/4
//	LPC_UART1->LCR = 0x83;
//	LPC_UART1->DLL = 0x5D;
//	LPC_UART1->DLM = 0x00;
//	LPC_UART1->FCR = 0x00;
//	LPC_UART1->FDR = 0x34;
//	LPC_UART1->LCR = 0x03;
//	LPC_PINCON->PINSEL4 = 0x0A;
//	LPC_PINCON->PINMODE4 = 0x00;
//	
//	LPC_UART1->TER = 0x80;
//	LPC_UART1->THR = 'a';
//	LPC_UART1->THR = 'o';
	
//	uart0Initialize();
	uart1Initialize();

	while (1) {
//		Driver_USART1.Receive(pData, 4);
//		Driver_USART1.Send("hey!\r", 5);
//		Driver_USART1.Receive(pData, 5);
//		itmPrintln(pData);
////		Driver_USART1.Send("hello", 5);
//		itmPrintln("hello");
		
//		osDelay(1000);
	}
	
	return 0;
}

void interruptDefinition(USART_RESOURCES *us1) {
	itmPrintlnInt(us1->uart_reg->IIR);
}
