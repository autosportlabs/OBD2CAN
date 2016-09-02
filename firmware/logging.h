/*
 * logging.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef LOGGING_H_
#define LOGGING_H_
#include "ch.h"
#include "hal.h"
#include "chprintf.h"


enum logging_levels
{
    logging_level_none,
    logging_level_info,
    logging_level_trace
};


#define log_info(msg, ...) if (get_logging_level() >= logging_level_info) {chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}
#define log_trace(msg, ...) if (get_logging_level() >= logging_level_trace) {chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}
//#define debug_write

void set_logging_level(enum logging_levels level);

enum logging_levels get_logging_level(void);

#endif /* LOGGING_H_ */
