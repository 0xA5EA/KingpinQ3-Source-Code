/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qbsp.h"
#include "../botlib/aasfile.h"
#include "aas_file.h"
#include "aas_store.h"
#include "aas_create.h"

void QDECL AAS_Error(char *fmt, ...);

//#define AAS_Error			Error

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_SwapAASData(void)
{
	int i, j;
	//bounding boxes
	for (i = 0; i < aasworld.numbboxes; i++)
	{
		aasworld.bboxes[i].presencetype = LittleLongl(aasworld.bboxes[i].presencetype);
		aasworld.bboxes[i].flags = LittleLongl(aasworld.bboxes[i].flags);
		for (j = 0; j < 3; j++)
		{
			aasworld.bboxes[i].mins[j] = LittleLongl(aasworld.bboxes[i].mins[j]);
			aasworld.bboxes[i].maxs[j] = LittleLongl(aasworld.bboxes[i].maxs[j]);
		} //end for
	} //end for
	//vertexes
	for (i = 0; i < aasworld.numvertexes; i++)
	{
		for (j = 0; j < 3; j++)
			aasworld.vertexes[i][j] = LittleFloatl(aasworld.vertexes[i][j]);
	} //end for
	//planes
	for (i = 0; i < aasworld.numplanes; i++)
	{
		for (j = 0; j < 3; j++)
			aasworld.planes[i].normal[j] = LittleFloatl(aasworld.planes[i].normal[j]);
		aasworld.planes[i].dist = LittleFloatl(aasworld.planes[i].dist);
		aasworld.planes[i].type = LittleLongl(aasworld.planes[i].type);
	} //end for
	//edges
	for (i = 0; i < aasworld.numedges; i++)
	{
		aasworld.edges[i].v[0] = LittleLongl(aasworld.edges[i].v[0]);
		aasworld.edges[i].v[1] = LittleLongl(aasworld.edges[i].v[1]);
	} //end for
	//edgeindex
	for (i = 0; i < aasworld.edgeindexsize; i++)
	{
		aasworld.edgeindex[i] = LittleLongl(aasworld.edgeindex[i]);
	} //end for
	//faces
	for (i = 0; i < aasworld.numfaces; i++)
	{
		aasworld.faces[i].planenum = LittleLongl(aasworld.faces[i].planenum);
		aasworld.faces[i].faceflags = LittleLongl(aasworld.faces[i].faceflags);
		aasworld.faces[i].numedges = LittleLongl(aasworld.faces[i].numedges);
		aasworld.faces[i].firstedge = LittleLongl(aasworld.faces[i].firstedge);
		aasworld.faces[i].frontarea = LittleLongl(aasworld.faces[i].frontarea);
		aasworld.faces[i].backarea = LittleLongl(aasworld.faces[i].backarea);
	} //end for
	//face index
	for (i = 0; i < aasworld.faceindexsize; i++)
	{
		aasworld.faceindex[i] = LittleLongl(aasworld.faceindex[i]);
	} //end for
	//convex areas
	for (i = 0; i < aasworld.numareas; i++)
	{
		aasworld.areas[i].areanum = LittleLongl(aasworld.areas[i].areanum);
		aasworld.areas[i].numfaces = LittleLongl(aasworld.areas[i].numfaces);
		aasworld.areas[i].firstface = LittleLongl(aasworld.areas[i].firstface);
		for (j = 0; j < 3; j++)
		{
			aasworld.areas[i].mins[j] = LittleFloatl(aasworld.areas[i].mins[j]);
			aasworld.areas[i].maxs[j] = LittleFloatl(aasworld.areas[i].maxs[j]);
			aasworld.areas[i].center[j] = LittleFloatl(aasworld.areas[i].center[j]);
		} //end for
	} //end for
	//area settings
	for (i = 0; i < aasworld.numareasettings; i++)
	{
		aasworld.areasettings[i].contents = LittleLongl(aasworld.areasettings[i].contents);
		aasworld.areasettings[i].areaflags = LittleLongl(aasworld.areasettings[i].areaflags);
		aasworld.areasettings[i].presencetype = LittleLongl(aasworld.areasettings[i].presencetype);
		aasworld.areasettings[i].cluster = LittleLongl(aasworld.areasettings[i].cluster);
		aasworld.areasettings[i].clusterareanum = LittleLongl(aasworld.areasettings[i].clusterareanum);
		aasworld.areasettings[i].numreachableareas = LittleLongl(aasworld.areasettings[i].numreachableareas);
		aasworld.areasettings[i].firstreachablearea = LittleLongl(aasworld.areasettings[i].firstreachablearea);
	} //end for
	//area reachability
	for (i = 0; i < aasworld.reachabilitysize; i++)
	{
		aasworld.reachability[i].areanum = LittleLongl(aasworld.reachability[i].areanum);
		aasworld.reachability[i].facenum = LittleLongl(aasworld.reachability[i].facenum);
		aasworld.reachability[i].edgenum = LittleLongl(aasworld.reachability[i].edgenum);
		for (j = 0; j < 3; j++)
		{
			aasworld.reachability[i].start[j] = LittleFloatl(aasworld.reachability[i].start[j]);
			aasworld.reachability[i].end[j] = LittleFloatl(aasworld.reachability[i].end[j]);
		} //end for
		aasworld.reachability[i].traveltype = LittleLongl(aasworld.reachability[i].traveltype);
		aasworld.reachability[i].traveltime = LittleShortl(aasworld.reachability[i].traveltime);
	} //end for
	//nodes
	for (i = 0; i < aasworld.numnodes; i++)
	{
		aasworld.nodes[i].planenum = LittleLongl(aasworld.nodes[i].planenum);
		aasworld.nodes[i].children[0] = LittleLongl(aasworld.nodes[i].children[0]);
		aasworld.nodes[i].children[1] = LittleLongl(aasworld.nodes[i].children[1]);
	} //end for
	//cluster portals
	for (i = 0; i < aasworld.numportals; i++)
	{
		aasworld.portals[i].areanum = LittleLongl(aasworld.portals[i].areanum);
		aasworld.portals[i].frontcluster = LittleLongl(aasworld.portals[i].frontcluster);
		aasworld.portals[i].backcluster = LittleLongl(aasworld.portals[i].backcluster);
		aasworld.portals[i].clusterareanum[0] = LittleLongl(aasworld.portals[i].clusterareanum[0]);
		aasworld.portals[i].clusterareanum[1] = LittleLongl(aasworld.portals[i].clusterareanum[1]);
	} //end for
	//cluster portal index
	for (i = 0; i < aasworld.portalindexsize; i++)
	{
		aasworld.portalindex[i] = LittleLongl(aasworld.portalindex[i]);
	} //end for
	//cluster
	for (i = 0; i < aasworld.numclusters; i++)
	{
		aasworld.clusters[i].numareas = LittleLongl(aasworld.clusters[i].numareas);
		aasworld.clusters[i].numportals = LittleLongl(aasworld.clusters[i].numportals);
		aasworld.clusters[i].firstportal = LittleLongl(aasworld.clusters[i].firstportal);
	} //end for
} //end of the function AAS_SwapAASData
//===========================================================================
// dump the current loaded aas file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DumpAASData(void)
{
	/*
	if (aasworld.vertexes) FreeMemory(aasworld.vertexes);
	aasworld.vertexes = NULL;
	if (aasworld.planes) FreeMemory(aasworld.planes);
	aasworld.planes = NULL;
	if (aasworld.edges) FreeMemory(aasworld.edges);
	aasworld.edges = NULL;
	if (aasworld.edgeindex) FreeMemory(aasworld.edgeindex);
	aasworld.edgeindex = NULL;
	if (aasworld.faces) FreeMemory(aasworld.faces);
	aasworld.faces = NULL;
	if (aasworld.faceindex) FreeMemory(aasworld.faceindex);
	aasworld.faceindex = NULL;
	if (aasworld.areas) FreeMemory(aasworld.areas);
	aasworld.areas = NULL;
	if (aasworld.areasettings) FreeMemory(aasworld.areasettings);
	aasworld.areasettings = NULL;
	if (aasworld.reachability) FreeMemory(aasworld.reachability);
	aasworld.reachability = NULL;
	*/
	aasworld.loaded = qfalse;
} //end of the function AAS_DumpAASData
//===========================================================================
// allocate memory and read a lump of a AAS file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *AAS_LoadAASLump(FILE *fp, int offset, int length, void *buf)
{
	if (!length)
	{
		printf("lump size 0\n");
		return (char*)buf;
	} //end if
	//seek to the data
	if (fseek(fp, offset, SEEK_SET))
	{
		AAS_Error("can't seek to lump\n");
		AAS_DumpAASData();
		fclose(fp);
		return 0;
	} //end if
	//allocate memory
	if (!buf) buf = (void *) GetClearedMemory(length);
	//read the data
	if (fread((char *) buf, 1, length, fp) != (size_t)length)
	{
		AAS_Error("can't read lump\n");
		FreeMemory(buf);
		AAS_DumpAASData();
		fclose(fp);
		return NULL;
	} //end if
	return (char*)buf;
} //end of the function AAS_LoadAASLump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DData(unsigned char *data, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		data[i] ^= (unsigned char) i * 119;
	} //end for
} //end of the function AAS_DData
//===========================================================================
// load an aas file
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean AAS_LoadAASFile(char *filename, int fpoffset, int fplength)
{
	FILE *fp;
	aas_header_t header;
	int offset, length;

	//dump current loaded aas file
	AAS_DumpAASData();
	//open the file
	fp = fopen(filename, "rb");
	if (!fp)
	{
		AAS_Error("can't open %s\n", filename);
		return qfalse;
	} //end if
	//seek to the correct position (in the pak file)
	if (fseek(fp, fpoffset, SEEK_SET))
	{
		AAS_Error("can't seek to file %s\n");
		fclose(fp);
		return qfalse;
	} //end if
	//read the header
	if (fread(&header, sizeof(aas_header_t), 1, fp) != 1)
	{
		AAS_Error("can't read header of file %s\n", filename);
		fclose(fp);
		return qfalse;
	} //end if
	//check header identification
	header.ident = LittleLongl(header.ident);
	if (header.ident != AASID)
	{
		AAS_Error("%s is not an AAS file\n", filename);
		fclose(fp);
		return qfalse;
	} //end if
	//check the version
	header.version = LittleLongl(header.version);
	if (header.version != AASVERSION_OLD && header.version != AASVERSION)
	{
		AAS_Error("%s is version %i, not %i\n", filename, header.version, AASVERSION);
		fclose(fp);
		return qfalse;
	} //end if
	//
	if (header.version == AASVERSION)
	{
		AAS_DData((unsigned char *) &header + 8, sizeof(aas_header_t) - 8);
	} //end if
	aasworld.bspchecksum = LittleLongl(header.bspchecksum);
	//load the lumps:
	//bounding boxes
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_BBOXES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_BBOXES].filelen);
	aasworld.bboxes = (aas_bbox_t *) AAS_LoadAASLump(fp, offset, length, aasworld.bboxes);
	if (!aasworld.bboxes) return qfalse;
	aasworld.numbboxes = length / sizeof(aas_bbox_t);
	//vertexes
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_VERTEXES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_VERTEXES].filelen);
	aasworld.vertexes = (aas_vertex_t *) AAS_LoadAASLump(fp, offset, length, aasworld.vertexes);
	if (!aasworld.vertexes) return qfalse;
	aasworld.numvertexes = length / sizeof(aas_vertex_t);
	//planes
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_PLANES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_PLANES].filelen);
	aasworld.planes = (aas_plane_t *) AAS_LoadAASLump(fp, offset, length, aasworld.planes);
	if (!aasworld.planes) return qfalse;
	aasworld.numplanes = length / sizeof(aas_plane_t);
	//edges
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_EDGES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_EDGES].filelen);
	aasworld.edges = (aas_edge_t *) AAS_LoadAASLump(fp, offset, length, aasworld.edges);
	if (!aasworld.edges) return qfalse;
	aasworld.numedges = length / sizeof(aas_edge_t);
	//edgeindex
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_EDGEINDEX].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_EDGEINDEX].filelen);
	aasworld.edgeindex = (aas_edgeindex_t *) AAS_LoadAASLump(fp, offset, length, aasworld.edgeindex);
	if (!aasworld.edgeindex) return qfalse;
	aasworld.edgeindexsize = length / sizeof(aas_edgeindex_t);
	//faces
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_FACES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_FACES].filelen);
	aasworld.faces = (aas_face_t *) AAS_LoadAASLump(fp, offset, length, aasworld.faces);
	if (!aasworld.faces) return qfalse;
	aasworld.numfaces = length / sizeof(aas_face_t);
	//faceindex
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_FACEINDEX].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_FACEINDEX].filelen);
	aasworld.faceindex = (aas_faceindex_t *) AAS_LoadAASLump(fp, offset, length, aasworld.faceindex);
	if (!aasworld.faceindex) return qfalse;
	aasworld.faceindexsize = length / sizeof(int);
	//convex areas
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_AREAS].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_AREAS].filelen);
	aasworld.areas = (aas_area_t *) AAS_LoadAASLump(fp, offset, length, aasworld.areas);
	if (!aasworld.areas) return qfalse;
	aasworld.numareas = length / sizeof(aas_area_t);
	//area settings
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_AREASETTINGS].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_AREASETTINGS].filelen);
	aasworld.areasettings = (aas_areasettings_t *) AAS_LoadAASLump(fp, offset, length, aasworld.areasettings);
	if (!aasworld.areasettings) return qfalse;
	aasworld.numareasettings = length / sizeof(aas_areasettings_t);
	//reachability list
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_REACHABILITY].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_REACHABILITY].filelen);
	aasworld.reachability = (aas_reachability_t *) AAS_LoadAASLump(fp, offset, length, aasworld.reachability);
	if (length && !aasworld.reachability) return qfalse;
	aasworld.reachabilitysize = length / sizeof(aas_reachability_t);
	//nodes
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_NODES].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_NODES].filelen);
	aasworld.nodes = (aas_node_t *) AAS_LoadAASLump(fp, offset, length, aasworld.nodes);
	if (!aasworld.nodes) return qfalse;
	aasworld.numnodes = length / sizeof(aas_node_t);
	//cluster portals
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_PORTALS].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_PORTALS].filelen);
	aasworld.portals = (aas_portal_t *) AAS_LoadAASLump(fp, offset, length, aasworld.portals);
	if (length && !aasworld.portals) return qfalse;
	aasworld.numportals = length / sizeof(aas_portal_t);
	//cluster portal index
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_PORTALINDEX].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_PORTALINDEX].filelen);
	aasworld.portalindex = (aas_portalindex_t *) AAS_LoadAASLump(fp, offset, length, aasworld.portalindex);
	if (length && !aasworld.portalindex) return qfalse;
	aasworld.portalindexsize = length / sizeof(aas_portalindex_t);
	//clusters
	offset = fpoffset + LittleLongl(header.lumps[AASLUMP_CLUSTERS].fileofs);
	length = LittleLongl(header.lumps[AASLUMP_CLUSTERS].filelen);
	aasworld.clusters = (aas_cluster_t *) AAS_LoadAASLump(fp, offset, length, aasworld.clusters);
	if (length && !aasworld.clusters) return qfalse;
	aasworld.numclusters = length / sizeof(aas_cluster_t);
	//swap everything
	AAS_SwapAASData();
	//aas file is loaded
	aasworld.loaded = qtrue;
	//close the file
	fclose(fp);
	return qtrue;
} //end of the function AAS_LoadAASFile
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int AAS_WriteAASLump(FILE *fp, aas_header_t *h, int lumpnum, void *data, int length)
{
	aas_lump_t *lump;

	lump = &h->lumps[lumpnum];

	lump->fileofs = LittleLongl(ftell(fp));
	lump->filelen = LittleLongl(length);

	if (length > 0)
	{
		if (fwrite(data, length, 1, fp) < 1)
		{
			Log_Print("error writing lump %s\n", lumpnum);
			fclose(fp);
			return qfalse;
		} //end if
	} //end if
	return qtrue;
} //end of the function AAS_WriteAASLump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowNumReachabilities(int tt, char *name)
{
	int i, num;

	num = 0;
	for (i = 0; i < aasworld.reachabilitysize; i++)
	{
		if ((aasworld.reachability[i].traveltype & TRAVELTYPE_MASK) == tt)
			num++;
	} //end for
	Log_Print("%6d %s\n", num, name);
} //end of the function AAS_ShowNumReachabilities
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowTotals(void)
{
	Log_Print("numvertexes = %d\r\n", aasworld.numvertexes);
	Log_Print("numplanes = %d\r\n", aasworld.numplanes);
	Log_Print("numedges = %d\r\n", aasworld.numedges);
	Log_Print("edgeindexsize = %d\r\n", aasworld.edgeindexsize);
	Log_Print("numfaces = %d\r\n", aasworld.numfaces);
	Log_Print("faceindexsize = %d\r\n", aasworld.faceindexsize);
	Log_Print("numareas = %d\r\n", aasworld.numareas);
	Log_Print("numareasettings = %d\r\n", aasworld.numareasettings);
	Log_Print("reachabilitysize = %d\r\n", aasworld.reachabilitysize);
	Log_Print("numnodes = %d\r\n", aasworld.numnodes);
	Log_Print("numportals = %d\r\n", aasworld.numportals);
	Log_Print("portalindexsize = %d\r\n", aasworld.portalindexsize);
	Log_Print("numclusters = %d\r\n", aasworld.numclusters);
	AAS_ShowNumReachabilities(TRAVEL_WALK, "walk");
	AAS_ShowNumReachabilities(TRAVEL_CROUCH, "crouch");
	AAS_ShowNumReachabilities(TRAVEL_BARRIERJUMP, "barrier jump");
	AAS_ShowNumReachabilities(TRAVEL_JUMP, "jump");
	AAS_ShowNumReachabilities(TRAVEL_LADDER, "ladder");
	AAS_ShowNumReachabilities(TRAVEL_WALKOFFLEDGE, "walk off ledge");
	AAS_ShowNumReachabilities(TRAVEL_SWIM, "swim");
	AAS_ShowNumReachabilities(TRAVEL_WATERJUMP, "water jump");
	AAS_ShowNumReachabilities(TRAVEL_TELEPORT, "teleport");
	AAS_ShowNumReachabilities(TRAVEL_ELEVATOR, "elevator");
	AAS_ShowNumReachabilities(TRAVEL_ROCKETJUMP, "rocket jump");
	AAS_ShowNumReachabilities(TRAVEL_BFGJUMP, "bfg jump");
	AAS_ShowNumReachabilities(TRAVEL_GRAPPLEHOOK, "grapple hook");
	AAS_ShowNumReachabilities(TRAVEL_DOUBLEJUMP, "double jump");
	AAS_ShowNumReachabilities(TRAVEL_RAMPJUMP, "ramp jump");
	AAS_ShowNumReachabilities(TRAVEL_STRAFEJUMP, "strafe jump");
	AAS_ShowNumReachabilities(TRAVEL_JUMPPAD, "jump pad");
	AAS_ShowNumReachabilities(TRAVEL_FUNCBOB, "func bob");
} //end of the function AAS_ShowTotals
//===========================================================================
// aas data is useless after writing to file because it is byte swapped
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean AAS_WriteAASFile(char *filename)
{
	aas_header_t header;
	FILE *fp;

	Log_Print("writing %s\n", filename);
	AAS_ShowTotals();
	//swap the aas data
	AAS_SwapAASData();
	//initialize the file header
	memset(&header, 0, sizeof(aas_header_t));
	header.ident = LittleLongl(AASID);
	header.version = LittleLongl(AASVERSION);
	header.bspchecksum = LittleLongl(aasworld.bspchecksum);
	//open a new file
	fp = fopen(filename, "wb");
	if (!fp)
	{
		Log_Print("error opening %s\n", filename);
		return qfalse;
	} //end if
	//write the header
	if (fwrite(&header, sizeof(aas_header_t), 1, fp) < 1)
	{
		fclose(fp);
		return qfalse;
	} //end if
	//add the data lumps to the file
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_BBOXES, aasworld.bboxes,
		aasworld.numbboxes * sizeof(aas_bbox_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_VERTEXES, aasworld.vertexes,
		aasworld.numvertexes * sizeof(aas_vertex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PLANES, aasworld.planes,
		aasworld.numplanes * sizeof(aas_plane_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_EDGES, aasworld.edges,
		aasworld.numedges * sizeof(aas_edge_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_EDGEINDEX, aasworld.edgeindex,
		aasworld.edgeindexsize * sizeof(aas_edgeindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_FACES, aasworld.faces,
		aasworld.numfaces * sizeof(aas_face_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_FACEINDEX, aasworld.faceindex,
		aasworld.faceindexsize * sizeof(aas_faceindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_AREAS, aasworld.areas,
		aasworld.numareas * sizeof(aas_area_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_AREASETTINGS, aasworld.areasettings,
		aasworld.numareasettings * sizeof(aas_areasettings_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_REACHABILITY, aasworld.reachability,
		aasworld.reachabilitysize * sizeof(aas_reachability_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_NODES, aasworld.nodes,
		aasworld.numnodes * sizeof(aas_node_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PORTALS, aasworld.portals,
		aasworld.numportals * sizeof(aas_portal_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_PORTALINDEX, aasworld.portalindex,
		aasworld.portalindexsize * sizeof(aas_portalindex_t))) return qfalse;
	if (!AAS_WriteAASLump(fp, &header, AASLUMP_CLUSTERS, aasworld.clusters,
		aasworld.numclusters * sizeof(aas_cluster_t))) return qfalse;
	//rewrite the header with the added lumps
	fseek(fp, 0, SEEK_SET);
	AAS_DData((unsigned char *) &header + 8, sizeof(aas_header_t) - 8);
	if (fwrite(&header, sizeof(aas_header_t), 1, fp) < 1)
	{
		fclose(fp);
		return qfalse;
	} //end if
	//close the file
	fclose(fp);
	return qtrue;
} //end of the function AAS_WriteAASFile

