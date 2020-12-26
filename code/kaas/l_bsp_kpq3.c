
#include "l_cmd.h"
#include "l_mem.h"
#include "../qcommon/qfiles.h"

int				kpq3_nummodels;
dmodel_t		*kpq3_dmodels;//[MAX_MAP_MODELS];

int				kpq3_numShaders;
dshader_t	*kpq3_dshaders;//[kpq3_MAX_MAP_SHADERS];

int				kpq3_entdatasize;
char			*kpq3_dentdata;//[Q3_MAX_MAP_ENTSTRING];

int				kpq3_numleafs;
dleaf_t		*kpq3_dleafs;//[Q3_MAX_MAP_LEAFS];

int				kpq3_numplanes;
dplane_t		*kpq3_dplanes;//[Q3_MAX_MAP_PLANES];

int				kpq3_numnodes;
dnode_t		*kpq3_dnodes;//[Q3_MAX_MAP_NODES];

int				kpq3_numleafsurfaces;
int				*kpq3_dleafsurfaces;//[Q3_MAX_MAP_LEAFFACES];

int				kpq3_numleafbrushes;
int				*kpq3_dleafbrushes;//[Q3_MAX_MAP_LEAFBRUSHES];

int				kpq3_numbrushes;
dbrush_t		*kpq3_dbrushes;//[Q3_MAX_MAP_BRUSHES];

int				kpq3_numbrushsides;
dbrushside_t	*kpq3_dbrushsides;//[Q3_MAX_MAP_BRUSHSIDES];

int				kpq3_numLightBytes;
byte			*kpq3_lightBytes;//[Q3_MAX_MAP_LIGHTING];

int				kpq3_numGridPoints;
byte			*kpq3_gridData;//[Q3_MAX_MAP_LIGHTGRID];

int				kpq3_numVisBytes;
byte			*kpq3_visBytes;//[Q3_MAX_MAP_VISIBILITY];

int				kpq3_numDrawVerts;
drawVert_t	*kpq3_drawVerts;//[Q3_MAX_MAP_DRAW_VERTS];

int				kpq3_numDrawIndexes;
int				*kpq3_drawIndexes;//[Q3_MAX_MAP_DRAW_INDEXES];

int				kpq3_numDrawSurfaces;
dsurface_t	*kpq3_drawSurfaces;//[Q3_MAX_MAP_DRAW_SURFS];

int				kpq3_numFogs;
dfog_t		*kpq3_dfogs;//[Q3_MAX_MAP_FOGS];

char			kpq3_dbrushsidetextured[Q3_MAX_MAP_BRUSHSIDES];

extern qboolean forcesidesvisible;

/*
=============
Q3_CopyLump
=============
*/
int KPQ3_CopyLump( dheader_t	*header, int lump, void **dest, int size ) {
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;
	
	if ( length % size ) {
		Error ("KPQ3_LoadBSPFile: odd lump size");
	}

	*dest = GetMemory(length);

	memcpy( *dest, (byte *)header + ofs, length );

	return length / size;
}

void KPQ3_ParseEntities(void)
{
#if 0 
	numEntities = 0;
	ParseFromMemory(kpq3_dentdata, kpq3_entdatasize);
	while(ParseEntity());

	/* ydnar: set number of bsp entities in case a map is loaded on top */
	numBSPEntities = numEntities;
#else
	script_t *script;

	num_entities = 0;
	script = LoadScriptMemory(kpq3_dentdata, kpq3_entdatasize, "*Quake3 bsp file");
	SetScriptFlags(script, SCFL_NOSTRINGWHITESPACES |
									SCFL_NOSTRINGESCAPECHARS);

	while(ParseEntity(script))
	{
	} //end while

	FreeScript(script);
#endif
}
#if 0
static void CopyLightGridLumps(dheader_t * header)
{
	int             i, j;
	xbspGridPoint_t *in;
	bspGridPoint_t *out;


	/* get count */
	numBSPGridPoints = GetLumpElements((bspHeader_t *) header, LUMP_LIGHTGRID, sizeof(*in));

	/* allocate buffer */
	bspGridPoints = safe_malloc(numBSPGridPoints * sizeof(*bspGridPoints));
	memset(bspGridPoints, 0, numBSPGridPoints * sizeof(*bspGridPoints));

	/* copy */
	in = GetLump((bspHeader_t *) header, LUMP_LIGHTGRID);
	out = bspGridPoints;
	for(i = 0; i < numBSPGridPoints; i++)
	{
		for(j = 0; j < MAX_LIGHTMAPS; j++)
		{
			VectorCopy(in->ambient, out->ambient[j]);
			VectorCopy(in->directed, out->directed[j]);
			out->styles[j] = LS_NONE;
		}

		out->styles[0] = LS_NORMAL;

		out->latLong[0] = in->latLong[0];
		out->latLong[1] = in->latLong[1];

		in++;
		out++;
	}
}
#endif
void KPQ3_LoadBSPFile(struct quakefile_s *qf)
{
	xbspHeader_t   *header;

	/* load the file header */
	//LoadFile(filename, (void **)&header);
	LoadQuakeFile(qf, (void **)&header);

	/* swap the header (except the first 4 bytes) */
	Q3_SwapBlock((int *)((byte *) header + sizeof(int)), sizeof(*header) - sizeof(int));

	/* make sure it matches the format we're trying to load */
	if ( header->ident != BSP_IDENT ) {
		Error( "%s is not a IBSP file", qf->filename );
	}
	if ( header->version != BSP_VERSION ) {
		Error( "%s is version %i, not %i", qf->filename, header->version, Q3_BSP_VERSION );
	}

	/* load/convert lumps */
	numBSPShaders = KPQ3_CopyLump((bspHeader_t *) header, LUMP_SHADERS,  (void *) &q3_dshaders, sizeof(dShader_t));

	numBSPModels = KPQ3_CopyLump((bspHeader_t *) header, LUMP_MODELS, bspModels, sizeof(bspModel_t));

	numBSPPlanes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_PLANES, bspPlanes, sizeof(bspPlane_t));

	numBSPLeafs = KPQ3_CopyLump((bspHeader_t *) header, LUMP_LEAFS, bspLeafs, sizeof(bspLeaf_t));

	numBSPNodes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_NODES, bspNodes, sizeof(bspNode_t));

	numBSPLeafSurfaces = KPQ3_CopyLump((bspHeader_t *) header, LUMP_LEAFSURFACES, bspLeafSurfaces, sizeof(bspLeafSurfaces[0]));

	numBSPLeafBrushes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_LEAFBRUSHES, bspLeafBrushes, sizeof(bspLeafBrushes[0]));

	numBSPBrushes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_BRUSHES, bspBrushes, sizeof(bspBrush_t));

//	CopyBrushSidesLump(header);

//	CopyDrawVertsLump(header);

//	CopyDrawSurfacesLump(header);

	numBSPFogs = KPQ3_CopyLump((bspHeader_t *) header, LUMP_FOGS, bspFogs, sizeof(bspFog_t));

	numBSPDrawIndexes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_DRAWINDEXES, bspDrawIndexes, sizeof(bspDrawIndexes[0]));

	numBSPVisBytes = KPQ3_CopyLump((bspHeader_t *) header, LUMP_VISIBILITY, bspVisBytes, 1);

	numBSPLightBytes = GetLumpElements((bspHeader_t *) header, LUMP_LIGHTMAPS, 1);
	bspLightBytes = safe_malloc(numBSPLightBytes);
	KPQ3_CopyLump((bspHeader_t *) header, LUMP_LIGHTMAPS, bspLightBytes, 1);

	bspEntDataSize = KPQ3_CopyLump((bspHeader_t *) header, LUMP_ENTITIES, bspEntData, 1);

//	CopyLightGridLumps(header);

	/* free the file buffer */
	free(header);
}
