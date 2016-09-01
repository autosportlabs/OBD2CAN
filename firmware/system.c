/*
 * system.c
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */
#include "system.h"

bool system_initialized = false;
bool pid_request_active = false;

void set_system_initialized(bool initialized)
{
	system_initialized = initialized;
}

bool get_system_initialized(void)
{
	return system_initialized;
}

void set_pid_request_active(bool active)
{
	pid_request_active = active;
}

bool get_pid_request_active(void)
{
	return pid_request_active;
}




