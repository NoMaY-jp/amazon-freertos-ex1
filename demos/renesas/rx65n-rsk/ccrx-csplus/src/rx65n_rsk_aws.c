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

/* Demo includes */
#include "aws_demo_runner.h"
#include "aws_system_init.h"

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

    if( SYSTEM_Init() == pdPASS )
    {
    	FreeRTOS_IPInit(0, 0, 0, 0, renesas_mac_address);
    	vTaskDelay(2000);
    	DEMO_RUNNER_RunDemos();
    }


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

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetTimerTaskMemory() to provide the memory that is
 * used by the RTOS daemon/time task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/
