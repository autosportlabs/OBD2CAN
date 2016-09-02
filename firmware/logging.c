/*
 * logging.c
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */
#include "logging.h"

enum logging_levels logging_level = logging_level_trace;

void set_logging_level(enum logging_levels level)
{
    logging_level = level;
}

enum logging_levels get_logging_level(void)
{
    return logging_level;
}
