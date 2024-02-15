

#include "l_cmd.h"
#include "l_mem.h"
#include "../qcommon/qfiles.h"
#include "../botlib/l_script.h"
#include "l_bsp_ent.h"

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

char			kpq3_dbrushsidetextured[MAX_MAP_BRUSHSIDES];

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
	dheader_t   *header;//xbspHeader_t

	/* load the file header */
	//LoadFile(filename, (void **)&header);
	LoadQuakeFile(qf, (void **)&header);

	/* swap the header (except the first 4 bytes) */
	Q3_SwapBlock((int *)((byte *) header + sizeof(int)), sizeof(*header) - sizeof(int));

	/* make sure it matches the format we're trying to load */
	if ( header->ident != BSP_IDENT_KPQ3 ) {
		Error( "%s is not a IBSP file", qf->filename );
	}
	if ( header->version != BSP_VERSION_KPQ3 ) {
		Error( "%s is version %i, not %i", qf->filename, header->version, BSP_VERSION_KPQ3 );
	}

	/* load/convert lumps */
	kpq3_numShaders = KPQ3_CopyLump((dheader_t *) header, LUMP_SHADERS,  (void **) &kpq3_dshaders, sizeof(dshader_t));

	kpq3_nummodels = KPQ3_CopyLump((dheader_t *) header, LUMP_MODELS, (void **) &kpq3_dmodels, sizeof(dmodel_t));

	kpq3_numplanes = KPQ3_CopyLump((dheader_t *) header, LUMP_PLANES, (void **) &kpq3_dplanes, sizeof(dplane_t));

	kpq3_numleafs = KPQ3_CopyLump((dheader_t *) header, LUMP_LEAFS, (void **) &kpq3_dleafs, sizeof(dleaf_t));

	kpq3_numnodes = KPQ3_CopyLump((dheader_t *) header, LUMP_NODES, (void **) &kpq3_dnodes, sizeof(dnode_t));

	kpq3_numleafsurfaces = KPQ3_CopyLump((dheader_t *) header, LUMP_LEAFSURFACES, (void **) &kpq3_dleafsurfaces, sizeof(kpq3_dleafsurfaces[0]));

	kpq3_numleafbrushes = KPQ3_CopyLump((dheader_t *) header, LUMP_LEAFBRUSHES, (void **) &kpq3_dleafbrushes, sizeof(kpq3_dleafbrushes[0]));

	kpq3_numbrushes = KPQ3_CopyLump((dheader_t *) header, LUMP_BRUSHES, (void **) &kpq3_dbrushes, sizeof(dbrush_t));

//	CopyBrushSidesLump(header);

//	CopyDrawVertsLump(header);

//	CopyDrawSurfacesLump(header);

	kpq3_numFogs = KPQ3_CopyLump((dheader_t *) header, LUMP_FOGS, (void **) &kpq3_dfogs, sizeof(dfog_t));

	kpq3_numDrawIndexes = KPQ3_CopyLump((dheader_t *) header, LUMP_DRAWINDEXES, (void **) &kpq3_drawIndexes, sizeof(kpq3_drawIndexes[0]));

	kpq3_numVisBytes = KPQ3_CopyLump((dheader_t *) header, LUMP_VISIBILITY, (void **) &kpq3_visBytes, 1);

	kpq3_numLightBytes = GetLumpElements((dheader_t *) header, LUMP_LIGHTMAPS, 1);
	kpq3_lightBytes = safe_malloc(kpq3_numLightBytes);
	KPQ3_CopyLump((dheader_t *) header, LUMP_LIGHTMAPS, (void **) &kpq3_lightBytes, 1);

	kpq3_entdatasize = KPQ3_CopyLump((dheader_t *) header, LUMP_ENTITIES, (void **) &kpq3_dentdata, 1);

//	CopyLightGridLumps(header);

	/* free the file buffer */
	free(header);
}
