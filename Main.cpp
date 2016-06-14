#include <LPC17xx.h>
#include <UART_LPC17xx.h>
#include <cmsis_os.h>
#include "ITM_ARM.h"
#include <string.h>
#include <Driver_ETH.h>
#include <Driver_ETH_MAC.h>
#include <Driver_ETH_PHY.h>
#include "rl_net.h"

extern ARM_DRIVER_ETH_MAC Driver_ETH_MAC0;
extern ARM_DRIVER_ETH_PHY Driver_ETH_PHY0;

static ARM_ETH_MAC_CAPABILITIES capabilities;
static ARM_DRIVER_ETH_MAC *mac;
static ARM_DRIVER_ETH_PHY *phy;
static ARM_ETH_MAC_ADDR macAddress;

const char mac_addr[]  = { "00-01-02-32-3c-46" };
const char ip_addr[]   = { "192.168.5.217"     };
const char def_gw[]    = { "192.168.5.1"       };
const char net_mask[]  = { "255.255.255.0"     };
const char pri_dns[]   = { "8.8.8.8"      };
const char sec_dns[]   = { "8.8.4.4"      };
const char host_name[] = { "RTS HARDWARE"          };
bool DHCP_enabled      = false;

void ethernetEvent(uint32_t event) {
//	itmPrintln("we are inside ethernetEvent");
}

void ethernetInitialize(void) {
//	mac = &Driver_ETH_MAC0;
//  phy = &Driver_ETH_PHY0;
//	capabilities = mac->GetCapabilities();
//	mac->PowerControl(ARM_POWER_FULL);
//	mac->GetMacAddress(macAddress);
//	mac->SetMacAddress(macAddress); 
//	mac->SetAddressFilter(macAddress, 0);
//	mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL);
//	mac->Initialize(ethernetEvent);
	
	mac = &Driver_ETH_MAC0;
  phy = &Driver_ETH_PHY0;
 
  capabilities = mac->GetCapabilities ();
   
  mac->Initialize (ethernetEvent);
  mac->PowerControl (ARM_POWER_FULL);
 
  if (capabilities.mac_address == 1)  {
    mac->SetMacAddress(&macAddress);
  }
  else {
    mac->GetMacAddress(&macAddress);
  }
 
  if (phy->Initialize (mac->PHY_Read, mac->PHY_Write) == ARM_DRIVER_OK) {
    phy->PowerControl (ARM_POWER_FULL);
    phy->SetInterface (ARM_ETH_INTERFACE_RMII);
    phy->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE);
  }
}

static ARM_ETH_LINK_STATE ethernet_link;  
 
void ethernet_check_link_status (void) {
  ARM_ETH_LINK_STATE link;
 
  link = phy->GetLinkState ();
  if (link == ethernet_link) {    
    return;                              
  }
                                       
  ethernet_link = link;   
  if (link == ARM_ETH_LINK_UP) {     
    ARM_ETH_LINK_INFO info = phy->GetLinkInfo ();
    mac->Control(ARM_ETH_MAC_CONFIGURE,
                 info.speed  << ARM_ETH_MAC_SPEED_Pos  |
                 info.duplex << ARM_ETH_MAC_DUPLEX_Pos |
                 ARM_ETH_MAC_ADDRESS_BROADCAST);
    mac->Control(ARM_ETH_MAC_CONTROL_TX, 1);
    mac->Control(ARM_ETH_MAC_CONTROL_RX, 1);
  }
  else {                     
    mac->Control(ARM_ETH_MAC_FLUSH, ARM_ETH_MAC_FLUSH_TX | ARM_ETH_MAC_FLUSH_RX);
    mac->Control(ARM_ETH_MAC_CONTROL_TX, 0);
    mac->Control(ARM_ETH_MAC_CONTROL_RX, 0);
  }
}

void ethernetConfig(void) {
	uint8_t buf[8];
 
  netInitialize ();
 
  /* Change host name */
  netSYS_SetHostName (host_name);
 
  /* Change MAC address */
  netMAC_aton (mac_addr, buf);
  netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionMAC_Address, buf, NET_ADDR_ETH_LEN);
 
  if (DHCP_enabled == false) {
    /* Static configuration mode */ 
    netDHCP_Disable (0);
 
    /* Change IP address */
    netIP_aton (ip_addr, NET_ADDR_IP4, buf);
    netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_Address, buf, NET_ADDR_IP4_LEN);
 
    /* Change Network mask */
    netIP_aton (net_mask, NET_ADDR_IP4, buf);
    netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_SubnetMask, buf, NET_ADDR_IP4_LEN);
 
    /* Change Default Gateway address */
    netIP_aton (def_gw, NET_ADDR_IP4, buf);
    netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_DefaultGateway, buf, NET_ADDR_IP4_LEN);
 
    /* Change Primary DNS Server address */
    netIP_aton (pri_dns, NET_ADDR_IP4, buf);
    netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_PrimaryDNS, buf, NET_ADDR_IP4_LEN);
 
    /* Change Secondary DNS Server address */
    netIP_aton (sec_dns, NET_ADDR_IP4, buf);
    netIF_SetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_SecondaryDNS, buf, NET_ADDR_IP4_LEN);
  }
}

void ledInitialize(void) {
	LPC_PINCON->PINSEL1 = 0x00;
	LPC_PINCON->PINMODE1 = 0x00;
	LPC_PINCON->PINMODE_OD1 = 0x00;
	LPC_GPIO1->FIODIR2 = 0xFF;
	LPC_GPIO1->FIOSET2 = 0XFF;
}                                                                                                                                      

void initializeThread(void const *arg) {
	ledInitialize();
}
osThreadDef(initializeThread, osPriorityNormal, 1, 0);

void heartBeatThread(void const *arg) {
	while (1) {
		LPC_GPIO1->FIOCLR2 = 0XFF;
		osDelay(70);
		LPC_GPIO1->FIOSET2 = 0XFF;
		osDelay(1000);
		ethernet_check_link_status();
	}
}
osThreadDef(heartBeatThread, osPriorityNormal, 1, 0);

int32_t tcp_sock;

uint32_t tcp_cb_func (int32_t socket, netTCP_Event event, const NET_ADDR *addr, const uint8_t *buf, uint32_t len) {
	itmPrintln("inside handler");
  switch (event) {
    case netTCP_EventConnect:
      if (addr->addr_type == NET_ADDR_IP4) {
        if (addr->addr[0] == 192  &&
            addr->addr[1] == 168  &&
            addr->addr[2] == 5    &&
            addr->addr[3] == 117) {
          return (1);
        }
      }
      return (0);
 
    case netTCP_EventEstablished:
      // Connection established
      break;
 
    case netTCP_EventClosed:
      // Connection was properly closed
      break;
 
    case netTCP_EventAborted:
      // Connection is for some reason aborted
      break;
 
    case netTCP_EventACK:
      // Previously sent data acknowledged
      break;
 
    case netTCP_EventData:
      // Data received
      if ((buf[0] == 0x01) && (len == 2)) {
        // Switch LEDs on and off
        // LED_SetOut (buf[1]);
      }
      break;
  }
  return (0);
}

void networkThread(const void *arg) {
	tcp_sock = netTCP_GetSocket (tcp_cb_func);
	if (tcp_sock >= 0) {
		netTCP_Listen (tcp_sock, 60300);
	}
}

int main(void) {
				
	SystemCoreClockUpdate ();
	SysTick_Config(SystemCoreClock/1000);
	
	osKernelInitialize();
	
	osThreadCreate(osThread(initializeThread), NULL);
	osThreadCreate(osThread(heartBeatThread), NULL);
	
	ethernetInitialize();
//	ethernetConfig();
//		netInitialize ();
		
		
	return 0;
}
