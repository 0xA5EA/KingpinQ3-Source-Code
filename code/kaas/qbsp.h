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


#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#endif

#include <stdlib.h> //hypov8 bspc
#include <malloc.h>
/////#include "../qcommon/q_platform.h"
//#include "../qcommon/q_platform.h"
//#include "../qcommon/qfiles.h"
#include "../qcommon/q_shared.h"

//#include "../qcommon/surfaceflags.h"
//#include "../qcommon/cm_polylib.h"

#include "l_cmd.h"
#include "../kaas/l_math.h"
#include "l_poly.h"
#include "l_threads.h"
#include "../botlib/l_script.h"
#include "../botlib/botlib.h" //hypov8 add: ->bsp_trace_t
#include "l_bsp_ent.h"
//#include "q2files.h"
#include "l_mem.h"
#include "../kaas/l_utils.h"
#include "../kaas/l_log.h"
#include "l_qfiles.h"

#if 0
#define USE_ASM_LIB_
#ifdef USE_ASM_LIB_
#include "../asmlib/asmlib.h"
#define stricmp stricmp_az
#define Q_stricmp stricmp
#endif
#endif
//FIXME (0xA5EA):

#define KAAS_VERSION  "2.2.1"
#if defined (USE_ASM_LIB) && !defined (USE_SSE)
#  define BSPC_VERSION  KAAS_VERSION" with asmlib"
#elif !defined (USE_ASM_LIB) && defined (USE_SSE)
#  define BSPC_VERSION  KAAS_VERSION" sse2"
#elif defined (USE_ASM_LIB) && defined (USE_SSE)
#  define BSPC_VERSION  KAAS_VERSION" with asmlib and sse2"
#else
#define BSPC_VERSION KAAS_VERSION
#endif

#define ME
#define DEBUG
#define NODELIST
#define SIN

#define MAX_BRUSH_SIDES		128		//maximum number of sides per brush
//#define CLIP_EPSILON		0.1
#define MAX_MAP_BOUNDS		65535
#define BOGUS_RANGE			(MAX_MAP_BOUNDS+128)	//somewhere outside the map
#define TEXINFO_NODE		-1		//side is allready on a node
#define PLANENUM_LEAF		-1		//used for leaf nodes
#define MAXEDGES			20		//maximum number of face edges
#define MAX_NODE_BRUSHES	8		//maximum brushes in a node
//side flags
#define SFL_TESTED			1
#define SFL_VISIBLE			2
#define SFL_BEVEL			4
#define SFL_TEXTURED		8
#define SFL_CURVE			16

//map plane
typedef struct plane_s
{
	vec3_t normal;
	vec_t dist;
	int type;
	int signbits;
	struct plane_s	*hash_chain;
} plane_t;
//brush texture
typedef struct
{
	vec_t	shift[2];
	vec_t	rotate;
	vec_t	scale[2];
	char	name[32];
	int		flags;
	int		value;
} brush_texture_t;
//brush side
#if 1
typedef struct side_s
{
	int				planenum;	// map plane this side is in
	int				texinfo;		// texture reference
	winding_t		*winding;	// winding of this side
	struct side_s	*original;	// bspbrush_t sides will reference the mapbrush_t sides
   int				lightinfo;	// for SIN only
	int				contents;	// from miptex
	int				surf;			// from miptex
	unsigned short flags;		// side flags
} side_t;		//sizeof(side_t) = 36
#else

#endif
//map brush
typedef struct mapbrush_s
{
	int		entitynum;
	int		brushnum;

	int		contents;
#ifdef ME
	int		expansionbbox;			//bbox used for expansion of the brush
	int		leafnum;
	int		modelnum;
#endif

	vec3_t	mins, maxs;

	int		numsides;
	side_t	*original_sides;
} mapbrush_t;
//bsp face
typedef struct face_s
{
	struct face_s		*next;		// on node

	// the chain of faces off of a node can be merged or split,
	// but each face_t along the way will remain in the chain
	// until the entire tree is freed
	struct face_s		*merged;	// if set, this face isn't valid anymore
	struct face_s		*split[2];	// if set, this face isn't valid anymore

	struct portal_s	*portal;
	int					texinfo;

	int					planenum;
	int					contents;	// faces in different contents can't merge
	int					outputnumber;
	winding_t			*w;
	int					numpoints;
	qboolean				badstartvert;	// tjunctions cannot be fixed without a midpoint vertex
	int					vertexnums[MAXEDGES];
} face_t;
#if 1
//bsp brush
typedef struct bspbrush_s
{
	struct		bspbrush_s	*next;
	vec3_t		mins, maxs;
	int			side, testside;		// side of node during construction
	mapbrush_t	*original;
	int			numsides;
	side_t		sides[6];			// variably sized
} bspbrush_t;	//sizeof(bspbrush_t) = 44 + numsides * sizeof(side_t)


//bsp node
typedef struct node_s
{
	//both leafs and nodes
	int				planenum;	// -1 = leaf node
	struct node_s	*parent;
	vec3_t			mins, maxs;	// valid after portalization
	bspbrush_t		*volume;		// one for each leaf/node

	// nodes only
	qboolean			detail_seperator;	// a detail brush caused the split
	side_t			*side;		// the side that created the node
	struct node_s	*children[2];
	face_t			*faces;

	// leafs only
	bspbrush_t		*brushlist;	// fragments of all brushes in this leaf
	int				contents;	// OR of all brush contents
	int				occupied;	// 1 or greater can reach entity
	entity_t			*occupant;	// for leak file testing
	int				cluster;		// for portalfile writing
	int				area;			// for areaportals
	struct portal_s *portals;	// also on nodes during construction
#ifdef NODELIST
	struct node_s *next;			//next node in the nodelist
#endif
#ifdef ME
	int expansionbboxes;			//OR of all bboxes used for expansion of the brushes
	int modelnum;
#endif
} bspnode_t;		//sizeof(node_t) = 80 bytes

#elif 0
typedef struct bspbrush_s
{
	struct bspbrush_s *next;
	struct bspbrush_s *nextColorModBrush;	/* ydnar: colorMod volume brushes go here */
	struct bspbrush *original;	/* chopped up brushes will reference the originals */

	int             entityNum, brushNum;	/* editor numbering */
	int             outputNum;	/* set when the brush is written to the file list */

	/* ydnar: for shadowcasting entities */
	int             castShadows;
	int             recvShadows;

	shaderInfo_t   *contentShader;
	shaderInfo_t   *celShader;	/* :) */

	/* ydnar: gs mods */
	int             lightmapSampleSize;	/* jal : entity based _lightmapsamplesize */
	float           lightmapScale;
	float           shadeAngleDegrees;	/* jal : entity based _shadeangle */
	vec3_t          eMins, eMaxs;
	indexMap_t     *im;

	int             contentFlags;
	int             compileFlags;	/* ydnar */
	qboolean        detail;
	qboolean        opaque;

	int             portalareas[2];

	vec3_t          mins, maxs;
	int             numsides;

	side_t          sides[6];	/* variably sized */
}
bspbrush_t;
typedef struct bspnode_s
{
	/* both leafs and nodes */
	int             planenum;	/* -1 = leaf node */
	struct bspnode_s  *parent;
	vec3_t          mins, maxs;	/* valid after portalization */
	brush_t        *volume;		/* one for each leaf/node */

	/* nodes only */
	side_t         *side;		/* the side that created the node */
	struct bspnode_s  *children[2];
	int             compileFlags;	/* ydnar: hint, antiportal */
	int             tinyportals;
	vec3_t          referencepoint;

	/* leafs only */
	qboolean        opaque;		/* view can never be inside */
	qboolean        areaportal;
	qboolean        skybox;		/* ydnar: a skybox leaf */
	qboolean        sky;		/* ydnar: a sky leaf */
	int             cluster;	/* for portalfile writing */
	int             area;		/* for areaportals */
	brush_t        *brushlist;	/* fragments of all brushes in this leaf */
	drawSurfRef_t  *drawSurfReferences;

	int             occupied;	/* 1 or greater can reach entity */
	entity_t       *occupant;	/* for leak file testing */

	struct portal_s *portals;	/* also on nodes during construction */
	qboolean        has_structural_children;
}
bspnode_t;
#else

typedef struct node_s
{
	// common with leaf and node
	int             contents;	// -1 for nodes, to differentiate from leafs
	int             visCounts[MAX_VISCOUNTS];	// node needs to be traversed if current
	int             lightCount;
	vec3_t          mins, maxs;	// for bounding box culling
	struct node_s	*parent;

	// node specific
	cplane_t       *plane;
	struct node_s *children[2];

	// leaf specific
	int             cluster;
	int             area;

	int             numMarkSurfaces;
	bspSurface_t  **markSurfaces;
} bspnode_t;
#endif


//bsp portal
typedef struct portal_s
{
	plane_t plane;
	bspnode_t *onnode;					// NULL = outside box
	bspnode_t *nodes[2];				// [0] = front side of plane
	struct portal_s *next[2];
	winding_t *winding;

	qboolean	sidefound;			// false if ->side hasn't been checked
	side_t *side;					// NULL = non-visible
	face_t *face[2];				// output face in bsp file
#ifdef ME
	struct tmp_face_s *tmpface;		//pointer to the tmpface created for this portal
	int planenum;					//number of the map plane used by the portal
#endif
} portal_t;
//bsp tree
typedef struct
{
	bspnode_t		*headnode;
	bspnode_t		outside_node;
	vec3_t		mins, maxs;
} tree_t;	//identisch zu kmap2 und q3map2

//=============================================================================
// bspc.c
//=============================================================================

extern	qboolean noprune;
extern	qboolean nodetail;
extern	qboolean fulldetail;
extern	qboolean nomerge;
extern	qboolean nosubdiv;
extern	qboolean nowater;
extern	qboolean noweld;
extern	qboolean noshare;
extern	qboolean notjunc;
extern	qboolean onlyents;
#ifdef ME
extern	qboolean nocsg;
extern	qboolean create_aas;
extern	qboolean freetree;
extern	qboolean lessbrushes;
extern	qboolean nobrushmerge;
extern	qboolean cancelconversion;
extern	qboolean noliquids;
extern	qboolean capsule_collision;
#endif //ME

extern	float subdivide_size;
extern	vec_t microvolume;

extern	char outbase[32];
extern	char source[1024];

//=============================================================================
// map.c
//=============================================================================

#define MAX_MAPFILE_PLANES			512000	// 256000
#define MAX_MAPFILE_BRUSHES			65535
#define MAX_MAPFILE_BRUSHSIDES		(MAX_MAPFILE_BRUSHES*8)
#define MAX_MAPFILE_TEXINFO			8192

extern	int			entity_num;

extern	plane_t		mapplanes[MAX_MAPFILE_PLANES];
extern	int			nummapplanes;
extern	int			mapplaneusers[MAX_MAPFILE_PLANES];

extern	int			nummapbrushes;
extern	mapbrush_t	mapbrushes[MAX_MAPFILE_BRUSHES];

extern	vec3_t		map_mins, map_maxs;

extern	int			nummapbrushsides;
extern	side_t		brushsides[MAX_MAPFILE_BRUSHSIDES];
extern	brush_texture_t	side_brushtextures[MAX_MAPFILE_BRUSHSIDES];

#ifdef ME

typedef struct
{
	float	vecs[2][4];		// [s/t][xyz offset]
	int		flags;			// miptex flags + overrides
	int		value;
	char	texture[64];	// texture name (textures/*.wal)
	int		nexttexinfo;	// for animations, -1 = end of chain
} map_texinfo_t;

extern	map_texinfo_t		map_texinfo[MAX_MAPFILE_TEXINFO];
extern	int					map_numtexinfo;
#define NODESTACKSIZE		1024

//#define MAPTYPE_QUAKE1		1
//#define MAPTYPE_QUAKE2		2
#define MAPTYPE_QUAKE3		3
//#define MAPTYPE_HALFLIFE	4
//#define MAPTYPE_SIN			5

extern	int nodestack[NODESTACKSIZE];
extern	int *nodestackptr;
extern	int nodestacksize;
extern	int brushmodelnumbers[MAX_MAPFILE_BRUSHES];
extern	int dbrushleafnums[MAX_MAPFILE_BRUSHES];
extern	int dplanes2mapplanes[MAX_MAPFILE_PLANES];

extern	int loadedmaptype;
#endif //ME

extern	int c_boxbevels;
extern	int c_edgebevels;
extern	int c_areaportals;
extern	int c_clipbrushes;
extern	int c_squattbrushes;

//finds a float plane for the given normal and distance
int FindFloatPlane(vec3_t normal, vec_t dist);
//returns the plane type for the given normal
#if 0	// q2 related
int BSPC_PlaneTypeForNormal(vec3_t normal);
#endif
//returns the plane defined by the three given points
int BSPC_PlaneFromPoints(int *p0, int *p1, int *p2);
//add bevels to the map brush
void AddBrushBevels(mapbrush_t *b);
//makes brush side windings for the brush
qboolean MakeBrushWindings(mapbrush_t *ob);
//marks brush bevels of the brush as bevel
void MarkBrushBevels(mapbrush_t *brush);
//returns true if the map brush already exists
int BrushExists(mapbrush_t *brush);
//loads a map from a bsp file
int LoadMapFromBSP(struct quakefile_s *qf);
//resets map loading
void ResetMapLoading(void);
//print some map info
void PrintMapInfo(void);
//writes a map file (type depending on loaded map type)
void WriteMapFile(char *filename);

//=============================================================================
// map_q2.c
//=============================================================================

void Q2_ResetMapLoading(void);
//loads a Quake2 map file
void Q2_LoadMapFile(char *filename);
//loads a map from a Quake2 bsp file
void Q2_LoadMapFromBSP(char *filename, int offset, int length);

//=============================================================================
// map_q1.c
//=============================================================================

void Q1_ResetMapLoading(void);
//loads a Quake2 map file
void Q1_LoadMapFile(char *filename);
//loads a map from a Quake1 bsp file
void Q1_LoadMapFromBSP(char *filename, int offset, int length);

//=============================================================================
// map_q3.c
//=============================================================================
void Q3_ResetMapLoading(void);
//loads a map from a Quake3 bsp file
void Q3_LoadMapFromBSP(struct quakefile_s *qf);

//=============================================================================
// map_sin.c
//=============================================================================

void Sin_ResetMapLoading(void);
//loads a Sin map file
void Sin_LoadMapFile(char *filename);
//loads a map from a Sin bsp file
void Sin_LoadMapFromBSP(char *filename, int offset, int length);

//=============================================================================
// map_hl.c
//=============================================================================

void HL_ResetMapLoading(void);
//loads a Half-Life map file
void HL_LoadMapFile(char *filename);
//loads a map from a Half-Life bsp file
void HL_LoadMapFromBSP(char *filename, int offset, int length);

//=============================================================================
// textures.c
//=============================================================================

typedef struct
{
	char	name[64];
	int		flags;
	int		value;
	int		contents;
	char	animname[64];
} textureref_t;

#define	MAX_MAP_TEXTURES	1024

extern	textureref_t	textureref[MAX_MAP_TEXTURES];

int FindMiptex(char *name);
#if 0	// 0xA5EA: q2 related
int TexinfoForBrushTexture(plane_t *plane, brush_texture_t *bt, vec3_t origin);
#endif
void TextureAxisFromPlane(plane_t *pln, vec3_t xv, vec3_t yv);

//=============================================================================
// csg
//=============================================================================

bspbrush_t *MakeBspBrushList(int startbrush, int endbrush, vec3_t clipmins, vec3_t clipmaxs);
bspbrush_t *ChopBrushes(bspbrush_t *head);
bspbrush_t *InitialBrushList(bspbrush_t *list);
bspbrush_t *OptimizedBrushList(bspbrush_t *list);
void WriteBrushMap(char *name, bspbrush_t *list);
void CheckBSPBrush(bspbrush_t *brush);
void BSPBrushWindings(bspbrush_t *brush);
bspbrush_t *TryMergeBrushes(bspbrush_t *brush1, bspbrush_t *brush2);
tree_t *ProcessWorldBrushes(int brush_start, int brush_end);

//=============================================================================
// brushbsp
//=============================================================================

#define	PSIDE_FRONT			1
#define	PSIDE_BACK			2
#define	PSIDE_BOTH			(PSIDE_FRONT|PSIDE_BACK)
#define	PSIDE_FACING		4

void WriteBrushList(char *name, bspbrush_t *brush, qboolean onlyvis);
bspbrush_t *CopyBrush(bspbrush_t *brush);
void SplitBrush(bspbrush_t *brush, int planenum, bspbrush_t **front, bspbrush_t **back);
bspnode_t *AllocNode(void);
bspbrush_t *AllocBrush(int numsides);
int CountBrushList(bspbrush_t *brushes);
void FreeBrush(bspbrush_t *brushes);
vec_t BrushVolume(bspbrush_t *brush);
void BoundBrush(bspbrush_t *brush);
void FreeBrushList(bspbrush_t *brushes);
tree_t *BrushBSP(bspbrush_t *brushlist, vec3_t mins, vec3_t maxs);
bspbrush_t *BrushFromBounds(vec3_t mins, vec3_t maxs);
int BrushMostlyOnSide(bspbrush_t *brush, plane_t *plane);
qboolean WindingIsHuge(winding_t *w);
qboolean WindingIsTiny(winding_t *w);
void ResetBrushBSP(void);

//=============================================================================
// portals.c
//=============================================================================

int VisibleContents (int contents);
void MakeHeadnodePortals (tree_t *tree);
void MakeNodePortal (bspnode_t *node);
void SplitNodePortals (bspnode_t *node);
qboolean Portal_VisFlood (portal_t *p);
qboolean FloodEntities (tree_t *tree);
void FillOutside (bspnode_t *headnode);
void FloodAreas (tree_t *tree);
void MarkVisibleSides (tree_t *tree, int start, int end);
void FreePortal (portal_t *p);
void EmitAreaPortals (bspnode_t *headnode);
void MakeTreePortals (tree_t *tree);

//=============================================================================
// glfile.c
//=============================================================================

void OutputWinding(winding_t *w, FILE *glview);
void WriteGLView(tree_t *tree, char *source);

//=============================================================================
// gldraw.c
//=============================================================================

extern vec3_t draw_mins, draw_maxs;
extern qboolean drawflag;

void Draw_ClearWindow (void);
void DrawWinding (winding_t *w);
void GLS_BeginScene (void);
void GLS_Winding (winding_t *w, int code);
void GLS_EndScene (void);

//=============================================================================
// leakfile.c
//=============================================================================

void LeakFile (tree_t *tree);

//=============================================================================
// tree.c
//=============================================================================

tree_t *Tree_Alloc(void);
void Tree_Free(tree_t *tree);
void Tree_Free_r(bspnode_t *node);
void Tree_Print_r(bspnode_t *node, int depth);
void Tree_FreePortals_r(bspnode_t *node);
void Tree_PruneNodes_r(bspnode_t *node);
void Tree_PruneNodes(bspnode_t *node);
