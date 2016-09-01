/*
 * system.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <stdbool.h>

void set_system_initialized(bool initialized);
bool get_system_initialized(void);

void set_pid_request_active(bool active);
bool get_pid_request_active(void);



#endif /* SYSTEM_H_ */
