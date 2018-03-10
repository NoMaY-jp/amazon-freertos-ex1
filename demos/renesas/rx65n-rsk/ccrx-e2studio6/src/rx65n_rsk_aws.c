/***********************************************************************
*
*  FILE        : rx65n_rsk_aws.c
*  DATE        : 2018-02-13
*  DESCRIPTION : Main Program
*
*  NOTE:THIS IS A TYPICAL EXAMPLE.
*
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include "r_smc_entry.h"
#include "Pin.h"
#include "r_simple_graphic_if.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "task.h"

#define DEBUG_PRINT


extern void lcd_open(void);
extern void freertos_main(void);

#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 5 )
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 15 )

#define MY_MAC_ADDR0    0x74,0x90,0x50,0x00,0x79,0x03

static const uint8_t renesas_mac_address[6] = {configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5};

void main(void);

void main(void)
{
	char buff[256];
	uint32_t main_loop_count = 0;

	R_Pins_Create();
	lcd_open();

	R_SIMPLE_GRAPHIC_Open();

	/* hello message */
	sprintf(buff, "hello, this is Amazon FreeRTOS on RX65N RSK system log.\n");
	printf("%s", buff);
	sprintf(buff, "built in %s, %s\n", __DATE__, __TIME__);
	printf("%s", buff);
	sprintf(buff, "\r\n");
	printf("%s", buff);


    FreeRTOS_IPInit(0, 0, 0, 0, renesas_mac_address);
    while(1)
    {
    	vTaskDelay(10000);
    	sprintf(buff, "main loop keeps alive. 10 seconds report count = %d\n", main_loop_count++);
    	printf("%s", buff);
    }
}


#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )

const char * pcApplicationHostnameHook( void )
{
    /* Assign the name "FreeRTOS" to this network node.  This function will
     * be called during the DHCP: the machine will be registered with an IP
     * address plus this name. */
    return "RX65N_FREERTOS_TCP_TEST_BY_SHELTY";
}
#endif

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    char cBuffer[ 16 ];
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        /* Print out the network configuration, which may have come from a DHCP
         * server. */
        FreeRTOS_GetAddressConfiguration(
            &ulIPAddress,
            &ulNetMask,
            &ulGatewayAddress,
            &ulDNSServerAddress );
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        FreeRTOS_printf( ( "\n\nIP Address: %s\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        FreeRTOS_printf( ( "Subnet Mask: %s\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        FreeRTOS_printf( ( "Gateway Address: %s\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        FreeRTOS_printf( ( "DNS Server Address: %s\n", cBuffer ) );
    }
}
