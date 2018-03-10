/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>
#include "r_ether_rx_config.h"

/* Prevent Renesas headers redefining some stdint.h types. */
#define __TYPEDEF__	1

/*-----------------------------------------------------------
* Application specific definitions.
*
* These definitions should be adjusted for your particular hardware and
* application requirements.
*
* THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
* FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
* http://www.freertos.org/a00110.html
*
* The bottom of this file contains some constants specific to running the UDP
* stack in this demo.  Constants specific to FreeRTOS+TCP itself (rather than
* the demo) are contained in FreeRTOSIPConfig.h.
*----------------------------------------------------------*/

/* FIX ME: Uncomment and set to the specifications for your MCU. */
#define configCPU_CLOCK_HZ				( 120000000UL ) /*_RB_ guess*/
#define configPERIPHERAL_CLOCK_HZ		( 60000000UL ) /*_RB_ guess*/

#define configSUPPORT_STATIC_ALLOCATION            1
/* 
 * Setting configSUPPORT_STATIC_ALLOCATION = 0 causes following link errors.
 * E0562310:Undefined external symbol "_xSemaphoreCreateBinaryStatic" referenced in ".\lib\aws\shadow\aws_shadow.obj"
 * E0562310:Undefined external symbol "_xTaskCreateStatic" referenced in ".\lib\aws\mqtt\aws_mqtt_agent.obj"
 * E0562310:Undefined external symbol "_xSemaphoreCreateMutexStatic" referenced in ".\lib\aws\shadow\aws_shadow.obj"
 * E0562310:Undefined external symbol "_xQueueCreateStatic" referenced in ".\application_code\common_demos\source\aws_shadow_lightbulb_on_off.obj"
 * E0562310:Undefined external symbol "_xQueueCreateStatic" referenced in ".\lib\aws\mqtt\aws_mqtt_agent.obj"
 * E0562310:Undefined external symbol "_xQueueCreateStatic" referenced in ".\lib\aws\ota\aws_ota_agent.obj"
 */

#define configUSE_DAEMON_TASK_STARTUP_HOOK         0
#define configENABLE_BACKWARD_COMPATIBILITY        0
#define configUSE_PREEMPTION                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    0
#define configMAX_PRIORITIES                       ( 7 )
#define configTICK_RATE_HZ                         ( 1000 )
#define configMINIMAL_STACK_SIZE                   ( ( unsigned short ) 140 )
#define configTOTAL_HEAP_SIZE                      ( ( size_t ) ( 200 * 1024U ) )
#define configMAX_TASK_NAME_LEN                    ( 15 )
#define configUSE_TRACE_FACILITY                   1
#define configUSE_16_BIT_TICKS                     0
#define configIDLE_SHOULD_YIELD                    1
#define configUSE_CO_ROUTINES                      0
#define configUSE_MUTEXES                          1
#define configUSE_RECURSIVE_MUTEXES                1
#define configQUEUE_REGISTRY_SIZE                  0
#define configUSE_APPLICATION_TASK_TAG             0
#define configUSE_COUNTING_SEMAPHORES              1
#define configUSE_ALTERNATIVE_API                  0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS    3      /* FreeRTOS+FAT requires 2 pointers if a CWD is supported. */
#define configRECORD_STACK_HIGH_ADDRESS            1

/* Hook function related definitions. */
#define configUSE_TICK_HOOK                        0
#define configUSE_IDLE_HOOK                        1
#define configUSE_MALLOC_FAILED_HOOK               1
#define configCHECK_FOR_STACK_OVERFLOW             0      /* Not applicable to the Win32 port. */

/* Software timer related definitions. */
#define configUSE_TIMERS                           1
#define configTIMER_TASK_PRIORITY                  ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                   5
#define configTIMER_TASK_STACK_DEPTH               ( configMINIMAL_STACK_SIZE * 2 )

/* Event group related definitions. */
#define configUSE_EVENT_GROUPS                     1


/* The interrupt priority used by the kernel itself for the tick interrupt and
the pended interrupt.  This would normally be the lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY         1

/* The maximum interrupt priority from which FreeRTOS API calls can be made.
Interrupts that use a priority above this will not be effected by anything the
kernel is doing. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    7

/* The peripheral used to generate the tick interrupt is configured as part of
the application code.  This constant should be set to the vector number of the
peripheral chosen.  As supplied this is CMT0. */
#define configTICK_VECTOR						_CMT0_CMI0

/* Set the timer channel used for tick interrupt of FreeRTOS */
#define configFreeRTOS_CLOCK_TIMER              0 // TICK TIMER = CMT0

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			1
#define INCLUDE_vTaskDelete					1
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_uxTaskGetStackHighWaterMark	1
#define INCLUDE_xTaskGetSchedulerState		1
#define INCLUDE_eTaskGetState				1
#define INCLUDE_xTimerPendFunctionCall		1
#define INCLUDE_xTaskResume                 1
#define INCLUDE_xTaskResumeFromISR          1
#define INCLUDE_xTaskGetCurrentTaskHandle   1

void vAssertCalled( void );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled()

/* Override some of the priorities set in the common demo tasks.  This is
required to ensure flase positive timing errors are not reported. */
#define bktPRIMARY_PRIORITY		( configMAX_PRIORITIES - 3 )
#define bktSECONDARY_PRIORITY	( configMAX_PRIORITIES - 4 )
#define intqHIGHER_PRIORITY		( configMAX_PRIORITIES - 3 )


/*********************************************
 * Amazon FreeRTOS aws_demos project
 ********************************************/

#define __FUNCTION__ __func__ /* GCC <--> CC-RX */
#define __attribute__( attr ) /* GCC <--> CC-RX */

void vMainUARTPrintString( char * pcString );
void vLoggingPrintf( const char * pcFormat, ... );

#define configPRINT_STRING( x )    vMainUARTPrintString( x );

#define configLOGGING_MAX_MESSAGE_LENGTH            100

#define configLOGGING_INCLUDE_TIME_AND_TASK_NAME    1

/*********************************************
 * FreeRTOS specific demos
 ********************************************/

/* The address of an echo server that will be used by the two demo echo client
 * tasks:
 * http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_Echo_Clients.html,
 * http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/UDP_Echo_Clients.html. */
#define configECHO_SERVER_ADDR0              192
#define configECHO_SERVER_ADDR1              168
#define configECHO_SERVER_ADDR2              2
#define configECHO_SERVER_ADDR3              6
#define configTCP_ECHO_CLIENT_PORT           7

/* Default MAC address configuration.  The demo creates a virtual network
 * connection that uses this MAC address by accessing the raw Ethernet/WiFi data
 * to and from a real network connection on the host PC.  See the
 * configNETWORK_INTERFACE_TO_USE definition above for information on how to
 * configure the real network connection to use. */
#define configMAC_ADDR0                      0x74
#define configMAC_ADDR1                      0x90
#define configMAC_ADDR2                      0x50
#define configMAC_ADDR3                      0x00
#define configMAC_ADDR4                      0x79
#define configMAC_ADDR5                      0x03

/* Default IP address configuration.  Used in ipconfigUSE_DHCP is set to 0, or
 * ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted.
 * (no effective for RX MCU sample configuration) */
#define configIP_ADDR0                       0
#define configIP_ADDR1                       0
#define configIP_ADDR2                       0
#define configIP_ADDR3                       0

/* Default gateway IP address configuration.  Used in ipconfigUSE_DHCP is set to
 * 0, or ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted.
 * (no effective for RX MCU sample configuration) */
#define configGATEWAY_ADDR0                  0
#define configGATEWAY_ADDR1                  0
#define configGATEWAY_ADDR2                  0
#define configGATEWAY_ADDR3                  0

/* Default DNS server configuration.  OpenDNS addresses are 208.67.222.222 and
 * 208.67.220.220.  Used in ipconfigUSE_DHCP is set to 0, or ipconfigUSE_DHCP is
 * set to 1 but a DNS server cannot be contacted.
 * (no effective for RX MCU sample configuration) */
#define configDNS_SERVER_ADDR0               0
#define configDNS_SERVER_ADDR1               0
#define configDNS_SERVER_ADDR2               0
#define configDNS_SERVER_ADDR3               0

/* Default netmask configuration.  Used in ipconfigUSE_DHCP is set to 0, or
 * ipconfigUSE_DHCP is set to 1 but a DNS server cannot be contacted.
 * (no effective for RX MCU sample configuration) */
#define configNET_MASK0                      0
#define configNET_MASK1                      0
#define configNET_MASK2                      0
#define configNET_MASK3                      0

#endif /* FREERTOS_CONFIG_H */
