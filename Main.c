#include <LPC17xx.h>
#include <UART_LPC17xx.h>
//#include <SPI_LPC17xx.h>
#include <cmsis_os.h>
#include "ITM_ARM.h"

#define IER_RBR 1U << 0
#define IER_THRE 1U << 1
#define IER_RLS 1U << 2
#define BUFSIZE 100
#define RINGBUFFSIZE 100
#define RINGBUFFLENGTH (RINGBUFFSIZE + 1)

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;
ARM_USART_STATUS Driver_USART1_STATUS;
USART_TRANSFER_INFO Driver_USART1_INFO;

char hello[100];

char pData;
char temp[100];
char *readout = hello;

char ringBuffer[RINGBUFFLENGTH];
unsigned int head, tail;

void ringBufferInit(void) {
	head = tail = 0;
}

uint32_t ringBufferWrite(char in) {
	if (head == ((tail - 1 + RINGBUFFLENGTH) % 	RINGBUFFLENGTH)) {
		return 1;
	}
	
	ringBuffer[head] = in;
	head = (head + 1) % RINGBUFFLENGTH;
	return 0;
}

uint32_t ringBufferRead(char *out) {
	if (head == tail) {
		return 1;
	}
	*out = ringBuffer[tail];
//	ringBuffer[tail] = 0; // just for acknowledge;
	tail = ((tail + 1) % RINGBUFFLENGTH);
	return 0; 
}

void USART_callback(uint32_t event)
{
//		static int i;
	
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE: 
//				itmPrintln("receive complete");
					ringBufferWrite(pData);
//					ringBufferRead(readout);
//					itmPrint("readout:"); itmPrintlnInt(*readout);
//					i++;
//				itmPrintln(temp);
				break;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
			itmPrintln("transfer complete");
			break;
    case ARM_USART_EVENT_SEND_COMPLETE:
			itmPrintln("send complete");
			break;
    case ARM_USART_EVENT_TX_COMPLETE:
			itmPrintln("tx complete");
        /* Success: Wakeup Thread */
//        osSignalSet(tid_myUART_Thread, 0x01);
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
			itmPrintln("rx timeout");
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
			itmPrintln("rx overflow");
			break;
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
		break;		
	}
}

void uart0Initialize(void) {
        Driver_USART0.Initialize(USART_callback);
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
        Driver_USART1.Initialize(USART_callback);
        Driver_USART1.PowerControl(ARM_POWER_FULL);
        Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS| ARM_USART_DATA_BITS_8| ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,57600);
        Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
        Driver_USART1.Control(ARM_USART_CONTROL_RX,1);  

				NVIC_EnableIRQ(UART1_IRQn);
				LPC_UART1->IER |= IER_RBR | IER_THRE |IER_RLS; //Enable Interrupt
}

void uart1UnInitialize(void) {
	Driver_USART1.Uninitialize();
}

void SPI_callback(uint32_t event) {

}

//void spiInitialize(void) {
//	Driver_SPI2.Initialize(SPI_callback);
//	Driver_SPI2.PowerControl(ARM_POWER_FULL);
//	Driver_SPI2.Control(ARM_SPI_MODE_MASTER | ARM_SPI_MODE_MASTER_SIMPLEX | ARM_SPI_SET_BUS_SPEED | ARM_SPI_SS_MASTER_SW | ARM_SPI_CPOL0_CPHA1, 400000);
//	Driver_SPI2.Control(ARM_SPI_SET_DEFAULT_TX_VALUE, 0x00);
//	Driver_SPI2.Control(ARM_SPI_CONTROL_SS, 0); // Control slave select; arg = 0:inactive, 1:active
//}

void ledInitialize(void) {
	LPC_PINCON->PINSEL1 = 0x00;
	LPC_PINCON->PINMODE1 = 0x00;
	LPC_PINCON->PINMODE_OD1 = 0x00;
	LPC_GPIO1->FIODIR2 = 0xFF;
}

void writeThread(void const *arg) {
	while(1) {
		Driver_USART1.Send("AT", 2);
		osDelay(3000);
	}
}
osThreadDef(writeThread, osPriorityNormal, 1, 0);                                                                                                                                         

void initializeThread(void const *arg) {
	ringBufferInit();
	uart1Initialize();
//	spiInitialize();
	ledInitialize();
	itmPrintln("Initialized");
}
osThreadDef(initializeThread, osPriorityNormal, 1, 0);

void heartBeatThread(void const *arg) {
	while (1) {
		LPC_GPIO1->FIOCLR2 = 0XFF;
		osDelay(1000);
		LPC_GPIO1->FIOSET2 = 0XFF;
		osDelay(1000);
	}
}
osThreadDef(heartBeatThread, osPriorityNormal, 1, 0);

void readThread(void const *arg) {
		while (1) {
		Driver_USART1.Receive(&pData,1);
		if ( head != tail ) {
			ringBufferRead(readout);
			itmPrint(readout);
		}
	}
}
osThreadDef(readThread, osPriorityNormal, 1, 0);

int main(void) {
				
	SystemCoreClockUpdate ();
	SysTick_Config(SystemCoreClock/1000);
	
	osKernelInitialize();
	osThreadCreate(osThread(initializeThread), NULL);
	osThreadCreate(osThread(heartBeatThread), NULL);
	osThreadCreate(osThread(readThread), NULL);
	osThreadCreate(osThread(writeThread), NULL);
	
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
			
	return 0;
}
