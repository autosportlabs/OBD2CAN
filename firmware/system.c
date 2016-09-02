/*
 * OBD2CAN firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
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




