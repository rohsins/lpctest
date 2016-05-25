#include <LPC17xx.h>
#include <UART_LPC17xx.h>
#include <cmsis_os.h>
#include "ITM_ARM.h"
#include <string.h>
#include "RingBuffer.h"

#define IER_RBR 1U << 0
#define IER_THRE 1U << 1
#define IER_RLS 1U << 2
//#define BUFSIZE 100
//#define RINGBUFFSIZE 100
//#define RINGBUFFLENGTH (RINGBUFFSIZE + 1)

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;
//extern ARM_DRIVER_SPI Driver_SPI0;
ARM_USART_STATUS Driver_USART1_STATUS;
USART_TRANSFER_INFO Driver_USART1_INFO;

char pData;
char temp[100];
char *readout = temp;
char ne[25];
int wh=0;
char checkstring[25]={'+','I','P','D',',','0',',','1','3',':','\0','\0','L','I','G','H','T','O','N','E',':','7','1','0',','};
unsigned int flag = 0;

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
//			itmPrintln("transfer complete");
			break;
    case ARM_USART_EVENT_SEND_COMPLETE:
//			itmPrintln("send complete");
			break;
    case ARM_USART_EVENT_TX_COMPLETE:
//			itmPrintln("tx complete");
        /* Success: Wakeup Thread */
//        osSignalSet(tid_myUART_Thread, 0x01);
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
//			itmPrintln("rx timeout");
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
//			itmPrintln("rx overflow");
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
        Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS| ARM_USART_DATA_BITS_8| ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,115200);
        Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
        Driver_USART1.Control(ARM_USART_CONTROL_RX,1);  

				NVIC_EnableIRQ(UART1_IRQn);
				LPC_UART1->IER |= IER_RBR | IER_THRE |IER_RLS; //Enable Interrupt
}

void uart1UnInitialize(void) {
	Driver_USART1.Uninitialize();
}

void SPI_callback(uint32_t event) {
//	itmPrintln("inside SPI Callback");
}

//void spi0Initialize(void) {
//	Driver_SPI0.Initialize(SPI_callback);
//	Driver_SPI0.PowerControl(ARM_POWER_FULL);
//	Driver_SPI0.Control(ARM_SPI_MODE_MASTER | ARM_SPI_MODE_MASTER_SIMPLEX | ARM_SPI_SET_BUS_SPEED | ARM_SPI_SS_MASTER_SW | ARM_SPI_CPOL0_CPHA1, 400000);
//	Driver_SPI0.Control(ARM_SPI_SET_DEFAULT_TX_VALUE, 0x00);
//	Driver_SPI0.Control(ARM_SPI_CONTROL_SS, 0); // Control slave select; arg = 0:inactive, 1:active
//}

void ledInitialize(void) {
	LPC_PINCON->PINSEL1 = 0x00;
	LPC_PINCON->PINMODE1 = 0x00;
	LPC_PINCON->PINMODE_OD1 = 0x00;
	LPC_GPIO1->FIODIR2 = 0xFF;
	
	LPC_GPIO1->FIOSET2 = 0XFF;
}

void writeThread(void const *arg) {
	Driver_USART0.Send("AT+CWSAP=\"HWR&D\",\"onetest\",3,0\r\n",32);
	osDelay(1000);
	Driver_USART0.Send("AT+CWMODE=2\r\n",13);
	osDelay(1000);
	Driver_USART0.Send("AT\r\n", 4);
	osDelay(1000);
	Driver_USART0.Send("AT+CIPMUX=1\r\n", 13);
	osDelay(1000);
	Driver_USART0.Send("AT+CIPSERVER=1,60300\r\n", 22);
	osDelay(1000);
	Driver_USART0.Send("AT+CIPSTO=1\r\n", 13);
	osDelay(1000);
	
//	while(1) {
//		Driver_USART0.Send("AT\r", 3);
//		osDelay(3000);
//	}
}
osThreadDef(writeThread, osPriorityNormal, 1, 0);                                                                                                                                         

void initializeThread(void const *arg) {
	ringBufferInit();
	uart0Initialize();
	ledInitialize();
//	spi0Initialize();
//	itmPrintln("Initialized");
}
osThreadDef(initializeThread, osPriorityNormal, 1, 0);

void heartBeatThread(void const *arg) {
	while (1) {

		osDelay(1000);
		
		if (strcmp(ne, checkstring) == 0 ) {
			LPC_GPIO1->FIOCLR2 = 0XFF;
		}
		else {
			LPC_GPIO1->FIOSET2 = 0XFF;
		}
	}
}
osThreadDef(heartBeatThread, osPriorityNormal, 1, 0);

void readThread(void const *arg) {
		flag = head;
		while (1) {
		Driver_USART0.Receive(&pData,1);
		if ( head != tail ) {
			ringBufferRead(readout);
			itmPrint(readout);
			ne[wh]=*readout;
			wh++;
			if ( *readout == '\n' ) wh = 0;
		}
	}
}
osThreadDef(readThread, osPriorityNormal, 1, 0);

//void refreshThread(void const *arg) {
//}
//osThreadDef(refreshThread, osPriorityNormal, 1, 0);

//void spiDoThread(void const *arg) {
//	while (1) {
//		Driver_SPI0.Control(ARM_SPI_CONTROL_SS, 1);
//		Driver_SPI0.Send("hello", 5);
//		Driver_SPI0.Control(ARM_SPI_CONTROL_SS, 0);
//		itmPrintln("SPI Data Sent");
//		osDelay(3000);
//	}
//}
//osThreadDef(spiDoThread, osPriorityNormal, 1, 0);

int main(void) {
	
	char oetest[] = "heyadfgijfdsl;kfjasd\nakl;sdjf;asdl";
	char readTest[40];
				
	SystemCoreClockUpdate ();
	SysTick_Config(SystemCoreClock/1000);
	
	RingBuffer ringBufferVariable;
	RingBuffer ringBufferVariable2;
	ringBufferVariable.ringBufferStringWrite(oetest);
	ringBufferVariable2.ringBufferStringWrite("Hey");	
	ringBufferVariable.ringBufferStringRead(readTest);
	
	osKernelInitialize();
	osThreadCreate(osThread(initializeThread), NULL);
	osThreadCreate(osThread(heartBeatThread), NULL);
	osThreadCreate(osThread(readThread), NULL);
	osThreadCreate(osThread(writeThread), NULL);
//	osThreadCreate(osThread(refreshThread), NULL);
//	osThreadCreate(osThread(spiDoThread), NULL);
	
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
