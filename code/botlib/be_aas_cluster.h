/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler

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

/*****************************************************************************
 * name:		be_aas_cluster.h
 *
 * desc:		AAS
 *
 * $Archive: /source/code/botlib/be_aas_cluster.h $
 *
 *****************************************************************************/
#ifndef BE_AAS_CLUSTER_H_
#define BE_AAS_CLUSTER_H_
#ifdef AASINTERN
//initialize the AAS clustering
void AAS_InitClustering(void);
//
void AAS_SetViewPortalsAsClusterPortals(void);
#endif //AASINTERN
void AAS_InitClustering(void);
#endif

