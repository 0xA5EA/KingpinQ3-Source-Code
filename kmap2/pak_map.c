
/*
hypov8
===========
pack map file into .pk3
generate a shader file that includes non kpq3 assets
these shaders will be renamed to prevent conflicts (eg maps using the same map models)
shader names are textures/kmap2_<mapname>/<index>
also .bsp internal shader names will be re-named


todo: 
	deal with tables?
	fix kmap image files
	cubemap images. 
	change pk3 file name/date?
	change compresion for other file types. eg png is uncompresed in zip.
	imagefile names are not renamed and will cause duplicates. should these be unique to?
	handle mod folder/s. this is also not loading global paths properly(local to vfs.c)
	check for missing -fs_path switches.
*/

#ifdef KMAP2
#include "kmap2.h"
#include "zip.h" //compress


typedef struct
{
	char newName[MAX_QPATH];
	char oldName[MAX_QPATH];
	char *script; //complete script
	int unique;
} shaderStruct;
shaderStruct pakMapShaders[MAX_SHADER_INFO];


static int isPakMap = 0; //todo: move to global?
static int numMapTextures = 0;
static int numShaderScripts = 0;
static char bspFilename[1024], bspTmpFilename[1024];
static char textureNames[MAX_SHADER_INFO][MAX_QPATH]; //image files size ok?

static qboolean ReadTexturesAdvancedModes(char *shaderText);


/*
==============
PakMap_Cleanup

if error is found, fix file names then print Error
==============
*/
static void PakMap_Cleanup(qboolean renamedMap, qboolean printError, char *str)
{
	int i;

	for (i = 0; i < numBSPShaders; i++)
	{
		if (pakMapShaders[i].unique == 1)
			free(pakMapShaders[i].script);
	}

	//delete shader file
	remove(mapShaderFile);

	if (renamedMap)
	{	//delete edited file
		remove(bspFilename);
		//rename old bsp back to original.
		rename(bspTmpFilename, bspFilename);		
	}

	Sys_Printf("=========================\n");

	if (printError)
		Error(str);
}



static void PakMap_ReNameShader()
{
	int i, j;
	char *c, newShaderName[256];


	if (mapName == NULL || mapName == '\0')
		PakMap_Cleanup(qtrue, qtrue, "mapname failed");

	for(i = 0; i < numBSPShaders; i++)
	{
		j = 0;
		//skip rename
		if (pakMapShaders[i].unique != 1)
			continue;

		c = &bspShaders[i].shader[0];
		//find first '/'
		while (j< 64 && c[0] != '/' && c[0] != '\\') {
			j++; c++;
		}
		j++; c++;
		//copy shader root folder
		Q_strncpyz(newShaderName, bspShaders[i].shader, j+1);
		sprintf(newShaderName, "%skmap2_%s", newShaderName, mapName);

		//find next '/'
		while (j< 64 && c[0] != '/' && c[0] != '\\') {
			j++; c++;
		}	

#if 0	//copy the end of shader.
		Q_strcat(newShaderName, 64, c);
#else	
		//todo: add index as name instead?
		Q_strcat(newShaderName, 64, va("/%.3i", i));
#endif
		//copy new shader name to .bsp and .mtr
		Q_strncpyz(bspShaders[i].shader, newShaderName, 64); //rename bsp shader
		Q_strncpyz(pakMapShaders[i].newName, newShaderName, 64); //rename .mtr name
	}
}

static qboolean PakMap_IgnoredFile()
{
	int i;

	//ignore internal and base images
	if (token[0] == '*' || token[0] == '$' || token[0] == '_' ||
		/* skip known base textures */
		!Q_strncasecmp(token, "textures/strombine", 18) ||
		!Q_strncasecmp(token, "textures/decals/", 16) ||
		!Q_strncasecmp(token, "textures/method/", 16) ||
		!Q_strncasecmp(token, "textures/common/", 16) || //textures not used in kpq3
		!Q_strncasecmp(token, "textures/skies/", 15) || //todo: in kpq3
		!Q_strncasecmp(token, "textures/color", 14) || //move to common?
		!Q_strncasecmp(token, "textures/kpq3_", 14) ||
		!Q_strncasecmp(token, "textures/misc_", 14) ||
		!Q_strncasecmp(token, "lights/kpq3/", 12) ||
		//Q_strnicmp(token, "cubemaps/", 9) || //todo: move default?
		!Q_strncasecmp(token, "sprites/", 8) ||
		!Q_strncasecmp(token, "gfx/", 4) ||
		!Q_strncasecmp(token, "ui/", 3))
	{
		return qtrue;
	}

	//check if we have the image allready
	for (i = 0; i < numMapTextures; i++)
	{
		if (textureNames[i][0] != '\0' && !Q_stricmp(token, textureNames[i]))
			return qtrue;
	}

	return qfalse;
}

static int PakMap_Shader_CheckForMatchingInBsp(char *shaderName)
{
	int i;

	//check if shader is used
	for(i = 0; i < numBSPShaders; i++)
	{
		//found match?
		if (!Q_stricmp(shaderName, bspShaders[i].shader))
			return i;
	}

	return -1;
}

static qboolean PakMap_Shader_CheckForMatchingShader(char *shaderName)
{
	int i;

	//check if shader is used
	for(i = 0; i < numShaderScripts; i++)
	{
		//found match?
		if (!Q_stricmp(shaderName, pakMapShaders[i].oldName))
			return qtrue; //skip
	}

	return qfalse;
}

static qboolean PakMap_SetTextureLists_global(char *shaderText/*, char *shaderName*/)
{
	int i;
	char *list1[8] = {"colorMap", "diffuseMap", "bumpMap", "normalMap", "specularMap", "glowMap", "reflectionMap", "reflectionMapBlended"};


	/// match kmap_
	if (!Q_strncasecmp(token, "kmap_", PREFIX_LEN) || !Q_strncasecmp(token, "xmap_", PREFIX_LEN))
	{	//kmap_cloneShader, kmap_remapShader, kmap_globaltexture
		if (!Q_stricmp(token + PREFIX_LEN, "backshader... cloneShader...remapShader...")) //todo: real names!!
		{
			ReadTexturesAdvancedModes(shaderText);
		}
	}
	else if (!Q_stricmp(token, "qer_editorimage"))
	{
		// skip texture
		GetTokenAppend(shaderText, qfalse);
	}
	else
	{
		for (i = 0; i < 8; i++)
		{
			if (!Q_stricmp(token, list1[i]))
			{
				ReadTexturesAdvancedModes(shaderText);
				break;
			}
		}
	}

	//ignore all other tokens on the line.
	while(TokenAvailable() && GetTokenAppend(shaderText, qfalse));

	return qtrue;
}

static qboolean PakMap_SetTextureLists_stage(char *shaderText)
{
	int i;

	if (!Q_stricmp(token, "map") || !Q_stricmp(token, "clampMap") || !Q_stricmp(token, "videoMap"))
	{
		ReadTexturesAdvancedModes(shaderText);
	}
	/*else if (!Q_stricmp(token, "cubeMap")) //todo:
	{
		readCubemapTextures(); 
	}*/
	else if (!Q_stricmp(token, "animMap"))
	{
		//skip fps number
		GetTokenAppend(shaderText, qfalse);

		//try max count of shaders
		for (i = 0; i < 24; i++)
		{
			if (TokenAvailable() && GetTokenAppend(shaderText, qfalse))
			{
				if (!PakMap_IgnoredFile())
				{
					strcpy(textureNames[numMapTextures], token);
					numMapTextures++;
				}
			}
			else
				break;
		}
		return qtrue;
	}


	/* ignore all other tokens on the line */
	while(TokenAvailable() && GetTokenAppend(shaderText, qfalse));

	return qtrue;
}


int PakMap_ReadShaderFile()
{
	int bspIndex;
	char shaderText[MAX_SHADER_INFO];
	int depth = 0;

	if (!isPakMap)
		return qfalse;

	bspIndex = PakMap_Shader_CheckForMatchingInBsp(token);

	if (bspIndex == -1 || PakMap_Shader_CheckForMatchingShader(token) || PakMap_IgnoredFile())
	{	//skip whole shader
		SkipBracedSection();
	}
	else
	{
		strcpy(pakMapShaders[bspIndex].oldName, token); //copy shader name
		shaderText[0] = '\0';

		if (!GetTokenAppend(shaderText, qtrue))
			PakMap_Cleanup(qtrue, qtrue, "invalid token in shader");

		if (strcmp(token, "{"))
			PakMap_Cleanup(qtrue, qtrue, va("brace { missing in shader %s", pakMapShaders[bspIndex].oldName));

		while (1)
		{
			if (!GetTokenAppend(shaderText, qtrue))
				break;
			if (!strcmp(token, "}"))
				break;

			depth = 0;
			/* parse stage directives */
			if (!strcmp(token, "{"))
			{
				depth++;
				while (1)
				{
					if (!GetTokenAppend(shaderText, qtrue))
						break;
					if (!strcmp(token, "}"))
						break;
					//skip blend/stage
					PakMap_SetTextureLists_stage(shaderText);
				}
			}
			PakMap_SetTextureLists_global(shaderText);
		}

		//store shader
		pakMapShaders[bspIndex].script = safe_malloc(strlen(shaderText) + 1);
		Q_strncpyz(pakMapShaders[bspIndex].script, shaderText, strlen(shaderText)+1); //copy all shader text
		numShaderScripts++;
		pakMapShaders[bspIndex].unique = 1;
	}

	return qtrue;
}
/*
write .bsp in the original file name
*/
void PakMap_SaveBspFile()
{
	Sys_Printf ("%-22s (maps/%s.bsp)\n", va("Saving .bsp file."), mapName);
	WriteBSPFile(bspFilename);
}

static void PakMap_GetTime(char * timeString)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(timeString, va("%d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday));
}

/*
============
PakMap_CompType

zip compresion types
dont compress already compress files. store
============
*/
int PakMap_CompType(const char *fileName)
{
	char str[256];
	ExtractFileExtension(fileName, str);

	if (!Q_strncasecmp(str, ".png", 4) || !Q_strncasecmp(str, ".jpg", 4))
		return 0;
	//else if (!Q_strncasecmp(str, ".tga", 4))
	
	//default compress
	return Z_DEFLATED;
}

void PakMap_AddFileToPk3(zipFile pk3File, char *texName)
{
	byte *buffer = NULL;
	int i, ret, size, compType;

	//search for file in virtual file system
	size = vfsLoadFile((const char *)texName, (void **)&buffer, 0);
	if (size > 0 )
	{
		const zip_fileinfo zfi = {0};
		compType = PakMap_CompType(texName);

		zipOpenNewFileInZip(pk3File, texName, &zfi, NULL, 0, NULL, 0, NULL, compType, Z_DEFAULT_COMPRESSION);
		ret = zipWriteInFileInZip(pk3File, buffer, size); //-1?
		zipCloseFileInZip(pk3File);
		free(buffer);
	}
	else
	{
		const char *fExt[4] = {".png", ".tga", ".jpg", ".pcx"};
		char tmp[256];

		for (i = 0; i < 4; i++)
		{	//add new file extension
			Q_strncpyz(tmp, texName, 256);
			StripExtension(tmp);
			Q_strcat(tmp,256, fExt[i]);

			//search for file in virtual file system
			size = vfsLoadFile((const char *)tmp, (void **)&buffer, 0);
			if (size > 0 ) //fileAsset != NULL)
			{
				const zip_fileinfo zfi = {0};
				compType = PakMap_CompType(tmp);

				zipOpenNewFileInZip(pk3File, tmp, &zfi, NULL, 0, NULL, 0, NULL, compType, Z_DEFAULT_COMPRESSION);
				ret = zipWriteInFileInZip(pk3File, buffer, size); //-1?
				zipCloseFileInZip(pk3File);
				free(buffer);
				break;
			}
		}

		if (i==4)
			Sys_Printf("%-22s (%s)\n", "Warning: can't read.", texName);
	}

}

static void PakMap_SavePk3File()
{
	FILE *fileAsset = NULL;
	char pakFilename[256], curTime[256];
	char baseDir[256], tmpPath[256];
	int i, count = 0;
	char *ptrBase = strstr(bspFilename, game->gamePath); //todo: check multiple mod names?
	char *bspPath = &bspFilename[0];

	if (ptrBase)
	{
		zipFile pk3File;
		size_t size = ptrBase - bspPath;

		PakMap_GetTime(curTime);
		Q_strncpyz(baseDir, bspFilename, size +strlen(game->gamePath) + 1);
		sprintf(pakFilename, "%s/map-%s-%s.pk3", baseDir, mapName, curTime);
		remove(pakFilename); //delete .pk3 if it exists

		Sys_Printf ("%-22s (map-%s-%s.pk3)\n", "Saving .pk3 file.", mapName, curTime);
		//open new .pk3 file
		pk3File = zipOpen(pakFilename, 0);
		if (pk3File == NULL)
			PakMap_Cleanup(qtrue, qtrue, va("Can't create new pk3 file: %s", pakFilename));

		//loop through all map assets
		for(i = 0; i < numMapTextures; i++)
		{
			//skip rename
			if (textureNames[i] == NULL || textureNames[i][0] =='\0')
				continue;

			count++;
			PakMap_AddFileToPk3(pk3File, textureNames[i]);
		}

		Sys_Printf("Added %i textures.\n", count);

		//add default files that should exist
		PakMap_AddFileToPk3(pk3File, va("maps/%s.bsp", mapName));
		PakMap_AddFileToPk3(pk3File, va("maps/%s.aas", mapName));
		PakMap_AddFileToPk3(pk3File, va("levelshots/%s.jpg", mapName));
		PakMap_AddFileToPk3(pk3File, va("video/%s.ogv", mapName)); //menu play video
		PakMap_AddFileToPk3(pk3File, va( "%s/kmap2_%s.mtr", game->shaderPath, mapName)); //add new shader file. note: mapShaderFile full path fails

		i = 0;
		while (FileExists(va("%s/maps/%s/lm_%04d.tga", baseDir, mapName, i)))
		{
			sprintf(tmpPath, "maps/%s/lm_%04d.tga", mapName, i++);
			PakMap_AddFileToPk3(pk3File, tmpPath);
		}

		if (zipClose(pk3File, "closing pk3"))
			PakMap_Cleanup(qtrue, qtrue, "cant close pk3 file"); 
	}
	else
	{
		//warn, no base
		PakMap_Cleanup(qtrue, qtrue, va("GamePath game not found in filename. <%s> <%s>", game->gamePath, bspFilename)); 
	}
}

/*
========
PakMap_ReadBspFile

rename ".bsp" to ".bsp.pakmap"
then load renamed file
========
*/
void PakMap_ReadBspFile()
{
	//set old/new map file names
	sprintf(bspFilename, "%s.bsp", source);
	sprintf(bspTmpFilename, "%s.bsp.pakmap", source);

	//make sure map exists
	if (source[0] == '\0' || !FileExists(bspFilename))
		PakMap_Cleanup(qfalse, qtrue, va("Can't open bsp file: %s", bspFilename)); 

	//remove any failed attemps
	remove(bspTmpFilename);

	if (rename(bspFilename, bspTmpFilename) != 0)
		PakMap_Cleanup(qfalse, qtrue, va("Can't rename bsp file: %s", bspFilename)); 

	Sys_Printf("Reading bsp: %s\n", bspFilename);
	LoadBSPFile(bspTmpFilename);

	//set mapShaderFile and delete file if it exists
	BeginMapShaderFile(source);

	//read all shaders from shaderlist.txt
	LoadShaderInfo();
}

/*
============
PakMap_SaveShaderFile

clone all used shaders.
All comments removed.

todo: add tables if required
===========
*/
static void PakMap_SaveShaderFile()
{
	FILE           *file;
	int             i;

	/* dummy check */
	if(mapShaderFile[0] == '\0')
		return;
	
	if (!numShaderScripts)
		return;

	/* are there any custom shaders? */
	for(i = 0; i < numBSPShaders; i++)
	{
		if(pakMapShaders[i].unique != 1)
			break;
	}
	if(i == numBSPShaders)
		return;

	/* note it */
	Sys_FPrintf(SYS_VRB, "--- WriteMapShaderFile ---\n");
	Sys_FPrintf(SYS_VRB, "Writing %s", mapShaderFile);

	/* open shader file */
	file = fopen(mapShaderFile, "w");
	if(file == NULL)
	{
		Sys_Printf("WARNING: Unable to open map shader file %s for writing\n", mapShaderFile);
		return;
	}

	/* print header */
	fprintf(file,
			"// Custom shader file for %s.bsp\n"
			"// Generated by KMap2\n" "// This file is overwritten on -pakmap recompiles.\n\n", mapName);

	/* walk the shader list */
	for(i = 0; i < numBSPShaders; i++)
	{
		// skip?
		if(pakMapShaders[i].unique != 1 || pakMapShaders[i].oldName == NULL || pakMapShaders[i].oldName[0] == '\0')
			continue;

		// print it to the file
		fprintf(file, "//%s\n%s%s\n", pakMapShaders[i].oldName, pakMapShaders[i].newName, pakMapShaders[i].script);

		Sys_FPrintf(SYS_VRB, ".");
	}

	//close the shader 
	fflush(file);
	fclose(file);

	Sys_FPrintf(SYS_VRB, "\n");

	/* print some stats */
	Sys_Printf ("=========================\n"
				"%d shaders used.\n", numBSPShaders);	

	Sys_Printf ("%-22s (kmap_%s.mtr)\n", va("%d shaders added.", numShaderScripts) , mapName);
}

static qboolean ParseHeightMap(char *shaderText)
{
//"heightMap"(<map>, <float>)
	GetTokenAppend(shaderText, qfalse);
	if (token[0] != '(')
		return qfalse;

	if (!ReadTexturesAdvancedModes(shaderText)) //<map>
		return qfalse;

	GetTokenAppend(shaderText, qfalse);
	if (token[0] != ',')
		return qfalse;

	GetTokenAppend(shaderText, qfalse); //float value

	GetTokenAppend(shaderText, qfalse); //')'
	if (token[0] != ')')
		return qfalse;

	return qtrue;
}

static qboolean ParseAddNormals(char *shaderText)
{
//"addNormals"(<map>, <map>)
	GetTokenAppend(shaderText, qfalse);
	if (token[0] != '(')
		return qfalse;

	if (!ReadTexturesAdvancedModes(shaderText))
		return qfalse;

	GetTokenAppend(shaderText, qfalse);
	if (token[0] != ',')
		return qfalse;

	if (!ReadTexturesAdvancedModes(shaderText))
		return qfalse;

	GetTokenAppend(shaderText, qfalse);
	if (token[0] != ')')
		return qfalse;

	return qtrue;
}

static qboolean ParseMakeAlpha(char *shaderText)
{
//"smoothNormals"(<map>)
	GetTokenAppend(shaderText, qfalse);
	if (token[0] != '(')
		return qfalse;

	if (!ReadTexturesAdvancedModes(shaderText))
		return qfalse;

	GetTokenAppend(shaderText, qfalse);
	if (token[0] != ')')
		return qfalse;

	return qtrue;
}


/*
============
"heightMap"(<map>, <float>)
"scale"(<map>, <float> [,float] [,float] [,float])

"displaceMap"(<map>, <map>)
"addNormals"(<map>, <map>)
"add"(<map>, <map>)	

"smoothNormals"(<map>)
"invertAlpha"(<map>)
"invertColor"(<map>)
"makeIntensity"(<map>)
"makeAlpha"(<map>)

some textures are combined. parse them out
=========
*/

static qboolean ReadTexturesAdvancedModes(char *shaderText)
{
	GetTokenAppend(shaderText, qfalse);

	//(<map>, <float>)
	if (!Q_stricmp(token, "heightMap"))
	{
		if (!ParseHeightMap(shaderText))
			return qfalse; //error
	}
	else if (!Q_stricmp(token, "scale"))
	{
		//"not supported";
		return qfalse;
	}
	//(<map>, <map>)
	else if (!Q_stricmp(token, "displaceMap") || !Q_stricmp(token, "addNormals") || !Q_stricmp(token, "add"))
	{
		if (!ParseAddNormals(shaderText))
			return qfalse; //error
	}
	//(<map>)
	else if (!Q_stricmp(token, "smoothNormals")|| !Q_stricmp(token, "invertAlpha")|| 
		!Q_stricmp(token, "invertColor")|| !Q_stricmp(token, "makeIntensity")|| !Q_stricmp(token, "makeAlpha"))
	{
		if (!ParseMakeAlpha(shaderText))
			return qfalse; //error
	}
	//add image
	else
	{
		if (!PakMap_IgnoredFile())
		{
			strcpy(textureNames[numMapTextures], token);
			numMapTextures++;
		}
		return qtrue;
	}

	return qtrue; //check this...
}


int PackMapAssets(int argc, char **argv)
{
//dupe bspMain
	int             i;
	char            tempSource[1024];

	Sys_Printf("--- PACKMAP ---\n");
	SetDrawSurfacesBuffer();
	mapDrawSurfs = safe_malloc(sizeof(mapDrawSurface_t) * MAX_MAP_DRAW_SURFS);
	memset(mapDrawSurfs, 0, sizeof(mapDrawSurface_t) * MAX_MAP_DRAW_SURFS);
	numMapDrawSurfs = 0;
	tempSource[0] = '\0';

	/* set standard game flags */
	maxSurfaceVerts = game->maxSurfaceVerts;
	maxLMSurfaceVerts = game->maxLMSurfaceVerts;
	maxSurfaceIndexes = game->maxSurfaceIndexes;
	emitFlares = game->emitFlares;

	/* process arguments */
	for (i = 1; i < (argc - 1); i++)
	{
		//todo: do we need any arguments?
	}

	/* copy source name */
	strcpy(source, ExpandArg(argv[argc - 1])); //get last arg
	StripExtension(source);
	strcpy(name, ExpandArg(argv[argc - 1]));
//end bspMain

//start -pakmap
	isPakMap = 1;
	//check base paths are ok?

	memset(&pakMapShaders, 0, sizeof(shaderStruct) * MAX_SHADER_INFO);

	PakMap_ReadBspFile();
	PakMap_ReNameShader();
	PakMap_SaveShaderFile();
	PakMap_SaveBspFile();
	PakMap_SavePk3File();
	
	PakMap_Cleanup(qtrue, qfalse, NULL);

	return 0;
}

#endif