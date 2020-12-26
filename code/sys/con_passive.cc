/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-xyyz Lars '0xA5EA' Kandler

This file is part of KingpinQ3 source code.

KingpinQ3 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

KingpinQ3 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with KingpinQ3 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <stdio.h>

/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown(void)
{
}

/*
==================
CON_Init
==================
*/
void CON_Init(void)
{
}

/*
==================
CON_Input
==================
*/
char *CON_Input(void)
{
	return NULL;
}

/*
==================
CON_Print
==================
*/
void CON_Print(const char *msg)
{
	if(com_ansiColor && com_ansiColor->integer)
		Sys_AnsiColorPrint(msg);
	else
		fputs(msg, stderr);
}
/*
==================
CON_SetVisibility
==================
*/
void CON_SetVisibility(int visLevel)
{
}
/*
==================
CON_SetErrorText
==================
*/
void CON_SetErrorText(const char *buf)
{
}