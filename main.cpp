/* ! Echtzeitbetriebssysteme Task 1
 * ! Serial Communication / UART
 *
 * \description A MBED-OS serial communication program
 *
 * \author Sebastian Dichler <el16b032@technikum-wien.at> <sedi343@gmail.com>
 *
 * \version Rev.: 01, 14.03.2018 - Created the File and First Style
 *          Rev.: 02, 14.03.2018 - First steps into C++ and MBED-OS, create 2
 *                                 threads for each led. halt threads if each
 *                                 button is pressed
 *          Rev.: 03, 28.03.2018 - First steps to the task requirements. Add
 *                                 functions for led1 and led2 with the ontime,
 *                                 offtime and cycles
 *
 */

/********************************** Includes **********************************/
#include "mbed.h"
#include "rtos.h"
#include <string.h>

/********************************** Defines ***********************************/
#define UARTTX P1_5
#define UARTRX P1_4

#define DEBUG 1
#define BUF_SIZE 16 /* 20 */

/********************************** Typedef ***********************************/
typedef struct {
	uint32_t counter;							/* Number of received commands */
} mail_t;

/********************************** Globals ***********************************/
/*
	LED1 and LED2 are defined in:
	targets/TARGET_Infineon/TARGET_XMC4XXX/TARGET_XMC4500/PinNames.h
*/
DigitalOut led_1(LED1);					/* LED1 = P1.1 */
DigitalOut led_2(LED2);					/* LED2 = P1.0 */

Serial device(UARTTX, UARTRX);	/* UART -> TX = P1.5, RX = P1.4 */

/* Create Threads */
Thread *thread_led1;									/* Thread for LED1 */
Thread *thread_led2;									/* Thread for LED2 */
Thread *thread_com;										/* Thread for communication */

/********************************* Functions **********************************/
/* Function for LED 1 */
void com_led_1(uint32_t ontime, uint32_t offtime, uint32_t cycle)
{
	uint32_t timer = 0;	
	
	while (timer != cycle)
	{
		led_1 = 1;
		Thread::wait(ontime);
		led_1 = 0;
		Thread::wait(offtime);
		
		timer++;
	}
}

/* Function for LED 2 */
void com_led_2(uint32_t ontime, uint32_t offtime, uint32_t cylce)
{
	uint32_t timer = 0;
	
	while (timer != cylce)
	{
		led_2 = 1;
		Thread::wait(ontime);
		led_2 = 0;
		Thread::wait(offtime);
		
		timer++;
	}
}

/* Function for UART communication */
void com_communication(void)
{
	osStatus status;
	char receiver_char;
	char receiver_buffer[BUF_SIZE];
	uint32_t receiver_counter = 0;
	
	/*
		Configureation of the UART,
		Change Baudrate to 9600
	*/
	device.baud(9600);
	
	device.printf("Threaded UART LED Task\n");
	device.printf("by Sebastian Dichler | <el16b032@technikum-wien.at>\n");
	device.printf("Task 1 - Communication\n");
	device.printf("Task 2 - Control of LED1\n");
	device.printf("Task 3 - Control of LED2\n");
	
	while(1)
	{
		/* Initialize the Buffer with 0 */
		memset(receiver_buffer, 0, BUF_SIZE));
		
		/* If the UART is readable */
		if (device.readable())
		{
			/* Remove Characters until received char equals # */
			if (receiver_char = device.getc() == '#')
			{
				/*
					Read characters and write them to buffer
					until char is $ or BUF_SIZE is reached
				*/
				while(receiver_char != '$' || receiver_counter == BUF_SIZE)
				{
					if (device.readable())
					{
						receiver_char = device.getc();
						receiver_buffer[receiver_counter] = receiver_char;
						
						receiver_counter++;
					}
				}

#if DEBUG
				/* Check received data */
				receiver_buffer[BUF_SIZE] = '\0';
				device.printf("%s\n", receiver_buffer);
#endif

				/* Split string into commands */
								
				receiver_counter = 0;
			}
		}
	}
}

/******************************* Main Function ********************************/

int main(void)
{
	osStatus status;
	
	thread_com = new Thread();
	
/* ---- Start Thread 1 ---- */
	status = thread_com->start(com_communication);
	if (status != osOK)
	{
		error("ERROR: Thread 1: Failed!");
	}
}
