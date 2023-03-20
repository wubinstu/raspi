

#ifndef __RASPI_DRIVE_H_
#define __RASPI_DRIVE_H_

#include "global.h"
#include "head.h"

#if (SIMULATOR == false)

#include <wiringPi.h>

#endif

/** init wiringPi lib
 * @return 0 for success,-1 for failed*/
extern int initPi ();

/** read cpu temperature and return it */
extern float read_cpu_temp ();

extern void turn_on_led (int pinNum);

extern void turn_off_led (int pinNum);

/** Flash the light once for "micro_secs" */
extern void flash_led (int pinNum, int micro_secs);

/** Ultrasonic ranging and return distance */
extern float disMeasure (int Trans, int Receive);

/** test function */
extern bool TEST_IN (int LINE);

/** Detect temperature and humidity and write variables */
extern bool readSensorData (int pinNum, float * Humidity, float * Temperature);

/** Write the specified number into the nixie tube, with the range of 0 ~ 9999 */
extern bool screen (int num, int dataPin, int clockPin);


#endif//__RASPI_DRIVE_H_