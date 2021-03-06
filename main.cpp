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
 *          Rev.: 07, 08.04.2018 - BL1 BL2 and RES are now working (check for
 *                                 running threads)
 *          Rev.: 08, 09.04.2018 - Buttons keep the current process in a while(1)
 *                                 loop
 *          Rev.: 09, 09.04.2018 - TL commands are now working
 *          Rev.: 10, 09.04.2018 - RES now supports les OFF or ON
 *          Rev.: 11, 09.04.2018 - Add Error_val to protect the protcol,
 *                                 using a command without the last frame char $
 *                                 Now we need last frame char $ to go forward
 *          Rev.: 12, 09.04.2018 - Removed the value and use if(1) because
 *                                 if(error_val == 0) created a failure
 *          Rev.: 13, 10.04.2018 - Add Runtimetest and Ledtest for oscilloscope
 *                                 using a DAC.
 *          Rev.: 14, 11.04.2018 - Add the DONE message to the PC from XMC
 * \notes
 *
 * \commands #0:BL1:400$ - is creating a blink led1/led2 with 400 cylces with 1s
 *           #0:BL2:400$ - on and offtime
 *
 *           #0:TL1:H100L1000$ - is creating a blink led1/led2 with 1000 cycles
 *           #0:TL2:H100L1000$ - and a hightime of 100ms and a lowtime of 1sec
 *
 *           #0:RES:OFF$ - is reseting the tasks and sets both leds off
 *           #0:RES:ON$  - is reseting the tasks and sets both leds on
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

#define LEDTEST 0
#define RUNTIMETEST 0

#if !RUNTIMETEST
#define DEFAULT 1000
#endif

#if RUNTIMETEST
#define DEFAULT 10
#endif

/********************************** Globals ***********************************/
/*
	LED1 and LED2 are defined in:
	targets/TARGET_Infineon/TARGET_XMC4XXX/TARGET_XMC4500/PinNames.h
*/
DigitalOut led_1(LED1);					/* LED1 = P1.1 */
DigitalOut led_2(LED2);					/* LED2 = P1.0 */

#if LEDTEST || RUNTIMETEST
AnalogOut dac_out(P14_8);					/* DAC = P14.8 */
#endif

//dac_out.write(float %)
//dac_out.write_u16(0x0 - 0xFFFF)

DigitalIn button_1(SW1);				/* Button 1 */
DigitalIn button_2(SW2);				/* Button 2 */

Serial device(UARTTX, UARTRX);	/* UART -> TX = P1.5, RX = P1.4 */

/* Create Threads */
Thread *thread_led1;									/* Thread for LED1 */
Thread *thread_led2;									/* Thread for LED2 */
Thread *thread_com;										/* Thread for communication */

/*
	This globals are required, as I was trying to use the callback function
	but it was not working for me.
	* status = thread_led1->start(callback(com_led_1,
	*                                     (uint32_t *)&default_value,
	*                                     (uint32_t *)&default_value,
	*                                     (uint32_t *)&int_of_data));
*/
uint32_t ontime;
uint32_t offtime;
uint32_t cycle;

/********************************* Functions **********************************/
/* Function for LED 1 */
void com_led_1_bl(void)
{
	uint32_t timer = 0;
	uint32_t local_ontime = ontime;
	uint32_t local_offtime = offtime;
	uint32_t local_cycle = cycle;
	
#if RUNTIMETEST
	/* Set DAC Value */
	dac_out.write(0.0f);
#endif
	
	while (timer != local_cycle)
	{
		while (!button_1);
		
#if LEDTEST
		/* Set DAC Value */
		dac_out.write(0.5f);
#endif
		
		led_1 = 1;
		Thread::wait(local_ontime);
		
		while (!button_1);
		
#if LEDTEST
		/* Set DAC Value */
		dac_out.write(0.0f);
#endif
		
		
		led_1 = 0;
		Thread::wait(local_offtime);
		
		timer++;
	}
	
	device.printf("#0:BL1:DONE$\n");
	
#if RUNTIMETEST
	/* Set DAC Value */
	dac_out.write(0.5f);
	Thread::wait(10);
	dac_out.write(0.0f);
#endif
	
	thread_led1->terminate();
	delete thread_led1;
	thread_led1 = NULL;
}

void com_led_1_tl(void)
{
	uint32_t timer = 0;
	uint32_t local_ontime = ontime;
	uint32_t local_offtime = offtime;
	
	while (timer != 1000)
	{
		while (!button_1);
		
#if LEDTEST
		/* Set DAC Value */
		dac_out.write(0.5f);
#endif
		
		led_1 = 1;
		Thread::wait(local_ontime);
		
		while (!button_1);
		
#if LEDTEST
		/* Set DAC Value */
		dac_out.write(0.0f);
#endif
		
		led_1 = 0;
		Thread::wait(local_offtime);
		
		timer++;
	}
	
	device.printf("#0:TL1:DONE$\n");
	
	thread_led1->terminate();
	delete thread_led1;
	thread_led1 = NULL;
}

/* Function for LED 2 */
void com_led_2_bl(void)
{
	uint32_t timer = 0;
	uint32_t local_ontime = ontime;
	uint32_t local_offtime = offtime;
	uint32_t local_cycle = cycle;
	
	while (timer != local_cycle)
	{
		while (!button_2);
		
		led_2 = 1;
		Thread::wait(local_ontime);
		
		while (!button_2);
		
		led_2 = 0;
		Thread::wait(local_offtime);
		
		timer++;
	}
	
	device.printf("#0:BL2:DONE$\n");
	
	thread_led2->terminate();
	delete thread_led2;
	thread_led2 = NULL;
}

void com_led_2_tl(void)
{
	uint32_t timer = 0;
	uint32_t local_ontime = ontime;
	uint32_t local_offtime = offtime;
	
	while (timer != 1000)
	{
		while (!button_2);
		
		led_2 = 1;
		Thread::wait(local_ontime);
		
		while (!button_2);
		
		led_2 = 0;
		Thread::wait(local_offtime);
		
		timer++;
	}
	
	device.printf("#0:TL2:DONE$\n");
	
	
	thread_led2->terminate();
	delete thread_led2;
	thread_led2 = NULL;
}

/* Function for UART communication */
void com_communication(void)
{
	osStatus status;
	char receiver_char;
	char receiver_buffer[BUF_SIZE];
	uint32_t receiver_counter = 0;
	uint32_t i = 0;
	uint32_t n = 0;
	uint32_t data_int;
	
	char *savepointer;
	char *token;
	
	char *number_pointer;
	char *command_pointer;
	char *data_pointer;
	char *ptr;
	
	char number_buffer[BUF_SIZE];
	char command_buffer[BUF_SIZE];
	char data_buffer[BUF_SIZE];
	
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
	device.printf("Task 3 - Control of LED2\n\n");
	
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
						/* Put every received character into the buffer */
						receiver_char = device.getc();
						receiver_buffer[receiver_counter] = receiver_char;
						
						receiver_counter++;
					}
				}
				
#if RUNTIMETEST
				/* Set DAC Value */
				dac_out.write(0.5f);
#endif
				
				/* Send ACK */
				device.putc(0x06);
			
#if DEBUG
				/* Check received data */
				receiver_buffer[BUF_SIZE] = '\0';
				device.printf("#%s\n", receiver_buffer);
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
				data_buffer[strlen(data_buffer)-1] = '\0';
				
#if DEBUG
				device.printf("Number:  %s\n", number_buffer);
				device.printf("Command: %s\n", command_buffer);
				device.printf("Data:    %s\n", data_buffer);
#endif
				
				/* Command BL1 */
				if (strncmp(command_buffer, "BL1", BUF_SIZE*sizeof(char)) == 0)
				{
					if (thread_led1 != NULL)
					{
						thread_led1->terminate();
						delete thread_led1;
						thread_led1 = NULL;
					}
					thread_led1 = new Thread();
					
#if DEBUG
					device.printf("Started BL1\n\n");
#endif
					
					ontime = DEFAULT;
					offtime = DEFAULT;
					cycle = (uint32_t)strtoul(data_buffer, &ptr, 10);
					
					status = thread_led1->start(com_led_1_bl);
					if (status != osOK)
					{
						error("ERROR: Thread LED1: Failed!");
					}
				}
				
				/* Command BL2 */
				else if (strncmp(command_buffer, "BL2", BUF_SIZE*sizeof(char)) == 0)
				{
					if (thread_led2 != NULL)
					{
						thread_led2->terminate();
						delete thread_led2;
						thread_led2 = NULL;
					}
					thread_led2 = new Thread();
					
#if DEBUG
					device.printf("Started BL2\n\n");
#endif
					
					ontime = DEFAULT;
					offtime = DEFAULT;
					cycle = (uint32_t)strtoul(data_buffer, &ptr, 10);
					
					status = thread_led2->start(com_led_2_bl);
					if (status != osOK)
					{
						error("ERROR: Thread LED2: Failed!");
					}
				}
				
				/* Command TL1 */
				else if (strncmp(command_buffer, "TL1", BUF_SIZE*sizeof(char)) == 0)
				{
					if (thread_led1 != NULL)
					{
						thread_led1->terminate();
						delete thread_led1;
						thread_led1 = NULL;
					}
					thread_led1 = new Thread();
					
					token = data_buffer;
					n = 0;
					
					if (data_buffer[0] == 'H')
					{
						while ((token = strtok_r(token, "HL", &savepointer)) != NULL)
						{
							data_int = (uint32_t)strtoul(token, &ptr, 10);
							
							if (n == 0)
							{
								ontime = data_int;
							}
							
							if (n == 1)
							{
								offtime = data_int;
							}
							
							token = NULL;
							n++;
						}
					}
					
					if (data_buffer[0] == 'L')
					{
						while ((token = strtok_r(token, "LH", &savepointer)) != NULL)
						{
							data_int = (uint32_t)strtoul(token, &ptr, 10);
							
							if (n == 0)
							{
								offtime = data_int;
							}
							
							if (n == 1)
							{
								ontime = data_int;
							}
							
							token = NULL;
							n++;
						}
					}
					
					status = thread_led1->start(com_led_1_tl);
					if (status != osOK)
					{
						error("ERROR: Thread LED2: Failed!");
					}
					
#if DEBUG
					device.printf("Started TL1\n\n");
#endif
					
				}
				
				/* Command TL2 */
				else if (strncmp(command_buffer, "TL2", BUF_SIZE*sizeof(char)) == 0)
				{
					if (thread_led2 != NULL)
					{
						thread_led2->terminate();
						delete thread_led2;
						thread_led2 = NULL;
					}
					thread_led2 = new Thread();
					
					token = data_buffer;
					n = 0;
					
					if (data_buffer[0] == 'H')
					{
						while ((token = strtok_r(token, "HL", &savepointer)) != NULL)
						{
							data_int = (uint32_t)strtoul(token, &ptr, 10);
							
							if (n == 0)
							{
								ontime = data_int;
							}
							
							if (n == 1)
							{
								offtime = data_int;
							}
							
							token = NULL;
							n++;
						}
					}
					
					if (data_buffer[0] == 'L')
					{
						while ((token = strtok_r(token, "LH", &savepointer)) != NULL)
						{
							data_int = (uint32_t)strtoul(token, &ptr, 10);
							
							if (n == 0)
							{
								offtime = data_int;
							}
							
							if (n == 1)
							{
								ontime = data_int;
							}
							
							token = NULL;
							n++;
						}
					}
					
					
					status = thread_led2->start(com_led_2_tl);
					if (status != osOK)
					{
						error("ERROR: Thread LED2: Failed!");
					}
					
#if DEBUG
					device.printf("Started TL2\n\n");
#endif
					
				}
				
				/* Command RES */
				else if (strncmp(command_buffer, "RES", BUF_SIZE*sizeof(char)) == 0)
				{
					
#if DEBUG
					device.printf("Started RES\n");
#endif
					
					
					if (thread_led1 != NULL)
					{
						device.printf("NOT DONE Task 1\n");
						thread_led1->terminate();
						delete thread_led1;
						thread_led1 = NULL;
					}
					
					if (thread_led2 != NULL)
					{
						device.printf("NOT DONE Task 2\n");
						thread_led2->terminate();
						delete thread_led2;
						thread_led2 = NULL;
					}
					
					if (strncmp(data_buffer, "ON", BUF_SIZE*sizeof(char)) == 0)
					{
						led_1 = 1;
						led_2 = 1;
					}
					
					else if (strncmp(data_buffer, "OFF", BUF_SIZE*sizeof(char)) == 0)
					{
						led_1 = 0;
						led_2 = 0;
					}
					
					device.printf("\n");
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
