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
// tr_font.c
//
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old
// fonts since the code is shared with standard Q3A.
//
// If you include this font rendering code in a commercial product you MUST include the
// following somewhere with your product, see www.freetype.org for specifics or changes.
// The Freetype code also uses some hinting techniques that MIGHT infringe on patents
// held by apple so be aware of that also.
//
// As of Q3A 1.25+ and Team Arena, we are shipping the game with the font rendering code
// disabled. This removes any potential patent issues and it keeps us from having to
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish.
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the
// font bitmaps so they scale a bit better in GL.
//
// There are limitations in the way fonts are saved and reloaded in that it is based on
// point size and not name. So if you pre-render Helvetica in 18 point and Impact in 18 point
// you will end up with a single 18 point data file and image set. Typically you will want to
// choose 3 sizes to best approximate the scaling you will be doing in the ui scripting system
//
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We
// rendered three sizes in Team Arena, 12, 16, and 20.
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, smallFont, and bigFont keyword with font name and
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. You will need to flip the tga's in Photoshop as the tga output code writes them upside
//    down.
// 6. In future runs of the game, the system looks for these images and data files when a s
//    specific point sized font is rendered and loads them for use.
// 7. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
//
// Currently a define in the project turns on or off the FreeType code which is currently
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project.


#include "tr_local.h"
#include "../qcommon/qcommon.h"
#if defined (_MSC_VER) || defined (USE_INTERNAL_PNG)
#include "../png/png.h"
#else
#include <png.h>
#endif

#ifdef BUILD_FREETYPE
//#include <ft2build.h>
//#include <freetype/fterrors.h>
#include <freetype/ftsystem.h>
#include <freetype/ftimage.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

#define _FLOOR(x) ((x) & - 64)
#define _CEIL(x) (((x) + 63) & - 64)
#define _TRUNC(x) ((x) >> 6)

FT_Library ftLibrary = NULL;
#endif

#define FONT_SIZE 512
#define MAX_FONTS 16        // 0xA5EA, org 6
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

#ifdef BUILD_FREETYPE
void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch)
{

	*left  = _FLOOR(glyph->metrics.horiBearingX);
	*right = _CEIL(glyph->metrics.horiBearingX + glyph->metrics.width);
	*width = _TRUNC(*right - *left);

	*top    = _CEIL(glyph->metrics.horiBearingY);
	*bottom = _FLOOR(glyph->metrics.horiBearingY - glyph->metrics.height);
	*height = _TRUNC(*top - *bottom);
	*pitch  = (qtrue ? (*width + 3) & - 4 : (*width + 7) >> 3);
}


FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t *glyphOut)
{

	FT_Bitmap *bit2;
	int left, right, width, top, bottom, height, pitch, size;

	R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

	if(glyph->format == ft_glyph_format_outline)
	{
		size = pitch * height;

		bit2 = (FT_Bitmap*)Z_Malloc(sizeof(FT_Bitmap));

		bit2->width      = width;
		bit2->rows       = height;
		bit2->pitch      = pitch;
		bit2->pixel_mode = ft_pixel_mode_grays;
		//bit2->pixel_mode = ft_pixel_mode_mono;
		bit2->buffer    = (unsigned char*)Z_Malloc(pitch * height);
		bit2->num_grays = 256;

		Com_Memset(bit2->buffer, 0, size);

		FT_Outline_Translate(&glyph->outline, -left, -bottom);

		FT_Outline_Get_Bitmap(ftLibrary, &glyph->outline, bit2);

		glyphOut->height = height;
		glyphOut->pitch  = pitch;
		glyphOut->top    = (glyph->metrics.horiBearingY >> 6) + 1;
		glyphOut->bottom = bottom;

		return bit2;
	}
	else
	{
		ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
	}
	return NULL;
}

void WriteTGA(char *filename, byte *data, int width, int height)
{
	byte *buffer;
	int i, c;

	buffer = (byte*)Z_Malloc(width * height * 4 + 18);
	Com_Memset(buffer, 0, 18);
	buffer[2]  = 2;                             // uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 32;                            // pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for(i = 18; i < c; i += 4)
	{
		buffer[i]     = data[i - 18 + 2];       // blue
		buffer[i + 1] = data[i - 18 + 1];       // green
		buffer[i + 2] = data[i - 18 + 0];       // red
		buffer[i + 3] = data[i - 18 + 3];       // alpha
	}

	ri.FS_WriteFile(filename, buffer, c);

	//f = fopen (filename, "wb");
	//fwrite (buffer, 1, c, f);
	//fclose (f);

	Z_Free(buffer);
}
/*
=========================================================

PNG SAVING

=========================================================
*/
static int png_compressed_size;

static void png_write_data(png_structp png, png_bytep data, png_size_t length)
{
	Com_Memcpy(png->io_ptr, data, length);

	png->io_ptr = (byte*)png->io_ptr + length;

	png_compressed_size += length;
}
static void png_flush_data(png_structp png)
{
}
void WritePNG(const char *name, const byte *pic, int width, int height)
{
	png_structp png;
	png_infop info;
	int i;
	int row_stride;
	byte *buffer;
	byte *row;
	png_bytep *row_pointers;

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png)
		return;

	// Allocate/initialize the image information data
	info = png_create_info_struct(png);
	if(!info)
	{
		png_destroy_write_struct(&png, (png_infopp)NULL);
		return;
	}

	png_compressed_size = 0;
	buffer              = (byte*)ri.Hunk_AllocateTempMemory(width * height * 3);

	// set error handling
	if(setjmp(png_jmpbuf(png)))
	{
		ri.Hunk_FreeTempMemory(buffer);
		png_destroy_write_struct(&png, &info);
		return;
	}

	png_set_write_fn(png, buffer, png_write_data, png_flush_data);
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);

	// write the file header information
	png_write_info(png, info);

	row_pointers = (png_bytep*)ri.Hunk_AllocateTempMemory(height * sizeof(png_bytep));

	if(setjmp(png_jmpbuf(png)))
	{
		ri.Hunk_FreeTempMemory(row_pointers);
		ri.Hunk_FreeTempMemory(buffer);
		png_destroy_write_struct(&png, &info);
		return;
	}

	row_stride = width * 4;
	row        = (byte*)(pic + (height - 1) * row_stride);

#if 0
	for(i = 0; i < height; i++)
	{
		row_pointers[i] = row;
		row            -= row_stride;
	}
#else
	for(i = height - 1; i >= 0; i--)
	{
		row_pointers[i] = row;
		row            -= row_stride;
	}
#endif

	png_write_image(png, row_pointers);
	png_write_end(png, info);

	// clean up after the write, and free any memory allocated
	png_destroy_write_struct(&png, &info);

	ri.Hunk_FreeTempMemory(row_pointers);

	ri.FS_WriteFile(name, buffer, png_compressed_size);

	ri.Hunk_FreeTempMemory(buffer);
}

static glyphInfo_t *RE_ConstructGlyphInfo(unsigned char *imageOut, int *xOut, int *yOut, int *maxHeight, FT_Face face, const unsigned char c, qboolean calcHeight)
{
	int i;
	static glyphInfo_t glyph;
	unsigned char *src, *dst;
	float scaled_width, scaled_height;
	FT_Bitmap *bitmap = NULL;

	Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
	// make sure everything is here
	if(face != NULL)
	{
		FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_DEFAULT);
		bitmap = R_RenderGlyph(face->glyph, &glyph);
		if(bitmap)
		{
			glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
		}
		else
		{
			return &glyph;
		}

		if(glyph.height > *maxHeight)
		{
			*maxHeight = glyph.height;
		}

		if(calcHeight)
		{
			Z_Free(bitmap->buffer);
			Z_Free(bitmap);
			return &glyph;
		}

/*
    // need to convert to power of 2 sizes so we do not get
    // any scaling from the gl upload
    for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)
        ;
    for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)
        ;
*/

		scaled_width  = glyph.pitch;
		scaled_height = glyph.height;

		// we need to make sure we fit
		if(*xOut + scaled_width + 1 >= FONT_SIZE - 1)
		{
			if(*yOut + *maxHeight + 1 >= FONT_SIZE - 1)
			{
				*yOut = -1;
				*xOut = -1;
				Z_Free(bitmap->buffer);
				Z_Free(bitmap);
				return &glyph;
			}
			else
			{
				*xOut  = 0;
				*yOut += *maxHeight + 1;
			}
		}
		else if(*yOut + *maxHeight + 1 >= FONT_SIZE - 1)
		{
			*yOut = -1;
			*xOut = -1;
			Z_Free(bitmap->buffer);
			Z_Free(bitmap);
			return &glyph;
		}


		src = bitmap->buffer;
		dst = imageOut + (*yOut * FONT_SIZE) + *xOut;

		if(bitmap->pixel_mode == ft_pixel_mode_mono)
		{
			for(i = 0; i < glyph.height; i++)
			{
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask  = 0x80;
				unsigned char val   = *_src;
				for(j = 0; j < glyph.pitch; j++)
				{
					if(mask == 0x80)
					{
						val = *_src++;
					}
					if(val & mask)
					{
						*_dst = 0xff;
					}
					mask >>= 1;

					if(mask == 0)
					{
						mask = 0x80;
					}
					_dst++;
				}

				src += glyph.pitch;
				dst += FONT_SIZE;

			}
		}
		else
		{
			for(i = 0; i < glyph.height; i++)
			{
				Com_Memcpy(dst, src, glyph.pitch);
				src += glyph.pitch;
				dst += FONT_SIZE;
			}
		}

		// we now have an 8 bit per pixel grey scale bitmap
		// that is width wide and pf->ftSize->metrics.y_ppem tall

		glyph.imageHeight = scaled_height;
		glyph.imageWidth  = scaled_width;
		glyph.s           = (float)*xOut / FONT_SIZE;
		glyph.t           = (float)*yOut / FONT_SIZE;
		glyph.s2          = glyph.s + (float)scaled_width / FONT_SIZE;
		glyph.t2          = glyph.t + (float)scaled_height / FONT_SIZE;

		*xOut += scaled_width + 1;
	}

	Z_Free(bitmap->buffer);
	Z_Free(bitmap);

	return &glyph;
}
#endif

static int fdOffset;
static byte *fdFile;

int readInt(void)
{
	int i = fdFile[fdOffset] + (fdFile[fdOffset + 1] << 8) + (fdFile[fdOffset + 2] << 16) + (fdFile[fdOffset + 3] << 24);

	fdOffset += 4;
	return i;
}

typedef union
{
	byte fred[4];
	float ffred;
} poor;

float readFloat(void)
{
	poor me;

#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset + 3];
	me.fred[1] = fdFile[fdOffset + 2];
	me.fred[2] = fdFile[fdOffset + 1];
	me.fred[3] = fdFile[fdOffset + 0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset + 0];
	me.fred[1] = fdFile[fdOffset + 1];
	me.fred[2] = fdFile[fdOffset + 2];
	me.fred[3] = fdFile[fdOffset + 3];
#endif
	fdOffset += 4;
	return me.ffred;
}

void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font)
{
#ifdef BUILD_FREETYPE
	FT_Face face;
	int j, k, xOut, yOut, lastStart, imageNumber;
	int scaledSize, newSize, maxHeight, left, satLevels;
	unsigned char  *out, *imageBuff;
	glyphInfo_t *glyph;
	image_t *image;
	qhandle_t h;
	float max;
#endif
	void *faceData;
	int i, len;
	char            fileName[MAX_QPATH];
	char strippedName[MAX_QPATH];
	float dpi        = 72;                                          //
	float glyphScale =  72.0f / dpi;                                // change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )


	if(!fontName)
	{
		ri.Printf(PRINT_ALL, "RE_RegisterFont: called with empty name\n");
		return;
	}
  ri.Printf(PRINT_ALL, "RE_RegisterFont: fontNames %s \n", fontName);
	if(pointSize <= 0)
	{
		pointSize = 12;
	}
	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	// make sure the render thread is stopped
	R_SyncRenderThread();

	if(registeredFontCount >= MAX_FONTS)
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: Too many fonts registered already.\n");
		return;
	}

#if defined(COMPAT_ET)
	Com_sprintf(fileName, sizeof(fileName), "fonts/%s_%i.dat", fontName, pointSize);
#else
	COM_StripExtension(fontName, strippedName, sizeof(strippedName));
	Com_sprintf(fileName, sizeof(fileName), "%s_%i.dat", strippedName, pointSize);
#endif
	for(i = 0; i < registeredFontCount; i++)
	{
		if (Q_stricmp(fileName, registeredFont[i].name) == 0)
		{
			Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
			return;
		}
	}

#if defined(COMPAT_ET) // DON'T DO THIS WITH VANILLA XREAL
	len = ri.FS_ReadFile(fileName, NULL);
	if(len == sizeof(fontInfo_t))
	{
		ri.FS_ReadFile(fileName, &faceData);
		fdOffset = 0;
		fdFile = static_cast<byte*>(faceData);
		for(i = 0; i < GLYPHS_PER_FONT; i++)
		{
			font->glyphs[i].height      = readInt();
			font->glyphs[i].top         = readInt();
			font->glyphs[i].bottom      = readInt();
			font->glyphs[i].pitch       = readInt();
			font->glyphs[i].xSkip       = readInt();
			font->glyphs[i].imageWidth  = readInt();
			font->glyphs[i].imageHeight = readInt();
			font->glyphs[i].s           = readFloat();
			font->glyphs[i].t           = readFloat();
			font->glyphs[i].s2          = readFloat();
			font->glyphs[i].t2          = readFloat();
			font->glyphs[i].glyph       = readInt();
			Com_Memcpy(font->glyphs[i].shaderName, &fdFile[fdOffset], 32);
			fdOffset += 32;
		}
		font->glyphScale = readFloat();
		Com_Memcpy(font->name, &fdFile[fdOffset], MAX_QPATH);

		//		Com_Memcpy(font, faceData, sizeof(fontInfo_t));
		Q_strncpyz(font->name, fileName, sizeof(font->name));
		for(i = GLYPH_START; i < GLYPH_END; i++)
		{
			font->glyphs[i].glyph = RE_RegisterShaderNoMip(font->glyphs[i].shaderName);
		}
		Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));
		return;
	}
#endif

#ifndef BUILD_FREETYPE
	ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType code not available\n");
#else
	if(ftLibrary == NULL)
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType not initialized.\n");
		return;
	}

	Com_sprintf(fileName, sizeof(fileName), "%s.ttf", strippedName);
	len = ri.FS_ReadFile(fileName, &faceData);
	if(len <= 0)
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: Unable to read font file '%s'\n", fileName);
		return;
	}

	// allocate on the stack first in case we fail
	if(FT_New_Memory_Face(ftLibrary, (FT_Byte*)faceData, len, 0, &face))
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType2, unable to allocate new face.\n");
		return;
	}


	if(FT_Set_Char_Size(face, pointSize << 6, pointSize << 6, dpi, dpi))
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType2, Unable to set face char size.\n");
		return;
	}


	// make a 256x256 image buffer, once it is full, register it, clean it and keep going
	// until all glyphs are rendered

	out = (unsigned char*)Z_Malloc(1024 * 1024);
	if(out == NULL)
	{
		ri.Printf(PRINT_ALL, "RE_RegisterFont: Z_Malloc failure during output image creation.\n");
		return;
	}
	Com_Memset(out, 0, 1024 * 1024);

	// calculate max height
	maxHeight = 0;

	for(i = GLYPH_START; i < GLYPH_END; i++)
	{
		glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qtrue);
	}
	//ri.Printf(PRINT_WARNING, "RE_RegisterFont: max glyph height for font %s is %i\n", strippedName, maxHeight);

	xOut        = 0;
	yOut        = 0;
	i           = GLYPH_START;
	lastStart   = i;
	imageNumber = 0;

	while(i <= GLYPH_END)
	{

		glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qfalse);

		if(xOut == -1 || yOut == -1 || i == GLYPH_END)
		{
			if(xOut == -1 || yOut == -1)
			{
				//ri.Printf(PRINT_WARNING, "RE_RegisterFont: character %c does not fit image number %i\n", (unsigned char) i, imageNumber);
			}
			// ran out of room
			// we need to create an image from the bitmap, set all the handles in the glyphs to this point

			scaledSize = FONT_SIZE * FONT_SIZE;
			newSize    = scaledSize * 4;
			imageBuff  = (unsigned char*)Z_Malloc(newSize);
			left       = 0;
			max        = 0;
			satLevels  = 255;
			for(k = 0; k < (scaledSize); k++)
			{
				if(max < out[k])
				{
					max = out[k];
				}
			}

			if(max > 0)
			{
				max = 255 / max;
			}

			for(k = 0; k < (scaledSize); k++)
			{
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;

				imageBuff[left++] = ((float)out[k] * max);
			}

			Com_sprintf(fileName, sizeof(fileName), "%s_%i_%i.png", strippedName, imageNumber++, pointSize);
			if(!ri.FS_FileExists(fileName))
			{
				R_SavePNG(fileName, imageBuff, FONT_SIZE, FONT_SIZE, 4, qtrue);
			}

			image = R_CreateImage(fileName, imageBuff, FONT_SIZE, FONT_SIZE, IF_NOPICMIP, FT_LINEAR, WT_CLAMP);
			h = RE_RegisterShaderFromImage(fileName, image, qfalse);


			Com_Memset(out, 0, 1024 * 1024);
			xOut = 0;
			yOut = 0;
			ri.Free(imageBuff);

			if(i == GLYPH_END)
			{
				for(j = lastStart; j <= GLYPH_END; j++)
				{
					font->glyphs[j].glyph = h;
					Q_strncpyz(font->glyphs[j].shaderName, fileName, sizeof(font->glyphs[j].shaderName));
				}
				break;
			}
			else
			{
				for(j = lastStart; j < i; j++)
				{
					font->glyphs[j].glyph = h;
					Q_strncpyz(font->glyphs[j].shaderName, fileName, sizeof(font->glyphs[j].shaderName));
				}
				lastStart = i;
			}
		}
		else
		{
			Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
			i++;
		}
	}

	registeredFont[registeredFontCount].glyphScale = glyphScale;
	font->glyphScale                               = glyphScale;
	Com_sprintf(fileName, sizeof(fileName), "%s_%i.dat", strippedName, pointSize);
	Q_strncpyz(font->name, fileName, sizeof(font->name));
	if(!ri.FS_FileExists(fileName))
	{
		ri.FS_WriteFile(fileName, font, sizeof(fontInfo_t));
	}
	Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));


	Z_Free(out);

	ri.FS_FreeFile(faceData);
#endif
}



void R_InitFreeType(void)
{
#ifdef BUILD_FREETYPE
	if(FT_Init_FreeType(&ftLibrary))
	{
		ri.Printf(PRINT_ALL, "R_InitFreeType: Unable to initialize FreeType.\n");
	}
#endif
	registeredFontCount = 0;
}


void R_DoneFreeType(void)
{
#ifdef BUILD_FREETYPE
	if(ftLibrary)
	{
		FT_Done_FreeType(ftLibrary);
		ftLibrary = NULL;
	}
#endif
	registeredFontCount = 0;
}
