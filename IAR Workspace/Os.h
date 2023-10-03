 /******************************************************************************
 *
 * Module: Os
 *
 * File Name: Os.h
 *
 * Description: Header file for Os Scheduler.
 *
 * Author: Mohamed Tarek
 ******************************************************************************/

#ifndef OS_H_
#define OS_H_

#include "Std_Types.h"

/* Timer counting time in ms */
#define OS_BASE_TIME 20

/* Description: 
 * Function responsible for:
 * 1. Enable Interrupts
 * 2. Start the Os timer
 * 3. Execute the Init Task
 * 4. Start the Scheduler to run the tasks
 */
void Os_start(void);

/* Description: The Engine of the Os Scheduler used for switch between different tasks */
void Os_Scheduler(void);

/* Description: Function called by the Timer Driver in the MCAL layer using the call back pointer */
void Os_NewTimerTick(void);

#endif /* OS_H_ */
