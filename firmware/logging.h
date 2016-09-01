/*
 * logging.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "chprintf.h"

#define debug_write(msg, ...) chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__); sdPut(&SD1, '\r'); sdPut(&SD1, '\n')
//#define debug_write

#endif /* LOGGING_H_ */
