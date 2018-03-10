/*
FreeRTOS+TCP V2.0.1
Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://aws.amazon.com/freertos
 http://www.FreeRTOS.org
*/

/* Standard includes. */
#include <stdio.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_IP_Private.h"

/* Hardware includes. */
#include "r_ether_rx_if.h"

/* Demo includes. */
#include "NetworkInterface.h"

/* include FIT module */
#include "platform.h"
#include "r_cmt_rx_if.h"

#if ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES != 1
	#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eProcessBuffer
#else
	#define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer ) eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

/* When a packet is ready to be sent, if it cannot be sent immediately then the
task performing the transmit will block for niTX_BUFFER_FREE_WAIT
milliseconds.  It will do this a maximum of niMAX_TX_ATTEMPTS before giving
up. */
#define niTX_BUFFER_FREE_WAIT	( ( TickType_t ) 2UL / portTICK_PERIOD_MS )
#define niMAX_TX_ATTEMPTS		( 5 )

/* The length of the queue used to send interrupt status words from the
interrupt handler to the deferred handler task. */
#define niINTERRUPT_QUEUE_LENGTH	( 10 )

#define ETHER_BUFSIZE_MIN 60

/*-----------------------------------------------------------*/

/*
 * A deferred interrupt handler task that processes
 */
extern void vEMACHandlerTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used to communicate Ethernet events with the IP task. */
extern QueueHandle_t xNetworkEventQueue;

/* The semaphore used to wake the deferred interrupt handler task when an Rx
interrupt is received. */
SemaphoreHandle_t xEMACRxEventSemaphore = NULL;
/*-----------------------------------------------------------*/

void check_ether_link(void);
void receive_check(void);
void get_random_number(uint8_t *data, uint32_t len);

int32_t lan_open(void);
int32_t lan_open(void);
int32_t lan_read(uint32_t lan_port_no, uint32_t **buf);
int32_t lan_write(uint8_t lan_port_no, uint8_t *header , uint32_t header_len, uint8_t *data , uint32_t data_len);
int32_t rcv_buff_release(uint8_t lan_port_no);

static TimerHandle_t timer_handle;
static uint32_t timer_id;
static TaskHandle_t ether_receive_check_task_handle;
static uint32_t tcpudp_time_cnt;
volatile uint8_t  pause_enable = ETHER_FLAG_OFF;
volatile uint8_t  magic_packet_detect[ETHER_CHANNEL_MAX];
volatile uint8_t  link_detect[ETHER_CHANNEL_MAX];

static const uint8_t renesas_mac_address[6] = {configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5};

static int32_t callback_ether_regist(void);
static void callback_ether(void * pparam);
static void callback_wakeon_lan(uint32_t channel);
static void callback_link_on(uint32_t channel);
static void callback_link_off(uint32_t channel);

static void lan_inthdr(void *ppram);

BaseType_t xNetworkInterfaceInitialise( void )
{
	BaseType_t return_code = pdFALSE;
	uint32_t channel;

	if(lan_open() == 0)
	{
		return_code = pdTRUE;
	}

	timer_id = 1;
	timer_handle = xTimerCreate("ETHER_TIMER", 10, pdTRUE, &timer_id, check_ether_link);
	xTimerStart(timer_handle, 0);

	while(link_detect[0] != ETHER_FLAG_ON_LINK_ON);

	return_code = xTaskCreate(receive_check, "ETHER_RECEIVE_CHECK_TASK", 100/* stack size (word) */, 0,  configMAX_PRIORITIES, &ether_receive_check_task_handle);
	if(return_code == xTaskCreate)
	{
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete( ether_receive_check_task_handle );
	}

	return return_code;
}
/*-----------------------------------------------------------*/

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer )
{
	int16_t return_code;

	/* 簡易実装しました。あとでゼロコピー対応に直します。（シェルティ） */
	static uint8_t tmp_buf[1514];
	memcpy(tmp_buf, pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);
	return_code = lan_write(0, tmp_buf, 14, tmp_buf + 14, pxNetworkBuffer->xDataLength - 14);
	if(return_code == 0)
	{
		/* Call the standard trace macro to log the send event. */
		iptraceNETWORK_INTERFACE_TRANSMIT();
	}
	else
	{
		/* fail sending (nothing to do) */
	}
	vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
	return 0;
}
/*-----------------------------------------------------------*/

uint32_t ulRand(void)
{
	/* 後でハードウェア実装(暗号器使用)に変更します (シェルティ) */
	uint32_t tmp;
	get_random_number(tmp, 4);
	return tmp;
}

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
	uint32_t ul;
    uint8_t *buffer_address;
    buffer_address = __sectop("B_ETHERNET_BUFFERS_1");

    /* ------------------------------------------------------------------
     * RX MCU Ether driver buffer specification
     * ------------------------------------------------------------------
     *
     * top -> B_ETHERNET_BUFFERS_1 |receive buffer 1| (each buffers are always aligned at 32bit)
     *                             |receive buffer 2|
     *                             |       :        |  ETHER_CFG_EMAC_RX_DESCRIPTORS
     *                             |transmit buffer1|
     *                             |transmit buffer2|
     *                             |       :        |  ETHER_CFG_EMAC_TX_DESCRIPTORS
     * */

	for( ul = 0; ul < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; ul++ )
	{
		pxNetworkBuffers[ul].pucEthernetBuffer = (buffer_address + (ETHER_CFG_BUFSIZE * ul));
	}
}

void check_ether_link(void)
{
	R_ETHER_LinkProcess(0);
	tcpudp_time_cnt++;
}

void receive_check(void)
{
	bool pktSuccess, pktLost;
	IPStackEvent_t xRxEvent;
	uint8_t *buffer_pointer;
	int32_t received_data_size;
	NetworkBufferDescriptor_t *pxBufferDescriptor;

	while(1)
	{
		ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
		received_data_size = lan_read(0, &buffer_pointer);

		if(received_data_size > 0)
		{
			pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( received_data_size, 0 );
			if(pxBufferDescriptor != NULL)
			{
				memcpy(pxBufferDescriptor->pucEthernetBuffer, buffer_pointer, received_data_size);
				pxBufferDescriptor->xDataLength = received_data_size;
				xRxEvent.eEventType = eNetworkRxEvent;
				xRxEvent.pvData = pxBufferDescriptor;
				// Send the data to the TCP/IP stack
				if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE )
				{   // failed
					vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
					pktLost = true;
				}
				else
				{   // success
					pktSuccess = true;
				}
			}
		}
		rcv_buff_release(0, buffer_pointer);
	}
}

/******************************************************************************
Functions : random number generator(XorShift method)
******************************************************************************/
void get_random_number(uint8_t *data, uint32_t len)
{
    static uint32_t y = 2463534242;
    uint32_t res;
    uint32_t lp;
    uint8_t *bPtr;

    y += tcpudp_time_cnt;
    res = len / 4;
    for (lp = 0; lp < res; lp++)
    {
        y = y ^ (y << 13);
        y = y ^ (y >> 17);
        y = y ^ (y << 5);
        bPtr = (uint8_t*) & y;
#if __LIT
        *((uint32_t *)data) = (*(bPtr + 3) << 24) | (*(bPtr + 2) << 16) | (*(bPtr + 1) << 8) | *(bPtr + 0);
#else
        *((uint32_t *)data) = y;
#endif
        data += 4;
    }
    y = y ^ (y << 13);
    y = y ^ (y >> 17);
    y = y ^ (y << 5);
    res = (uint32_t)len % 4;
    bPtr = (uint8_t*) & y;
    switch (res)
    {
        case 3:
#if __LIT
            *data++ = bPtr[3];
            *data++ = bPtr[2];
            *data++ = bPtr[1];
#else
            *data++ = bPtr[0];
            *data++ = bPtr[1];
            *data++ = bPtr[2];
#endif
            break;

        case 2:
#if __LIT
            *data++ = bPtr[3];
            *data++ = bPtr[2];
#else
            *data++ = bPtr[0];
            *data++ = bPtr[1];
#endif
            break;

        case 1:
#if __LIT
            *data++ = bPtr[3];
#else
            *data++ = bPtr[0];
#endif
            break;

        default:
            /* no op */
            break;
    }
}

int32_t lan_open(void)
{
    int32_t ret;
    ether_param_t   param;

    R_ETHER_Initial();
    callback_ether_regist();

    param.channel = ETHER_CHANNEL_0;
    R_ETHER_Control(CONTROL_POWER_ON, param);
#if (ETHER_CHANNEL_MAX >= 2)
    param.channel = ETHER_CHANNEL_1;
    R_ETHER_Control(CONTROL_POWER_ON, param);
#endif

    ret = R_ETHER_Open_ZC2(0, renesas_mac_address, false);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }
#if (ETHER_CHANNEL_MAX >= 2)
    ret = R_ETHER_Open_ZC2(1, (const uint8_t *) & _myethaddr[1], false);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }
#endif

    return 0;
}

int32_t lan_close(void)
{
    ether_param_t   param;

    R_ETHER_Close_ZC2(0);
#if (ETHER_CHANNEL_MAX >= 2)
    R_ETHER_Close_ZC2(1);
#endif

    param.channel = ETHER_CHANNEL_0;
    R_ETHER_Control(CONTROL_POWER_OFF, param);
#if (ETHER_CHANNEL_MAX >= 2)
    param.channel = ETHER_CHANNEL_1;
    R_ETHER_Control(CONTROL_POWER_OFF, param);
#endif

    return 0;
}

int32_t lan_read(uint32_t lan_port_no, uint8_t **buf)
{
    int32_t driver_ret;
    int32_t return_code;

    driver_ret = R_ETHER_Read_ZC2(lan_port_no, (void **)buf);
    if (driver_ret > 0)
    {
        return_code = driver_ret;
    }
    else if (driver_ret == 0)
    {
        /* R_Ether_Read() returns "0" when receiving data is nothing */
        return_code = -1; // Return code "-1" notifies "no data" to T4.
    }
    else
    {
        /* R_Ether_Read() returns "negative values" when error occurred */
        return_code = -2; // Return code "-2" notifies "Ether controller disable" to T4.
//      return_code = -5; // Return code "-5" notifies "reset request" to T4.
//      return_code = -6; // Return code "-6" notifies "CRC error" to T4.
    }

#if defined (_T4_TEST)
    return_code = lan_read_for_test(lan_port_no, buf, return_code);
#endif

    return return_code;
}

int32_t lan_write(uint8_t lan_port_no, uint8_t *header , uint32_t header_len, uint8_t *data , uint32_t data_len)
{
    int32_t driver_ret;
    uint8_t *buf;
    uint16_t buf_size;
    uint16_t    framesize;

    driver_ret = R_ETHER_Write_ZC2_GetBuf(lan_port_no, (void **) & buf, &buf_size);
    if (ETHER_SUCCESS == driver_ret)
    {
        framesize = header_len + data_len;                                      /*data length calc.*/
        if (buf_size >= framesize)
        {
            memcpy(buf, header, header_len);
            memcpy(buf + header_len, data, data_len);
            if (framesize < ETHER_BUFSIZE_MIN)                                  /*under minimum*/
            {
                memset((buf + framesize), 0, (ETHER_BUFSIZE_MIN - framesize));  /*padding*/
                framesize = ETHER_BUFSIZE_MIN;                                  /*resize*/
            }
            else
            {
                /*nothing todo*/
            }
            driver_ret =  R_ETHER_Write_ZC2_SetBuf(lan_port_no, (uint16_t)framesize);
            if (ETHER_SUCCESS == driver_ret)
            {
                return 0;
            }
        }
    }
    return -5;
}

int32_t rcv_buff_release(uint8_t lan_port_no)
{
    /* This function is called when TCP/IP finished using receive buffer specified lan_read. */
    R_ETHER_Read_ZC2_BufRelease(lan_port_no);
    return 0;
}

/***********************************************************************************************************************
* Function Name: callback_ether
* Description  : Regist of callback function
* Arguments    : -
* Return Value : 0: success, -1:failed
***********************************************************************************************************************/
int32_t callback_ether_regist(void)
{
    ether_param_t   param;
    ether_cb_t      cb_func;

    int32_t         ret;

    /* Set the callback function (LAN cable connect/disconnect event) */
    cb_func.pcb_func     = &callback_ether;
    param.ether_callback = cb_func;
    ret = R_ETHER_Control(CONTROL_SET_CALLBACK, param);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }

    /* Set the callback function (Ether interrupt event) */
    cb_func.pcb_int_hnd     = &lan_inthdr;
    param.ether_callback = cb_func;
    ret = R_ETHER_Control(CONTROL_SET_INT_HANDLER, param);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }
    return 0;
} /* End of function callback_ether_regist() */

/***********************************************************************************************************************
* Function Name: callback_ether
* Description  : Sample of the callback function
* Arguments    : pparam -
*
* Return Value : none
***********************************************************************************************************************/
void callback_ether(void * pparam)
{
    ether_cb_arg_t    * pdecode;
    uint32_t            channel;

    pdecode = (ether_cb_arg_t *)pparam;
    channel = pdecode->channel;                             /* Get Ethernet channel number */

    switch (pdecode->event_id)
    {
        /* Callback function that notifies user to have detected magic packet. */
        case ETHER_CB_EVENT_ID_WAKEON_LAN:
            callback_wakeon_lan(channel);
            break;

        /* Callback function that notifies user to have become Link up. */
        case ETHER_CB_EVENT_ID_LINK_ON:
            callback_link_on(channel);
            break;

        /* Callback function that notifies user to have become Link down. */
        case ETHER_CB_EVENT_ID_LINK_OFF:
            callback_link_off(channel);
            break;

        default:
            break;
    }
} /* End of function callback_ether() */

/***********************************************************************************************************************
* Function Name: callback_wakeon_lan
* Description  :
* Arguments    : channel -
*                    Ethernet channel number
* Return Value : none
***********************************************************************************************************************/
static void callback_wakeon_lan(uint32_t channel)
{
    if (ETHER_CHANNEL_MAX > channel)
    {
        magic_packet_detect[channel] = 1;

        /* Please add necessary processing when magic packet is detected.  */
    }
} /* End of function callback_wakeon_lan() */

/***********************************************************************************************************************
* Function Name: callback_link_on
* Description  :
* Arguments    : channel -
*                    Ethernet channel number
* Return Value : none
***********************************************************************************************************************/
static void callback_link_on(uint32_t channel)
{
    if (ETHER_CHANNEL_MAX > channel)
    {
        if((link_detect[channel] != ETHER_FLAG_ON_LINK_ON))
        {
        	/* notify link on event to FreeRTOS+TCP */
        	nop();
        }
        link_detect[channel] = ETHER_FLAG_ON_LINK_ON;

        /* Please add necessary processing when becoming Link up. */
    }
} /* End of function callback_link_on() */

/***********************************************************************************************************************
* Function Name: callback_link_off
* Description  :
* Arguments    : channel -
*                    Ethernet channel number
* Return Value : none
***********************************************************************************************************************/
static void callback_link_off(uint32_t channel)
{
    if (ETHER_CHANNEL_MAX > channel)
    {
        if((link_detect[channel] != ETHER_FLAG_ON_LINK_OFF))
        {
        	/* notify link off event to FreeRTOS+TCP */
        	nop();
        }
        link_detect[channel] = ETHER_FLAG_ON_LINK_OFF;

        /* Please add necessary processing when becoming Link down. */
    }
} /* End of function ether_cb_link_off() */

void lan_inthdr(void *ppram)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(ether_receive_check_task_handle != 0)
	{
		/* notify receive event to FreeRTOS+TCP */
		vTaskNotifyGiveFromISR(ether_receive_check_task_handle, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
