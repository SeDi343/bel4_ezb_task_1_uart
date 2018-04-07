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
 *          Rev.: 04, 05.04.2018 - Add communication thread, splitting the buffer
 *                                 To different buffers.
 *          Rev.: 05, 07.04.2018 - Add memcpy and memset functions to write split
 *                                 command into seperate buffers
 *          Rev.: 06, 07.04.2018 - Different Buffers are now working, connecting
 *                                 to other threads
 *
 */

/********************************** Includes **********************************/
#include "mbed.h"
#include "rtos.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

/********************************** Defines ***********************************/
#define UARTTX P1_5
#define UARTRX P1_4

#define DEBUG 1
#define BUF_SIZE 16 /* 20 */
#define DEFAULT 1000


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
	uint32_t i = 0;
	
	char *savepointer;
	char *token;
	
	char *number_pointer;
	char *command_pointer;
	char *data_pointer;
	
	char number_buffer[BUF_SIZE];
	char command_buffer[BUF_SIZE];
	char data_buffer[BUF_SIZE];
	char data_high_buffer[BUF_SIZE];
	char data_low_buffer[BUF_SIZE];
	
	using namespace std;
	
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
	
	while (1)
	{
		/* Initialize the Buffer with 0 */
		memset(receiver_buffer, 0, BUF_SIZE*sizeof(char));
		
		/* Reset Buffers and other variables */
		memset(number_buffer, 0, BUF_SIZE*sizeof(char));
		memset(command_buffer, 0, BUF_SIZE*sizeof(char));
		memset(data_buffer, 0, BUF_SIZE*sizeof(char));
		receiver_counter = 0;
		
		/* If the UART is readable */
		if (device.readable())
		{
			/* Remove Characters until received char equals # */
			receiver_char = device.getc();
			if (receiver_char == '#')
			{
				/*
					Read characters and write them to buffer
					until char is $ or BUF_SIZE is reached
				*/
				while (receiver_char != '$' && receiver_counter != BUF_SIZE)
				{
					if (device.readable())
					{
						receiver_char = device.getc();
						receiver_buffer[receiver_counter] = receiver_char;
						
#if DEBUG
						device.printf("Recognised Char %d\n", receiver_counter+1);
#endif
						
						receiver_counter++;
					}
				}
				
#if DEBUG
				/* Check received data */
				receiver_buffer[BUF_SIZE] = '\0';
				device.printf("%s\n", receiver_buffer);
#endif
				
				i = 0;
				
				/* Split string into commands */
				/*
					The strtok() function uses a static buffer while parsing,
					so it's not thread safe.  Use  strtok_r()  if  this matters to you.
				*/
				token = strtok_r(receiver_buffer, ":", &savepointer);
				while (token != NULL)
				{
					if (i == 0)
					{
						number_pointer = token;
					}
					
					if (i == 1)
					{
						command_pointer = token;
					}
					
					if (i == 2)
					{
						data_pointer = token;
					}
					
					token = strtok_r(NULL, ":", &savepointer);
					i++;
				}
				
				/* Copy splitted command into buffers */
				memcpy(number_buffer, (char *)number_pointer, BUF_SIZE*sizeof(char));
				memcpy(command_buffer, (char *)command_pointer, BUF_SIZE*sizeof(char));
				memcpy(data_buffer, (char *)data_pointer, BUF_SIZE*sizeof(char));
				data_buffer[strlen(data_buffer)-1] = 0;
				
#if DEBUG
				device.printf("Number:  %s\n", number_buffer);
				device.printf("Command: %s\n", command_buffer);
				device.printf("Data:    %s\n", data_buffer);
#endif
				
				if (strncmp(command_buffer, "BL1", BUF_SIZE*sizeof(char)))
				{
					com_led_1(DEFAULT, DEFAULT, (uint32_t)atoi(data_buffer));
				}

				else if (strncmp(command_buffer, "BL2", BUF_SIZE*sizeof(char)))
				{
					com_led_2(DEFAULT, DEFAULT, (uint32_t)atoi(data_buffer));
				}
				
				else if (strncmp(command_buffer, "TL1", BUF_SIZE*sizeof(char)))
				{
					token = strtok_r(data_buffer, "L", &savepointer);
					
					memcpy(data_high_buffer, (char *)token, BUF_SIZE*sizeof(char));
					memcpy(data_low_buffer, (char *)token, BUF_SIZE*sizeof(char));
					
					device.printf("High Buffer: %s\n", data_high_buffer);
					device.printf("Low Buffer:  %s\n", data_low_buffer);
				}
			} /* receiver_char == '#' */
		} /* device.readable() */
	} /* while(1) */
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
