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
 *
 */

/********************************** Includes **********************************/
#include "mbed.h"
#include "rtos.h"

/********************************** Typedef ***********************************/
typedef struct {
	uint32_t counter;					/* Number of received commands */
} mail_t;

/********************************** Globals ***********************************/
/*
	LED1 and LED2 are dfined in:
	targets/TARGET_Infineon/TARGET_XMC4XXX/TARGET_XMC4500/PinNames.h
*/
DigitalOut led_1(LED1);			/* LED1 = P1.1 */
DigitalOut led_2(LED2);			/* LED2 = P1.0 */

DigitalIn button_1(SW1);		/* Button 1 */
DigitalIn button_2(SW2);		/* Button 2 */

Serial device(P1_5, P1_4);	/* UART -> TX = P1.5, RX = P1.4 */

/* Create Threads */
Thread thread1;							/* Thread for LED1 */
Thread thread2;							/* Thread for LED2 */

/*
	mail_t ..... data type of the mail messages
	16 ......... size of the mailbox
*/
Mail<mail_t, 16> mail_box;

/********************************* Functions **********************************/
/* Function for Thread 1 */
void com_led_1(void)
{
	while(1)
	{
		while(!button_1);
		
		led_1 = !led_1;
		wait(0.5);
	}
}

/* Function for Thread 2 */
void com_led_2(void)
{
	while(1)
	{
		while(!button_2);
		
		led_2 = !led_2;
		wait(0.25);
	}
}

/******************************* Main Function ********************************/

int main(void)
{
	osStatus status;
	
/* ---- Start Thread 1 ---- */
	status = thread1.start(com_led_1);
	if (status != osOK)
	{
		error("ERROR: Thread 1: Failed!");
	}
	
/* ---- Start Thread 2 ---- */
	status = thread2.start(com_led_2);
	if (status != osOK)
	{
		error("ERROR: Thread 2: Failed!");
	}
	
	while(1)
	{
		if (!button_1 && !button_2)
		{
			//thread1.terminate();
			//thread2.terminate();
		}
	}
}
