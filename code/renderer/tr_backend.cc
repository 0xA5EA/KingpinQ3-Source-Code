/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of Daemon source code.

Daemon source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Daemon source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_backend.c

#include "tr_local.h"
#include "tr_glshader.hh"


backEndData_t  *backEndData[ SMP_FRAMES ];
backEndState_t backEnd;

void GL_Bind( image_t *image )
{
  int texnum;

  if ( !image )
  {
    ri.Printf( PRINT_WARNING, "GL_Bind: NULL image\n" );
    image = tr.defaultImage;
  }
  else
  {
    if ( r_logFile->integer )
    {
      // don't just call LogComment, or we will get a call to va() every frame!
      GLimp_LogComment( va( "--- GL_Bind( %s ) ---\n", image->name ) );
    }
  }

  texnum = image->texnum;

  if ( r_nobind->integer && tr.blackImage )
  {
    // performance evaluation option
    texnum = tr.blackImage->texnum;
  }

  if ( glState.currenttextures[ glState.currenttmu ] != texnum )
  {
    image->frameUsed = tr.frameCount;
    glState.currenttextures[ glState.currenttmu ] = texnum;
    glBindTexture( image->type, texnum );
  }
}

void GL_Unbind( image_t *image )
{
  GLimp_LogComment( "--- GL_Unbind() ---\n" );

  glState.currenttextures[ glState.currenttmu ] = 0;
  glBindTexture( image->type, 0 );
}

void BindAnimatedImage( textureBundle_t *bundle )
{
  int index;

  if ( bundle->isVideoMap )
  {
    ri.CIN_RunCinematic( bundle->videoMapHandle );
    ri.CIN_UploadCinematic( bundle->videoMapHandle );
    return;
  }

  if ( bundle->numImages <= 1 )
  {
    GL_Bind( bundle->image[ 0 ] );
    return;
  }

  // it is necessary to do this messy calc to make sure animations line up
  // exactly with waveforms of the same frequency
  index = Q_ftol( backEnd.refdef.floatTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE );
  index >>= FUNCTABLE_SIZE2;

  if ( index < 0 ) 
  {
    index = 0; // may happen with shader time offsets
  }
#ifdef HYPODEBUG
  if (index >= bundle->numImages)
    Com_DPrintf("image index: %i  ret: %i\n", index, index % bundle->numImages);
#endif

  index %= bundle->numImages;

  GL_Bind( bundle->image[ index ] );
}


void GL_BindProgram( shaderProgram_t *program )
{
  if ( !program )
  {
    GL_BindNullProgram();
    return;
  }

  if ( glState.currentProgram != program )
  {
    glUseProgram( program->program );
    glState.currentProgram = program;
  }
}

void GL_BindNullProgram( void )
{
  if ( r_logFile->integer )
  {
    GLimp_LogComment( "--- GL_BindNullProgram ---\n" );
  }

  if ( glState.currentProgram )
  {
    glUseProgram( 0 );
    glState.currentProgram = NULL;
  }
}

void GL_SelectTexture( int unit )
{
  if ( glState.currenttmu == unit )
  {
    return;
  }

  if ( unit >= 0 && unit <= 31 )
  {
    glActiveTexture( GL_TEXTURE0 + unit );

    if ( r_logFile->integer )
    {
      GLimp_LogComment( va( "glActiveTexture( GL_TEXTURE%i )\n", unit ) );
    }
  }
  else
  {
    ri.Error( ERR_DROP, "GL_SelectTexture: unit = %i", unit );
  }

  glState.currenttmu = unit;
}

void GL_BindToTMU( int unit, image_t *image )
{
  int texnum = image->texnum;

  if ( unit < 0 || unit > 31 )
  {
    ri.Error( ERR_DROP, "GL_BindToTMU: unit %i is out of range\n", unit );
  }

  if ( glState.currenttextures[ unit ] == texnum )
  {
    return;
  }

  GL_SelectTexture( unit );
  GL_Bind( image );
}

void GL_BlendFunc( GLenum sfactor, GLenum dfactor )
{
  if ( glState.blendSrc != ( signed ) sfactor || glState.blendDst != ( signed ) dfactor )
  {
    glState.blendSrc = sfactor;
    glState.blendDst = dfactor;

    glBlendFunc( sfactor, dfactor );
  }
}

void GL_ClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
  if ( glState.clearColorRed != red || glState.clearColorGreen != green || glState.clearColorBlue != blue || glState.clearColorAlpha != alpha )
  {
    glState.clearColorRed = red;
    glState.clearColorGreen = green;
    glState.clearColorBlue = blue;
    glState.clearColorAlpha = alpha;

    glClearColor( red, green, blue, alpha );
  }
}

void GL_ClearDepth( GLclampd depth )
{
  if ( glState.clearDepth != depth )
  {
    glState.clearDepth = depth;

    glClearDepth( depth );
  }
}

void GL_ClearStencil( GLint s )
{
  if ( glState.clearStencil != s )
  {
    glState.clearStencil = s;

    glClearStencil( s );
  }
}

void GL_ColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
  if ( glState.colorMaskRed != red || glState.colorMaskGreen != green || glState.colorMaskBlue != blue || glState.colorMaskAlpha != alpha )
  {
    glState.colorMaskRed = red;
    glState.colorMaskGreen = green;
    glState.colorMaskBlue = blue;
    glState.colorMaskAlpha = alpha;

    glColorMask( red, green, blue, alpha );
  }
}

void GL_CullFace( GLenum mode )
{
  if ( glState.cullFace != ( signed ) mode )
  {
    glState.cullFace = mode;

    glCullFace( mode );
  }
}

void GL_DepthFunc( GLenum func )
{
  if ( glState.depthFunc != ( signed ) func )
  {
    glState.depthFunc = func;

    glDepthFunc( func );
  }
}

void GL_DepthMask( GLboolean flag )
{
  if ( glState.depthMask != flag )
  {
    glState.depthMask = flag;

    glDepthMask( flag );
  }
}

void GL_DrawBuffer( GLenum mode )
{
  if ( glState.drawBuffer != ( signed ) mode )
  {
    glState.drawBuffer = mode;

    glDrawBuffer( mode );
  }
}

void GL_FrontFace( GLenum mode )
{
  if ( glState.frontFace != ( signed ) mode )
  {
    glState.frontFace = mode;

    glFrontFace( mode );
  }
}

void GL_LoadModelViewMatrix( const matrix_t m )
{
#if 1

  if (  MatrixCompare( glState.modelViewMatrix[ glState.stackIndex ], m ) )
  {
    return;
  }

#endif

  MatrixCopy( m, glState.modelViewMatrix[ glState.stackIndex ] );
  MatrixMultiply( glState.projectionMatrix[ glState.stackIndex ], glState.modelViewMatrix[ glState.stackIndex ],
                  glState.modelViewProjectionMatrix[ glState.stackIndex ] );
}

void GL_LoadProjectionMatrix( const matrix_t m )
{
#if 1

  if ( MatrixCompare( glState.projectionMatrix[ glState.stackIndex ], m ) )
  {
    return;
  }

#endif

  MatrixCopy( m, glState.projectionMatrix[ glState.stackIndex ] );
  MatrixMultiply( glState.projectionMatrix[ glState.stackIndex ], glState.modelViewMatrix[ glState.stackIndex ],
                  glState.modelViewProjectionMatrix[ glState.stackIndex ] );
}

void GL_PushMatrix()
{
  glState.stackIndex++;

  if ( glState.stackIndex >= MAX_GLSTACK )
  {
    glState.stackIndex = MAX_GLSTACK - 1;
    ri.Error( ERR_DROP, "GL_PushMatrix: stack overflow = %i", glState.stackIndex );
  }
}

void GL_PopMatrix()
{
  glState.stackIndex--;

  if ( glState.stackIndex < 0 )
  {
    glState.stackIndex = 0;
    ri.Error( ERR_DROP, "GL_PushMatrix: stack underflow" );
  }
}

void GL_PolygonMode( GLenum face, GLenum mode )
{
  if ( glState.polygonFace != ( signed ) face || glState.polygonMode != ( signed ) mode )
  {
    glState.polygonFace = face;
    glState.polygonMode = mode;

    glPolygonMode( face, mode );
  }
}

void GL_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
  if ( glState.scissorX != x || glState.scissorY != y || glState.scissorWidth != width || glState.scissorHeight != height )
  {
    glState.scissorX = x;
    glState.scissorY = y;
    glState.scissorWidth = width;
    glState.scissorHeight = height;

    glScissor( x, y, width, height );
  }
}

void GL_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
  if ( glState.viewportX != x || glState.viewportY != y || glState.viewportWidth != width || glState.viewportHeight != height )
  {
    glState.viewportX = x;
    glState.viewportY = y;
    glState.viewportWidth = width;
    glState.viewportHeight = height;

    glViewport( x, y, width, height );
  }
}

void GL_PolygonOffset( float factor, float units )
{
  if ( glState.polygonOffsetFactor != factor || glState.polygonOffsetUnits != units )
  {
    glState.polygonOffsetFactor = factor;
    glState.polygonOffsetUnits = units;

    glPolygonOffset( factor, units );
  }
}

void GL_Cull( int cullType )
{
  if ( backEnd.viewParms.isMirror )
  {
    GL_FrontFace( GL_CW );
  }
  else
  {
    GL_FrontFace( GL_CCW );
  }

  if ( glState.faceCulling == cullType )
  {
    return;
  }

  if ( cullType == CT_TWO_SIDED )
  {
    glDisable( GL_CULL_FACE );
  }
  else
  {
    if( glState.faceCulling == CT_TWO_SIDED )
      glEnable( GL_CULL_FACE );

    if ( cullType == CT_BACK_SIDED )
    {
      GL_CullFace( GL_BACK );
    }
    else
    {
      GL_CullFace( GL_FRONT );
    }
  }
  glState.faceCulling = cullType;
}

/*
GL_State

This routine is responsible for setting the most commonly changed state
in Q3.
*/
void GL_State( uint32_t stateBits )
{
  uint32_t diff = stateBits ^ glState.glStateBits;
  //diff &= ~glState.glStateBitsMask; //daemon .50

  if ( !diff )
  {
    return;
  }

  // check depthFunc bits
  if ( diff & GLS_DEPTHFUNC_BITS )
  {
    switch ( stateBits & GLS_DEPTHFUNC_BITS )
    {
      default:
        GL_DepthFunc( GL_LEQUAL );
        break;

      case GLS_DEPTHFUNC_LESS:
        GL_DepthFunc( GL_LESS );
        break;

      case GLS_DEPTHFUNC_EQUAL:
        GL_DepthFunc( GL_EQUAL );
        break;
    }
  }

  // check blend bits
  if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
  {
    GLenum srcFactor, dstFactor;

    if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
    {
      switch ( stateBits & GLS_SRCBLEND_BITS )
      {
        case GLS_SRCBLEND_ZERO:
          srcFactor = GL_ZERO;
          break;

        case GLS_SRCBLEND_ONE:
          srcFactor = GL_ONE;
          break;

        case GLS_SRCBLEND_DST_COLOR:
          srcFactor = GL_DST_COLOR;
          break;

        case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
          srcFactor = GL_ONE_MINUS_DST_COLOR;
          break;

        case GLS_SRCBLEND_SRC_ALPHA:
          srcFactor = GL_SRC_ALPHA;
          break;

        case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
          srcFactor = GL_ONE_MINUS_SRC_ALPHA;
          break;

        case GLS_SRCBLEND_DST_ALPHA:
          srcFactor = GL_DST_ALPHA;
          break;

        case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
          srcFactor = GL_ONE_MINUS_DST_ALPHA;
          break;

        case GLS_SRCBLEND_ALPHA_SATURATE:
          srcFactor = GL_SRC_ALPHA_SATURATE;
          break;

        default:
          srcFactor = GL_ONE; // to get warning to shut up
          ri.Error( ERR_DROP, "GL_State: invalid src blend state bits" );
      }

      switch ( stateBits & GLS_DSTBLEND_BITS )
      {
        case GLS_DSTBLEND_ZERO:
          dstFactor = GL_ZERO;
          break;

        case GLS_DSTBLEND_ONE:
          dstFactor = GL_ONE;
          break;

        case GLS_DSTBLEND_SRC_COLOR:
          dstFactor = GL_SRC_COLOR;
          break;

        case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
          dstFactor = GL_ONE_MINUS_SRC_COLOR;
          break;

        case GLS_DSTBLEND_SRC_ALPHA:
          dstFactor = GL_SRC_ALPHA;
          break;

        case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
          dstFactor = GL_ONE_MINUS_SRC_ALPHA;
          break;

        case GLS_DSTBLEND_DST_ALPHA:
          dstFactor = GL_DST_ALPHA;
          break;

        case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
          dstFactor = GL_ONE_MINUS_DST_ALPHA;
          break;

        default:
          dstFactor = GL_ONE; // to get warning to shut up
          ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits" );
      }

      glEnable( GL_BLEND );
      GL_BlendFunc( srcFactor, dstFactor );
    }
    else
    {
      glDisable( GL_BLEND );
    }
  }

  // check colormask
  if ( diff & GLS_COLORMASK_BITS )
  {
    GL_ColorMask( ( stateBits & GLS_REDMASK_FALSE ) ? GL_FALSE : GL_TRUE,
            ( stateBits & GLS_GREENMASK_FALSE ) ? GL_FALSE : GL_TRUE,
            ( stateBits & GLS_BLUEMASK_FALSE ) ? GL_FALSE : GL_TRUE,
            ( stateBits & GLS_ALPHAMASK_FALSE ) ? GL_FALSE : GL_TRUE );
  }

  // check depthmask
  if ( diff & GLS_DEPTHMASK_TRUE )
  {
    if ( stateBits & GLS_DEPTHMASK_TRUE )
    {
      GL_DepthMask( GL_TRUE );
    }
    else
    {
      GL_DepthMask( GL_FALSE );
    }
  }

  // fill/line mode
  if ( diff & GLS_POLYMODE_LINE )
  {
    if ( stateBits & GLS_POLYMODE_LINE )
    {
      GL_PolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
      GL_PolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
  }

  // depthtest
  if ( diff & GLS_DEPTHTEST_DISABLE )
  {
    if ( stateBits & GLS_DEPTHTEST_DISABLE )
    {
      glDisable( GL_DEPTH_TEST );
    }
    else
    {
      glEnable( GL_DEPTH_TEST );
    }
  }

  // stenciltest
  if ( diff & GLS_STENCILTEST_ENABLE )
  {
    if ( stateBits & GLS_STENCILTEST_ENABLE )
    {
      glEnable( GL_STENCIL_TEST );
    }
    else
    {
      glDisable( GL_STENCIL_TEST );
    }
  }

  glState.glStateBits = stateBits;
}

void GL_VertexAttribsState( uint32_t stateBits )
{
  uint32_t diff;
  uint32_t i;

  if ( glConfig2.vboVertexSkinningAvailable && tess.vboVertexSkinning ) //hypov8 bones
  {
    stateBits |= ( ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS );
  }

  GL_VertexAttribPointers( stateBits );

  diff = stateBits ^ glState.vertexAttribsState;

  if ( !diff )
  {
    return;
  }

  for ( i = 0; i < ATTR_INDEX_MAX; i++ )
  {
    uint32_t bit = Q_BIT( i );

    if ( ( diff & bit ) )
    {
      if ( ( stateBits & bit ) )
      {
        if ( r_logFile->integer )
        {
          static char buf[ MAX_STRING_CHARS ];
          Q_snprintf( buf, sizeof( buf ), "glEnableVertexAttribArray( %s )\n", attributeNames[ i ] );

          GLimp_LogComment( buf );
        }

        glEnableVertexAttribArray( i );
      }
      else
      {
        if ( r_logFile->integer )
        {
          static char buf[ MAX_STRING_CHARS ];
          Q_snprintf( buf, sizeof( buf ), "glDisableVertexAttribArray( %s )\n", attributeNames[ i ] );

          GLimp_LogComment( buf );
        }

        glDisableVertexAttribArray( i );
      }
    }
  }

  glState.vertexAttribsState = stateBits;
}

void GL_VertexAttribPointers( uint32_t attribBits )
{
  uint32_t i;

  if ( !glState.currentVBO )
  {
    ri.Error( ERR_FATAL, "GL_VertexAttribPointers: no VBO bound" );
  }

  if ( r_logFile->integer )
  {
    // don't just call LogComment, or we will get a call to va() every frame!
    GLimp_LogComment( va( "--- GL_VertexAttribPointers( %s ) ---\n", glState.currentVBO->name ) );
  }

  if ( glConfig2.vboVertexSkinningAvailable && tess.vboVertexSkinning )
  {
    attribBits |= ( ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS );
  }

  for ( i = 0; i < ATTR_INDEX_MAX; i++ )
  {
    uint32_t bit = Q_BIT( i );
    uint32_t frame = 0;

    if ( ( attribBits & bit ) && ( !( glState.vertexAttribPointersSet & bit ) || glState.vertexAttribsInterpolation >= 0 ) )
    {
      const vboAttributeLayout_t *layout = &glState.currentVBO->attribs[ i ];

      if ( r_logFile->integer )
      {
        static char buf[ MAX_STRING_CHARS ];
        Q_snprintf( buf, sizeof( buf ), "glVertexAttribPointer( %s )\n", attributeNames[ i ] );

        GLimp_LogComment( buf );
      }

      if ( ( ATTR_INTERP_BITS & bit ) && glState.vertexAttribsInterpolation > 0 )
      {
        frame = glState.vertexAttribsNewFrame;
      }
      else
      {
        frame = glState.vertexAttribsOldFrame;
      }

      glVertexAttribPointer( i, layout->numComponents, layout->componentType, layout->normalize, layout->stride, BUFFER_OFFSET( layout->ofs + ( frame * layout->frameOffset ) ) );
      glState.vertexAttribPointersSet |= bit;
    }
  }
}

/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void )
{
  float c;

  if ( !backEnd.isHyperspace )
  {
    // do initialization shit
  }

  c = ( backEnd.refdef.time & 255 ) / 255.0f;
  GL_ClearColor( c, c, c, 1 );
  glClear( GL_COLOR_BUFFER_BIT );

  backEnd.isHyperspace = qtrue;
}

static void SetViewportAndScissor( void )
{
  float	mat[16], scale;
  vec4_t	q, c;

  Com_Memcpy( mat, backEnd.viewParms.projectionMatrix, sizeof(mat) );
  if( backEnd.viewParms.isPortal ) {
    c[0] = -DotProduct( backEnd.viewParms.portalPlane.normal, backEnd.viewParms.orientation.axis[1] );
    c[1] = DotProduct(backEnd.viewParms.portalPlane.normal, backEnd.viewParms.orientation.axis[2]);
    c[2] = -DotProduct(backEnd.viewParms.portalPlane.normal, backEnd.viewParms.orientation.axis[0]);
    c[3] = DotProduct(backEnd.viewParms.portalPlane.normal, backEnd.viewParms.orientation.origin) - backEnd.viewParms.portalPlane.dist;

    q[0] = (c[0] < 0.0f ? -1.0f : 1.0f) / mat[0];
    q[1] = (c[1] < 0.0f ? -1.0f : 1.0f) / mat[5];
    q[2] = -1.0f;
    q[3] = (1.0f + mat[10]) / mat[14];

    scale = 2.0f / (DotProduct( c, q ) + c[3] * q[3]);
    mat[2]  = c[0] * scale;
    mat[6]  = c[1] * scale;
    mat[10] = c[2] * scale + 1.0f;
    mat[14] = c[3] * scale;
  }
  GL_LoadProjectionMatrix( mat );

  // set the window clipping
  GL_Viewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
               backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
              backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
}

/*
================
RB_SetGL2D
================
*/
static void RB_SetGL2D( void )
{
  matrix_t proj;

  GLimp_LogComment( "--- RB_SetGL2D ---\n" );

  // disable offscreen rendering
  if ( glConfig2.framebufferObjectAvailable )
  {
    R_BindNullFBO();
  }

  backEnd.projection2D = qtrue;

  // set 2D virtual screen size
  GL_Viewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
  GL_Scissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );

  MatrixOrthogonalProjection( proj, 0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1 );
  GL_LoadProjectionMatrix( proj );
  GL_LoadModelViewMatrix( matrixIdentity );

  GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

  GL_Cull( CT_TWO_SIDED );

  // set time for 2D shaders
  backEnd.refdef.time = ri.Milliseconds();
  backEnd.refdef.floatTime = float(double(backEnd.refdef.time) * 0.001);
}

// used as bitfield
enum renderDrawSurfaces_e
{
  DRAWSURFACES_WORLD         = 1,
  DRAWSURFACES_FAR_ENTITIES  = 2,
  DRAWSURFACES_ALL_FAR       = 3,
  DRAWSURFACES_NEAR_ENTITIES = 4,
  DRAWSURFACES_ALL_ENTITIES  = 6,
  DRAWSURFACES_ALL           = 7
};

static void RB_RenderDrawSurfaces( bool opaque, renderDrawSurfaces_e drawSurfFilter )
{
  trRefEntity_t *entity, *oldEntity;
  shader_t      *shader, *oldShader;
  int           lightmapNum, oldLightmapNum;
  int           fogNum, oldFogNum;
  qboolean      depthRange, oldDepthRange, isTimedShader;
  int           i;
  drawSurf_t    *drawSurf;
  float         originalTime;

  // save original time for entity shader offsets
  originalTime = backEnd.refdef.floatTime;

  GLimp_LogComment( "--- RB_RenderDrawSurfaces ---\n" );

  // draw everything
  oldEntity = NULL;
  oldShader = NULL;
  oldLightmapNum = -1;
  oldFogNum = -1;
  oldDepthRange = qfalse;
  depthRange = qfalse;
  backEnd.currentLight = NULL;
  isTimedShader = qfalse;

  for ( i = 0, drawSurf = backEnd.viewParms.drawSurfs; i < backEnd.viewParms.numDrawSurfs; i++, drawSurf++ )
  {
    // update locals
    entity = drawSurf->entity;
    shader = tr.sortedShaders[ drawSurf->shaderNum ];
    lightmapNum = drawSurf->lightmapNum;
    fogNum = drawSurf->fogNum;
    isTimedShader = qfalse;

    if( entity == &tr.worldEntity ) {
      if( !( drawSurfFilter & DRAWSURFACES_WORLD ) )
        continue;
    } else if( !( entity->e.renderfx & RF_DEPTHHACK ) ) {
      if( !( drawSurfFilter & DRAWSURFACES_FAR_ENTITIES ) )
        continue;
    } else {
      if( !( drawSurfFilter & DRAWSURFACES_NEAR_ENTITIES ) )
        continue;
    }

    if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicEntityOcclusionCulling->integer && !entity->occlusionQuerySamples )
    {
      continue;
    }

    if ( opaque )
    {
      // skip all translucent surfaces that don't matter for this pass
      if ( shader->sort > SS_OPAQUE )
      {
        break; //hypov8 typo? or pre-sorted?
      }
    }
    else
    {
      // skip all opaque surfaces that don't matter for this pass
      if ( shader->sort <= SS_OPAQUE )
      {
        continue;
      }
    }

    if ( entity == oldEntity && shader == oldShader && lightmapNum == oldLightmapNum && fogNum == oldFogNum )
    {
      // fast path, same as previous sort
      rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
      continue;
    }

    // change the tess parameters if needed
    // an "entityMergable" shader is a shader that can have surfaces from separate
    // entities merged into a single batch, like smoke and blood puff sprites
    if ( shader != oldShader || lightmapNum != oldLightmapNum || fogNum != oldFogNum || ( entity != oldEntity && !shader->entityMergable ) )
    {
      if ( oldShader != NULL )
      {
        Tess_End();
      }

      Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, lightmapNum, fogNum );

      oldShader = shader;
      oldLightmapNum = lightmapNum;
      oldFogNum = fogNum;
    }

    // change the modelview matrix if needed
    if ( entity != oldEntity )
    {
      depthRange = qfalse;

      if ( entity != &tr.worldEntity )
      {
        backEnd.currentEntity = entity;

#ifdef COMPAT_KPQ3
        // used to sync "animMap"
        backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
#endif
        // set up the transformation matrix
        R_RotateEntityForViewParms( backEnd.currentEntity, &backEnd.viewParms, &backEnd.orientation );

        if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK )
        {
          // hack the depth range to prevent view model from poking into walls
          depthRange = qtrue;
        }
      }
      else
      {
        backEnd.currentEntity = &tr.worldEntity;
#ifdef COMPAT_KPQ3
        backEnd.refdef.floatTime = originalTime;
#endif
        backEnd.orientation = backEnd.viewParms.world;
      }

      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

      // change depthrange if needed
      if ( oldDepthRange != depthRange )
      {
        if ( depthRange )
        {
          glDepthRange( 0, 0.3 );
        }
        else
        {
          glDepthRange( 0, 1 );
        }

        oldDepthRange = depthRange;
      }

      oldEntity = entity;
    }

    // add the triangles for this surface
    rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
  }

  //backEnd.refdef.floatTime = originalTime;

  // draw the contents of the last shader batch
  if ( oldShader != NULL )
  {
    Tess_End();
  }

  backEnd.refdef.floatTime = originalTime;

  // go back to the world modelview matrix
  GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

  if ( depthRange )
  {
    glDepthRange( 0, 1 );
  }

  GL_CheckErrors();
}

#if 1 //hypov8 merge: todo use
static void RB_RenderOpaqueSurfacesIntoDepth( bool onlyWorld )
{
  trRefEntity_t *entity, *oldEntity;
  shader_t      *shader, *oldShader;
  qboolean      depthRange, oldDepthRange;
  qboolean      alphaTest, oldAlphaTest;
  deformType_t  deformType, oldDeformType;
  int           i;
  drawSurf_t    *drawSurf;

  GLimp_LogComment( "--- RB_RenderOpaqueSurfacesIntoDepth ---\n" );

  // draw everything
  oldEntity = NULL;
  oldShader = NULL;
  oldDepthRange = depthRange = qfalse;
  oldAlphaTest = alphaTest = qfalse;
  oldDeformType = deformType = DEFORM_TYPE_NONE;
  backEnd.currentLight = NULL;

  for ( i = 0, drawSurf = backEnd.viewParms.drawSurfs; i < backEnd.viewParms.numDrawSurfs; i++, drawSurf++ )
  {
    // update locals
    entity = drawSurf->entity;
    shader = tr.sortedShaders[ drawSurf->shaderNum ];
    alphaTest = shader->alphaTest;

#if 0

    if ( onlyWorld && ( entity != &tr.worldEntity ) )
    {
      continue;
    }

#endif

    // skip all translucent surfaces that don't matter for this pass
    if ( shader->sort > SS_OPAQUE )
    {
      break;
    }

    if ( shader->numDeforms )
    {
      deformType = ShaderRequiresCPUDeforms( shader ) ? DEFORM_TYPE_CPU : DEFORM_TYPE_GPU;
    }
    else
    {
      deformType = DEFORM_TYPE_NONE;
    }

    // change the tess parameters if needed
    // an "entityMergable" shader is a shader that can have surfaces from separate
    // entities merged into a single batch, like smoke and blood puff sprites
    //if(shader != oldShader || lightmapNum != oldLightmapNum || (entity != oldEntity && !shader->entityMergable))

    if ( entity == oldEntity && ( alphaTest ? shader == oldShader : alphaTest == oldAlphaTest ) && deformType == oldDeformType )
    {
      // fast path, same as previous sort
      rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
      continue;
    }
    else
    {
      if ( oldShader != NULL )
      {
        Tess_End();
      }

      Tess_Begin( Tess_StageIteratorDepthFill, NULL, shader, NULL, qtrue, qfalse, -1, 0 );

      oldShader = shader;
      oldAlphaTest = alphaTest;
      oldDeformType = deformType;
    }

    // change the modelview matrix if needed
    if ( entity != oldEntity )
    {
      depthRange = qfalse;

      if ( entity != &tr.worldEntity )
      {
        backEnd.currentEntity = entity;

        // set up the transformation matrix
        R_RotateEntityForViewParms( backEnd.currentEntity, &backEnd.viewParms, &backEnd.orientation );

        if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK )
        {
          // hack the depth range to prevent view model from poking into walls
          depthRange = qtrue;
        }
      }
      else
      {
        backEnd.currentEntity = &tr.worldEntity;
        backEnd.orientation = backEnd.viewParms.world;
      }

      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

      // change depthrange if needed
      if ( oldDepthRange != depthRange )
      {
        if ( depthRange )
        {
          glDepthRange( 0, 0.3 );
        }
        else
        {
          glDepthRange( 0, 1 );
        }

        oldDepthRange = depthRange;
      }

      oldEntity = entity;
    }

    // add the triangles for this surface
    rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
  }

  // draw the contents of the last shader batch
  if ( oldShader != NULL )
  {
    Tess_End();
  }

  // go back to the world modelview matrix
  GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

  if ( depthRange )
  {
    glDepthRange( 0, 1 );
  }

  GL_CheckErrors();
}

#endif

// *INDENT-OFF*
#ifdef VOLUMETRIC_LIGHTING
static void Render_lightVolume( interaction_t *ia )
{
  int           j;
  trRefLight_t  *light;
  shader_t      *lightShader;
  shaderStage_t *attenuationXYStage;
  shaderStage_t *attenuationZStage;
  matrix_t      ortho;
  vec4_t        quadVerts[ 4 ];

  light = ia->light;

  // set the window clipping
  GL_Viewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
               backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  // set light scissor to reduce fillrate
  GL_Scissor( ia->scissorX, ia->scissorY, ia->scissorWidth, ia->scissorHeight );

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY,
                              backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  switch ( light->l.rlType )
  {
    case RL_PROJ:
      {
        MatrixSetupTranslation( light->attenuationMatrix, 0.5, 0.5, 0.0 );  // bias
        MatrixMultiplyScale( light->attenuationMatrix, 0.5f, 0.5f, 1.0f / std::min( light->falloffLength, 1.0f ) );   // scale
        break;
      }

    case RL_OMNI:
    default:
      {
        MatrixSetupTranslation( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // bias
        MatrixMultiplyScale( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // scale
        break;
      }
  }

  MatrixMultiply2( light->attenuationMatrix, light->projectionMatrix );  // light projection (frustum)
  MatrixMultiply2( light->attenuationMatrix, light->viewMatrix );

  lightShader = light->shader;
  attenuationZStage = lightShader->stages[ 0 ];

  for ( j = 1; j < MAX_SHADER_STAGES; j++ )
  {
    attenuationXYStage = lightShader->stages[ j ];

    if ( !attenuationXYStage )
    {
      break;
    }

    if ( attenuationXYStage->type != ST_ATTENUATIONMAP_XY )
    {
      continue;
    }

    if ( !RB_EvalExpression( &attenuationXYStage->ifExp, 1.0 ) )
    {
      continue;
    }

    Tess_ComputeColor( attenuationXYStage );
    R_ComputeFinalAttenuation( attenuationXYStage, light );

    if ( light->l.rlType == RL_OMNI )
    {
      vec3_t   viewOrigin;
      vec3_t   lightOrigin;
      vec4_t   lightColor;
      qboolean shadowCompare;

      GLimp_LogComment( "--- Render_lightVolume_omni ---\n" );

      // enable shader, set arrays
      gl_lightVolumeShader_omni->BindProgram();
      //GL_VertexAttribsState(tr.lightVolumeShader_omni.attribs);
      GL_Cull( CT_TWO_SIDED );
      GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
      //GL_State(GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
      //GL_State(GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
      //GL_State(attenuationXYStage->stateBits & ~(GLS_DEPTHMASK_TRUE | GLS_DEPTHTEST_DISABLE));

      // set uniforms
      VectorCopy( backEnd.viewParms.orientation.origin, viewOrigin );  // in world space
      VectorCopy( light->origin, lightOrigin );
      VectorCopy( tess.svars.color, lightColor );

      shadowCompare = (qboolean)((int)r_shadows->integer >= SHADOWING_ESM16 && !light->l.noShadows && light->shadowLOD >= 0);

      gl_lightVolumeShader_omni->SetUniform_ViewOrigin( viewOrigin );
      gl_lightVolumeShader_omni->SetUniform_LightOrigin( lightOrigin );
      gl_lightVolumeShader_omni->SetUniform_LightColor( lightColor );
      gl_lightVolumeShader_omni->SetUniform_LightRadius( light->sphereRadius );
      gl_lightVolumeShader_omni->SetUniform_LightScale( light->l.scale );
      gl_lightVolumeShader_omni->SetUniform_LightAttenuationMatrix( light->attenuationMatrix2 );

      // FIXME gl_lightVolumeShader_omni->SetUniform_ShadowMatrix( light->attenuationMatrix );
      gl_lightVolumeShader_omni->SetShadowing( shadowCompare );
      gl_lightVolumeShader_omni->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      gl_lightVolumeShader_omni->SetUniform_UnprojectMatrix( backEnd.viewParms.unprojectionMatrix );

      //gl_lightVolumeShader_omni->SetUniform_PortalClipping( backEnd.viewParms.isPortal );

      // bind u_DepthMap
      if ( r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable )
      {
        GL_BindToTMU( 0, tr.depthRenderImage );
      }
      else
      {
        // depth texture is not bound to a FBO
        GL_SelectTexture( 0 );
        GL_Bind( tr.depthRenderImage );
        glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.depthRenderImage->uploadWidth, tr.depthRenderImage->uploadHeight );
      }

      // bind u_AttenuationMapXY
      GL_SelectTexture( 1 );
      BindAnimatedImage( &attenuationXYStage->bundle[ TB_COLORMAP ] );

      // bind u_AttenuationMapZ
      GL_SelectTexture( 2 );
      BindAnimatedImage( &attenuationZStage->bundle[ TB_COLORMAP ] );

      // bind u_ShadowMap
      if ( shadowCompare )
      {
        GL_BindToTMU( 3, tr.shadowCubeFBOImage[ light->shadowLOD ] );
      }

      // draw light scissor rectangle
      Vector4Set( quadVerts[ 0 ], ia->scissorX, ia->scissorY, 0, 1 );
      Vector4Set( quadVerts[ 1 ], ia->scissorX + ia->scissorWidth - 1, ia->scissorY, 0, 1 );
      Vector4Set( quadVerts[ 2 ], ia->scissorX + ia->scissorWidth - 1, ia->scissorY + ia->scissorHeight - 1, 0,
                  1 );
      Vector4Set( quadVerts[ 3 ], ia->scissorX, ia->scissorY + ia->scissorHeight - 1, 0, 1 );
      Tess_InstantQuad( quadVerts );

      GL_CheckErrors();
    }
  }

  GL_PopMatrix();
}

#endif
// *INDENT-ON*

/*
 * helper function for parallel split shadow mapping
 */
static int MergeInteractionBounds( const matrix_t lightViewProjectionMatrix, interaction_t *ia, int iaCount, vec3_t bounds[ 2 ], bool shadowCasters )
{
  int           i;
  int           j;
  surfaceType_t *surface;
  vec4_t        point;
  vec4_t        transf;
  vec3_t        worldBounds[ 2 ];
  //vec3_t    viewBounds[2];
  //vec3_t    center;
  //float     radius;
  int       numCasters;

  frustum_t frustum;
  cplane_t  *clipPlane;
  int       r;

  numCasters = 0;
  ClearBounds( bounds[ 0 ], bounds[ 1 ] );

  // calculate frustum planes using the modelview projection matrix
  R_SetupFrustum2( frustum, lightViewProjectionMatrix );

  while ( iaCount < backEnd.viewParms.numInteractions )
  {
    surface = ia->surface;

    if ( shadowCasters )
    {
      if ( !(ia->type & IA_SHADOW) )
      {
        goto skipInteraction;
      }
    }
    else
    {
      // we only merge shadow receivers
      if ( !(ia->type & IA_LIGHT) )
      {
        goto skipInteraction;
      }
    }

    if ( *surface == SF_FACE || *surface == SF_GRID || *surface == SF_TRIANGLES )
    {
      srfGeneric_t *gen = ( srfGeneric_t * ) surface;

      VectorCopy( gen->bounds[ 0 ], worldBounds[ 0 ] );
      VectorCopy( gen->bounds[ 1 ], worldBounds[ 1 ] );
    }
    else if ( *surface == SF_VBO_MESH )
    {
      srfVBOMesh_t *srf = ( srfVBOMesh_t * ) surface;

      //ri.Printf(PRINT_ALL, "merging vbo mesh bounds\n");

      VectorCopy( srf->bounds[ 0 ], worldBounds[ 0 ] );
      VectorCopy( srf->bounds[ 1 ], worldBounds[ 1 ] );
    }
    else if ( *surface == SF_MDV )
    {
      //Tess_AddCube(vec3_origin, entity->localBounds[0], entity->localBounds[1], lightColor);
      goto skipInteraction;
    }
    else
    {
      goto skipInteraction;
    }

#if 1

    // use the frustum planes to cut off shadow casters beyond the split frustum
    for ( i = 0; i < 6; i++ )
    {
      clipPlane = &frustum[ i ];

      // we can have shadow casters outside the initial computed light view frustum
      if ( i == FRUSTUM_NEAR && shadowCasters )
      {
        continue;
      }

      r = BoxOnPlaneSide( worldBounds[ 0 ], worldBounds[ 1 ], clipPlane );

      if ( r == 2 )
      {
        goto skipInteraction;
      }
    }

#endif

    if ( shadowCasters && (ia->type & IA_SHADOW) )
    {
      numCasters++;
    }

#if 1

    for ( j = 0; j < 8; j++ )
    {
      point[ 0 ] = worldBounds[ j & 1 ][ 0 ];
      point[ 1 ] = worldBounds[( j >> 1 ) & 1 ][ 1 ];
      point[ 2 ] = worldBounds[( j >> 2 ) & 1 ][ 2 ];
      point[ 3 ] = 1;

      MatrixTransform4( lightViewProjectionMatrix, point, transf );
      transf[ 0 ] /= transf[ 3 ];
      transf[ 1 ] /= transf[ 3 ];
      transf[ 2 ] /= transf[ 3 ];

      AddPointToBounds( transf, bounds[ 0 ], bounds[ 1 ] );
    }

#elif 0
    ClearBounds( viewBounds[ 0 ], viewBounds[ 1 ] );

    for ( j = 0; j < 8; j++ )
    {
      point[ 0 ] = worldBounds[ j & 1 ][ 0 ];
      point[ 1 ] = worldBounds[( j >> 1 ) & 1 ][ 1 ];
      point[ 2 ] = worldBounds[( j >> 2 ) & 1 ][ 2 ];
      point[ 3 ] = 1;

      MatrixTransform4( lightViewProjectionMatrix, point, transf );
      transf[ 0 ] /= transf[ 3 ];
      transf[ 1 ] /= transf[ 3 ];
      transf[ 2 ] /= transf[ 3 ];

      AddPointToBounds( transf, viewBounds[ 0 ], viewBounds[ 1 ] );
    }

    // get sphere of AABB
    VectorAdd( viewBounds[ 0 ], viewBounds[ 1 ], center );
    VectorScale( center, 0.5, center );

    radius = RadiusFromBounds( viewBounds[ 0 ], viewBounds[ 1 ] );

    for ( j = 0; j < 3; j++ )
    {
      if ( ( transf[ j ] - radius ) < bounds[ 0 ][ j ] )
      {
        bounds[ 0 ][ j ] = transf[ i ] - radius;
      }

      if ( ( transf[ j ] + radius ) > bounds[ 1 ][ j ] )
      {
        bounds[ 1 ][ j ] = transf[ i ] + radius;
      }
    }

#else

    ClearBounds( viewBounds[ 0 ], viewBounds[ 1 ] );

    for ( j = 0; j < 8; j++ )
    {
      point[ 0 ] = worldBounds[ j & 1 ][ 0 ];
      point[ 1 ] = worldBounds[( j >> 1 ) & 1 ][ 1 ];
      point[ 2 ] = worldBounds[( j >> 2 ) & 1 ][ 2 ];
      point[ 3 ] = 1;

      MatrixTransform4( lightViewProjectionMatrix, point, transf );
      //transf[0] /= transf[3];
      //transf[1] /= transf[3];
      //transf[2] /= transf[3];

      AddPointToBounds( transf, viewBounds[ 0 ], viewBounds[ 1 ] );
    }

    // get sphere of AABB
    VectorAdd( viewBounds[ 0 ], viewBounds[ 1 ], center );
    VectorScale( center, 0.5, center );

    //MatrixTransform4(lightViewProjectionMatrix, center, transf);
    //transf[0] /= transf[3];
    //transf[1] /= transf[3];
    //transf[2] /= transf[3];

    radius = RadiusFromBounds( viewBounds[ 0 ], viewBounds[ 1 ] );

    if ( ( transf[ 2 ] + radius ) > bounds[ 1 ][ 2 ] )
    {
      bounds[ 1 ][ 2 ] = transf[ 2 ] + radius;
    }

#endif

skipInteraction:

    if ( !ia->next )
    {
      // this is the last interaction of the current light
      break;
    }
    else
    {
      // just continue
      ia = ia->next;
      iaCount++;
    }
  }

  return numCasters;
}

static interaction_t *IterateLights( const interaction_t *prev )
{
  if ( !prev && backEnd.viewParms.numInteractions > 0 )
  {
    return backEnd.viewParms.interactions;
  }

  if ( backEnd.viewParms.numInteractions <= 0 )
  {
    return NULL;
  }

  const interaction_t *next = prev;
  const interaction_t *last = &backEnd.viewParms.interactions[ backEnd.viewParms.numInteractions - 1 ];

  while ( next <= last && next->light == prev->light )
  {
    next++;
  }

  if ( next > last )
  {
    next = NULL;
  }

  return ( interaction_t * ) next;
}

static void RB_SetupLightAttenuationForEntity( trRefLight_t *light, const trRefEntity_t *entity )
{
  matrix_t modelToLight;

  // transform light origin into model space for u_LightOrigin parameter
  if ( entity != &tr.worldEntity )
  {
    vec3_t tmp;
    VectorSubtract( light->origin, backEnd.orientation.origin, tmp );
    light->transformed[0] = DotProduct(tmp, backEnd.orientation.axis[0]);
    light->transformed[1] = DotProduct(tmp, backEnd.orientation.axis[1]);
    light->transformed[2] = DotProduct(tmp, backEnd.orientation.axis[2]);
  }
  else
  {
    VectorCopy( light->origin, light->transformed );
  }

  MatrixMultiply( light->viewMatrix, backEnd.orientation.transformMatrix, modelToLight );

  // build the attenuation matrix using the entity transform
  switch ( light->l.rlType )
  {
    case RL_OMNI:
      {
        MatrixSetupTranslation( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // bias
        MatrixMultiplyScale( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // scale
        MatrixMultiply2( light->attenuationMatrix, light->projectionMatrix );
        MatrixMultiply2( light->attenuationMatrix, modelToLight );

        MatrixCopy( light->attenuationMatrix, light->shadowMatrices[ 0 ] );
        break;
      }

    case RL_PROJ:
      {
        MatrixSetupTranslation( light->attenuationMatrix, 0.5, 0.5, 0.0 );  // bias
        MatrixMultiplyScale( light->attenuationMatrix, 0.5f, 0.5f, 1.0f / std::min( light->falloffLength, 1.0f ) );   // scale
        MatrixMultiply2( light->attenuationMatrix, light->projectionMatrix );
        MatrixMultiply2( light->attenuationMatrix, modelToLight );

        MatrixCopy( light->attenuationMatrix, light->shadowMatrices[ 0 ] );
        break;
      }

    case RL_DIRECTIONAL:
      {
        MatrixSetupTranslation( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // bias
        MatrixMultiplyScale( light->attenuationMatrix, 0.5, 0.5, 0.5 );  // scale
        MatrixMultiply2( light->attenuationMatrix, light->projectionMatrix );
        MatrixMultiply2( light->attenuationMatrix, modelToLight );
        break;
      }

    case RL_MAX_REF_LIGHT_TYPE:
      {
        //Nothing for right now...
        break;
      }
  }
}

/*
=================
RB_RenderInteractions
=================
*/
static void RB_RenderInteractions()
{
  shader_t      *shader, *oldShader;
  trRefEntity_t *entity, *oldEntity;
  trRefLight_t  *light;
  const interaction_t *ia;
  const interaction_t *iaFirst;
  qboolean      depthRange, oldDepthRange;
  surfaceType_t *surface;
  int           startTime = 0, endTime = 0;

  GLimp_LogComment( "--- RB_RenderInteractions ---\n" );

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    startTime = ri.Milliseconds();
  }

  // draw everything
  oldEntity = NULL;
  oldShader = NULL;
  oldDepthRange = qfalse;
  depthRange = qfalse;
  iaFirst = NULL;

  // render interactions
  while ( ( iaFirst = IterateLights( iaFirst ) ) )
  {
    backEnd.currentLight = light = iaFirst->light;

    // skip all interactions of this light because it failed the occlusion query
    if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicLightOcclusionCulling->integer && !iaFirst->occlusionQuerySamples )
    {
      continue;
    }

    // set light scissor to reduce fillrate
    GL_Scissor( iaFirst->scissorX, iaFirst->scissorY, iaFirst->scissorWidth, iaFirst->scissorHeight );

    for ( ia = iaFirst; ia; ia = ia->next )
    {
      backEnd.currentEntity = entity = ia->entity;
      surface = ia->surface;
      shader = tr.sortedShaders[ ia->shaderNum ];

      if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicEntityOcclusionCulling->integer && !entity->occlusionQuerySamples )
      {
        continue;
      }

      if ( !shader || !shader->interactLight )
      {
        // skip this interaction because the surface shader has no ability to interact with light
        // this will save texcoords and matrix calculations
        continue;
      }

      if ( !(ia->type & IA_LIGHT) )
      {
        // skip this interaction because the interaction is meant for shadowing only
        continue;
      }

      GLimp_LogComment( "----- Rendering new light -----\n" );

      // Tr3B: this should never happen in the first iteration
      if ( entity == oldEntity && shader == oldShader )
      {
        // fast path, same as previous
        rb_surfaceTable[ *surface ]( surface );
        continue;
      }

      // draw the contents of the last shader batch
      Tess_End();

      // begin a new batch
      Tess_Begin( Tess_StageIteratorLighting, NULL, shader, light->shader, qfalse, qfalse, -1, 0 );

      // change the modelview matrix if needed
      if ( entity != oldEntity )
      {
        depthRange = qfalse;

        if ( entity != &tr.worldEntity )
        {
          // set up the transformation matrix
          R_RotateEntityForViewParms( backEnd.currentEntity, &backEnd.viewParms, &backEnd.orientation );

          if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK )
          {
            // hack the depth range to prevent view model from poking into walls
            depthRange = qtrue;
          }
        }
        else
        {
          backEnd.orientation = backEnd.viewParms.world;
        }

        GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

        // change depthrange if needed
        if ( oldDepthRange != depthRange )
        {
          if ( depthRange )
          {
            glDepthRange( 0, 0.3 );
          }
          else
          {
            glDepthRange( 0, 1 );
          }

          oldDepthRange = depthRange;
        }

        RB_SetupLightAttenuationForEntity( light, entity );
      }

      // add the triangles for this surface
      rb_surfaceTable[ *surface ]( surface );
      oldEntity = entity;
      oldShader = shader;
    }

    // draw the contents of the last shader batch
    Tess_End();

#ifdef VOLUMETRIC_LIGHTING
      // draw the light volume if needed
      if ( light->shader->volumetricLight )
      {
        Render_lightVolume( ia );
      }
#endif

    // force updates
    oldEntity = NULL;
    oldShader = NULL;
  }

  Tess_End();

  // go back to the world modelview matrix
  GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

  if ( depthRange )
  {
    glDepthRange( 0, 1 );
  }

  // reset scissor
  GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
              backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  GL_CheckErrors();

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    endTime = ri.Milliseconds();
    backEnd.pc.c_forwardLightingTime = endTime - startTime;
  }
}

static deformType_t GetDeformType( const shader_t *shader )
{
  deformType_t deformType;
  if ( shader->numDeforms )
  {
    deformType = ShaderRequiresCPUDeforms( shader ) ? DEFORM_TYPE_CPU : DEFORM_TYPE_GPU;
  }
  else
  {
    deformType = DEFORM_TYPE_NONE;
  }
  return deformType;
}

static void RB_SetupLightForShadowing( trRefLight_t *light, int index,
               qboolean shadowClip )
{
  // HACK: bring OpenGL into a safe state or strange FBO update problems will occur
  GL_BindProgram( NULL );
  GL_State( GLS_DEFAULT );
  //GL_VertexAttribsState(ATTR_POSITION);

  GL_BindToTMU( 0, tr.whiteImage );
  int cubeSide = index;
  int splitFrustumIndex = index;
  interaction_t *ia = light->firstInteraction;
  int iaCount = ia - backEnd.viewParms.interactions;

  switch ( light->l.rlType )
  {
    case RL_OMNI:
      {
        //float           xMin, xMax, yMin, yMax;
        //float           width, height, depth;
        float    zNear, zFar;
        float    fovX, fovY;
        qboolean flipX, flipY;
        //float          *proj;
        vec3_t   angles;
        matrix_t rotationMatrix, transformMatrix, viewMatrix;

        if ( r_logFile->integer )
        {
          // don't just call LogComment, or we will get
          // a call to va() every frame!
          GLimp_LogComment( va( "----- Rendering shadowCube side: %i -----\n", cubeSide ) );
        }

        R_BindFBO( tr.shadowMapFBO[ light->shadowLOD ] );
        if( shadowClip )
        {
          R_AttachFBOTexture2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeSide,
                    tr.shadowClipCubeFBOImage[ light->shadowLOD ]->texnum, 0 );
        }
        else
        {
          R_AttachFBOTexture2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeSide,
                    tr.shadowCubeFBOImage[ light->shadowLOD ]->texnum, 0 );
        }

        if ( !r_ignoreGLErrors->integer )
        {
          R_CheckFBO( tr.shadowMapFBO[ light->shadowLOD ] );
        }

        // set the window clipping
        GL_Viewport( 0, 0, shadowMapResolutions[ light->shadowLOD ], shadowMapResolutions[ light->shadowLOD ] );
        GL_Scissor( 0, 0, shadowMapResolutions[ light->shadowLOD ], shadowMapResolutions[ light->shadowLOD ] );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //backEnd.depthRenderImageValid = qfalse;

        switch ( cubeSide )
        {
          case 0:
            {
              // view parameters
              VectorSet( angles, 0, 0, 90 );

              // projection parameters
              flipX = qfalse;
              flipY = qfalse;
              break;
            }

          case 1:
            {
              VectorSet( angles, 0, 180, 90 );
              flipX = qtrue;
              flipY = qtrue;
              break;
            }

          case 2:
            {
              VectorSet( angles, 0, 90, 0 );
              flipX = qfalse;
              flipY = qfalse;
              break;
            }

          case 3:
            {
              VectorSet( angles, 0, -90, 0 );
              flipX = qtrue;
              flipY = qtrue;
              break;
            }

          case 4:
            {
              VectorSet( angles, -90, 90, 0 );
              flipX = qfalse;
              flipY = qfalse;
              break;
            }

          case 5:
            {
              VectorSet( angles, 90, 90, 0 );
              flipX = qtrue;
              flipY = qtrue;
              break;
            }

          default:
            {
              // shut up compiler
              VectorSet( angles, 0, 0, 0 );
              flipX = qfalse;
              flipY = qfalse;
              break;
            }
        }

        // Quake -> OpenGL view matrix from light perspective
        MatrixFromAngles( rotationMatrix, angles[ PITCH ], angles[ YAW ], angles[ ROLL ] );
        MatrixSetupTransformFromRotation( transformMatrix, rotationMatrix, light->origin );
        MatrixAffineInverse( transformMatrix, viewMatrix );

        // convert from our coordinate system (looking down X)
        // to OpenGL's coordinate system (looking down -Z)
        MatrixMultiply( quakeToOpenGLMatrix, viewMatrix, light->viewMatrix );

        // OpenGL projection matrix
        fovX = 90;
        fovY = 90;

        zNear = 1.0;
        zFar = light->sphereRadius;

        if ( flipX )
        {
          fovX = -fovX;
        }

        if ( flipY )
        {
          fovY = -fovY;
        }

        MatrixPerspectiveProjectionFovXYRH( light->projectionMatrix, fovX, fovY, zNear, zFar );

        GL_LoadProjectionMatrix( light->projectionMatrix );
        break;
      }

    case RL_PROJ:
      {
        GLimp_LogComment( "--- Rendering projective shadowMap ---\n" );

        R_BindFBO( tr.shadowMapFBO[ light->shadowLOD ] );
        if( shadowClip )
        {
          R_AttachFBOTexture2D( GL_TEXTURE_2D, tr.shadowClipMapFBOImage[ light->shadowLOD ]->texnum, 0 );
        }
        else
        {
          R_AttachFBOTexture2D( GL_TEXTURE_2D, tr.shadowMapFBOImage[ light->shadowLOD ]->texnum, 0 );
        }

        if ( !r_ignoreGLErrors->integer )
        {
          R_CheckFBO( tr.shadowMapFBO[ light->shadowLOD ] );
        }

        // set the window clipping
        GL_Viewport( 0, 0, shadowMapResolutions[ light->shadowLOD ], shadowMapResolutions[ light->shadowLOD ] );
        GL_Scissor( 0, 0, shadowMapResolutions[ light->shadowLOD ], shadowMapResolutions[ light->shadowLOD ] );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //backEnd.depthRenderImageValid = qfalse;

        GL_LoadProjectionMatrix( light->projectionMatrix );
        break;
      }

    case RL_DIRECTIONAL:
      {
        int      j;
        vec3_t   angles;
        vec4_t   forward, side, up;
        vec3_t   lightDirection;
        vec3_t   viewOrigin, viewDirection;
        matrix_t rotationMatrix, transformMatrix, viewMatrix, projectionMatrix, viewProjectionMatrix;
        matrix_t cropMatrix;
        vec4_t   splitFrustum[ 6 ];
        vec3_t   splitFrustumCorners[ 8 ];
        vec3_t   splitFrustumBounds[ 2 ];
//							vec3_t     splitFrustumViewBounds[2];
        vec3_t   splitFrustumClipBounds[ 2 ];
//							float      splitFrustumRadius;
        int      numCasters;
        vec3_t   casterBounds[ 2 ];
        vec3_t   receiverBounds[ 2 ];
        vec3_t   cropBounds[ 2 ];
        vec4_t   point;
        vec4_t   transf;

        GLimp_LogComment( "--- Rendering directional shadowMap ---\n" );

        R_BindFBO( tr.sunShadowMapFBO[ splitFrustumIndex ] );

        if( shadowClip )
        {
          R_AttachFBOTexture2D( GL_TEXTURE_2D, tr.sunShadowClipMapFBOImage[ splitFrustumIndex ]->texnum, 0 );
        }
        else if ( !r_evsmPostProcess->integer )
        {
          R_AttachFBOTexture2D( GL_TEXTURE_2D, tr.sunShadowMapFBOImage[ splitFrustumIndex ]->texnum, 0 );
        }
        else
        {
          R_AttachFBOTextureDepth( tr.sunShadowMapFBOImage[ splitFrustumIndex ]->texnum );
        }

        if ( !r_ignoreGLErrors->integer )
        {
          R_CheckFBO( tr.sunShadowMapFBO[ splitFrustumIndex ] );
        }

        // set the window clipping
        GL_Viewport( 0, 0, sunShadowMapResolutions[ splitFrustumIndex ], sunShadowMapResolutions[ splitFrustumIndex ] );
        GL_Scissor( 0, 0, sunShadowMapResolutions[ splitFrustumIndex ], sunShadowMapResolutions[ splitFrustumIndex ] );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //backEnd.depthRenderImageValid = qfalse;

#if 1
        VectorCopy( tr.sunDirection, lightDirection );
#else
        VectorCopy( light->direction, lightDirection );
#endif

        if ( r_parallelShadowSplits->integer )
        {
          // original light direction is from surface to light
          VectorInverse( lightDirection );
          VectorNormalize( lightDirection );

          VectorCopy( backEnd.viewParms.orientation.origin, viewOrigin );
          VectorCopy( backEnd.viewParms.orientation.axis[ 0 ], viewDirection );
          VectorNormalize( viewDirection );

          // calculate new up dir
          CrossProduct( lightDirection, viewDirection, side );
          VectorNormalize( side );

          CrossProduct( side, lightDirection, up );
          VectorNormalize( up );

          VectorToAngles( lightDirection, angles );
          MatrixFromAngles( rotationMatrix, angles[ PITCH ], angles[ YAW ], angles[ ROLL ] );
          AngleVectors( angles, forward, side, up );

          MatrixLookAtRH( light->viewMatrix, viewOrigin, lightDirection, up );

          for ( j = 0; j < 6; j++ )
          {
            VectorCopy( backEnd.viewParms.frustums[ 1 + splitFrustumIndex ][ j ].normal, splitFrustum[ j ] );
            splitFrustum[ j ][ 3 ] = backEnd.viewParms.frustums[ 1 + splitFrustumIndex ][ j ].dist;
          }

          R_CalcFrustumNearCorners( splitFrustum, splitFrustumCorners );
          R_CalcFrustumFarCorners( splitFrustum, splitFrustumCorners + 4 );

          if ( r_logFile->integer )
          {
            vec3_t rayIntersectionNear, rayIntersectionFar;
            float  zNear, zFar;

            // don't just call LogComment, or we will get
            // a call to va() every frame!
            //GLimp_LogComment(va("----- Skipping shadowCube side: %i -----\n", cubeSide));

            PlaneIntersectRay( viewOrigin, viewDirection, splitFrustum[ FRUSTUM_FAR ], rayIntersectionFar );
            zFar = Distance( viewOrigin, rayIntersectionFar );

            VectorInverse( viewDirection );

            PlaneIntersectRay( rayIntersectionFar, viewDirection, splitFrustum[ FRUSTUM_NEAR ], rayIntersectionNear );
            zNear = Distance( viewOrigin, rayIntersectionNear );

            VectorInverse( viewDirection );

            GLimp_LogComment( va( "split frustum %i: near = %5.3f, far = %5.3f\n", splitFrustumIndex, zNear, zFar ) );
            GLimp_LogComment( va( "pyramid nearCorners\n" ) );

            for ( j = 0; j < 4; j++ )
            {
              GLimp_LogComment( va( "(%5.3f, %5.3f, %5.3f)\n", splitFrustumCorners[ j ][ 0 ], splitFrustumCorners[ j ][ 1 ], splitFrustumCorners[ j ][ 2 ] ) );
            }

            GLimp_LogComment( va( "pyramid farCorners\n" ) );

            for ( j = 4; j < 8; j++ )
            {
              GLimp_LogComment( va( "(%5.3f, %5.3f, %5.3f)\n", splitFrustumCorners[ j ][ 0 ], splitFrustumCorners[ j ][ 1 ], splitFrustumCorners[ j ][ 2 ] ) );
            }
          }

          ClearBounds( splitFrustumBounds[ 0 ], splitFrustumBounds[ 1 ] );

          for ( j = 0; j < 8; j++ )
          {
            AddPointToBounds( splitFrustumCorners[ j ], splitFrustumBounds[ 0 ], splitFrustumBounds[ 1 ] );
          }



          //
          // Scene-Dependent Projection
          //

          // find the bounding box of the current split in the light's view space
          ClearBounds( cropBounds[ 0 ], cropBounds[ 1 ] );

          for ( j = 0; j < 8; j++ )
          {
            VectorCopy( splitFrustumCorners[ j ], point );
            point[ 3 ] = 1;
            MatrixTransform4( light->viewMatrix, point, transf );
            transf[ 0 ] /= transf[ 3 ];
            transf[ 1 ] /= transf[ 3 ];
            transf[ 2 ] /= transf[ 3 ];

            AddPointToBounds( transf, cropBounds[ 0 ], cropBounds[ 1 ] );
          }

          MatrixOrthogonalProjectionRH( projectionMatrix, cropBounds[ 0 ][ 0 ], cropBounds[ 1 ][ 0 ], cropBounds[ 0 ][ 1 ], cropBounds[ 1 ][ 1 ], -cropBounds[ 1 ][ 2 ], -cropBounds[ 0 ][ 2 ] );

          MatrixMultiply( projectionMatrix, light->viewMatrix, viewProjectionMatrix );

          numCasters = MergeInteractionBounds( viewProjectionMatrix, ia, iaCount, casterBounds, qtrue );
          MergeInteractionBounds( viewProjectionMatrix, ia, iaCount, receiverBounds, qfalse );

          // find the bounding box of the current split in the light's clip space
          ClearBounds( splitFrustumClipBounds[ 0 ], splitFrustumClipBounds[ 1 ] );

          for ( j = 0; j < 8; j++ )
          {
            VectorCopy( splitFrustumCorners[ j ], point );
            point[ 3 ] = 1;

            MatrixTransform4( viewProjectionMatrix, point, transf );
            transf[ 0 ] /= transf[ 3 ];
            transf[ 1 ] /= transf[ 3 ];
            transf[ 2 ] /= transf[ 3 ];

            AddPointToBounds( transf, splitFrustumClipBounds[ 0 ], splitFrustumClipBounds[ 1 ] );
          }

          if ( r_logFile->integer )
          {
            GLimp_LogComment( va( "shadow casters = %i\n", numCasters ) );

            GLimp_LogComment( va( "split frustum light space clip bounds (%5.3f, %5.3f, %5.3f) (%5.3f, %5.3f, %5.3f)\n",
                            splitFrustumClipBounds[ 0 ][ 0 ], splitFrustumClipBounds[ 0 ][ 1 ], splitFrustumClipBounds[ 0 ][ 2 ],
                            splitFrustumClipBounds[ 1 ][ 0 ], splitFrustumClipBounds[ 1 ][ 1 ], splitFrustumClipBounds[ 1 ][ 2 ] ) );

            GLimp_LogComment( va( "shadow caster light space clip bounds (%5.3f, %5.3f, %5.3f) (%5.3f, %5.3f, %5.3f)\n",
                            casterBounds[ 0 ][ 0 ], casterBounds[ 0 ][ 1 ], casterBounds[ 0 ][ 2 ],
                            casterBounds[ 1 ][ 0 ], casterBounds[ 1 ][ 1 ], casterBounds[ 1 ][ 2 ] ) );

            GLimp_LogComment( va( "light receiver light space clip bounds (%5.3f, %5.3f, %5.3f) (%5.3f, %5.3f, %5.3f)\n",
                            receiverBounds[ 0 ][ 0 ], receiverBounds[ 0 ][ 1 ], receiverBounds[ 0 ][ 2 ],
                            receiverBounds[ 1 ][ 0 ], receiverBounds[ 1 ][ 1 ], receiverBounds[ 1 ][ 2 ] ) );
          }

          // scene-dependent bounding volume
          cropBounds[ 0 ][ 0 ] = std::max( std::max( casterBounds[ 0 ][ 0 ], receiverBounds[ 0 ][ 0 ] ), splitFrustumClipBounds[ 0 ][ 0 ] );
          cropBounds[ 0 ][ 1 ] = std::max( std::max( casterBounds[ 0 ][ 1 ], receiverBounds[ 0 ][ 1 ] ), splitFrustumClipBounds[ 0 ][ 1 ] );

          cropBounds[ 1 ][ 0 ] = std::min( std::min( casterBounds[ 1 ][ 0 ], receiverBounds[ 1 ][ 0 ] ), splitFrustumClipBounds[ 1 ][ 0 ] );
          cropBounds[ 1 ][ 1 ] = std::min( std::min( casterBounds[ 1 ][ 1 ], receiverBounds[ 1 ][ 1 ] ), splitFrustumClipBounds[ 1 ][ 1 ] );

          cropBounds[ 0 ][ 2 ] = std::min( casterBounds[ 0 ][ 2 ], splitFrustumClipBounds[ 0 ][ 2 ] );
          //cropBounds[0][2] = casterBounds[0][2];
          //cropBounds[0][2] = splitFrustumClipBounds[0][2];
          cropBounds[ 1 ][ 2 ] = std::min( receiverBounds[ 1 ][ 2 ], splitFrustumClipBounds[ 1 ][ 2 ] );
          //cropBounds[1][2] = splitFrustumClipBounds[1][2];

          if ( numCasters == 0 )
          {
            VectorCopy( splitFrustumClipBounds[ 0 ], cropBounds[ 0 ] );
            VectorCopy( splitFrustumClipBounds[ 1 ], cropBounds[ 1 ] );
          }

          MatrixCrop( cropMatrix, cropBounds[ 0 ], cropBounds[ 1 ] );

          MatrixMultiply( cropMatrix, projectionMatrix, light->projectionMatrix );

          GL_LoadProjectionMatrix( light->projectionMatrix );
        }
        else
        {
          // original light direction is from surface to light
          VectorInverse( lightDirection );

          // Quake -> OpenGL view matrix from light perspective
#if 1
          VectorToAngles( lightDirection, angles );
          MatrixFromAngles( rotationMatrix, angles[ PITCH ], angles[ YAW ], angles[ ROLL ] );
          MatrixSetupTransformFromRotation( transformMatrix, rotationMatrix, backEnd.viewParms.orientation.origin );
          MatrixAffineInverse( transformMatrix, viewMatrix );
          MatrixMultiply( quakeToOpenGLMatrix, viewMatrix, light->viewMatrix );
#else
          MatrixLookAtRH( light->viewMatrix, backEnd.viewParms.orientation.origin, lightDirection, backEnd.viewParms.orientation.axis[ 0 ] );
#endif

          ClearBounds( splitFrustumBounds[ 0 ], splitFrustumBounds[ 1 ] );
          //BoundsAdd(splitFrustumBounds[0], splitFrustumBounds[1], backEnd.viewParms.visBounds[0], backEnd.viewParms.visBounds[1]);
          BoundsAdd( splitFrustumBounds[ 0 ], splitFrustumBounds[ 1 ], light->worldBounds[ 0 ], light->worldBounds[ 1 ] );

          ClearBounds( cropBounds[ 0 ], cropBounds[ 1 ] );

          for ( j = 0; j < 8; j++ )
          {
            point[ 0 ] = splitFrustumBounds[ j & 1 ][ 0 ];
            point[ 1 ] = splitFrustumBounds[( j >> 1 ) & 1 ][ 1 ];
            point[ 2 ] = splitFrustumBounds[( j >> 2 ) & 1 ][ 2 ];
            point[ 3 ] = 1;

            MatrixTransform4( light->viewMatrix, point, transf );
            transf[ 0 ] /= transf[ 3 ];
            transf[ 1 ] /= transf[ 3 ];
            transf[ 2 ] /= transf[ 3 ];

            AddPointToBounds( transf, cropBounds[ 0 ], cropBounds[ 1 ] );
          }

#if 0
          MatrixScaleTranslateToUnitCube( projectionMatrix, cropBounds[ 0 ], cropBounds[ 1 ] );
          MatrixMultiply( flipZMatrix, projectionMatrix, light->projectionMatrix );
#else
          MatrixOrthogonalProjectionRH( light->projectionMatrix, cropBounds[ 0 ][ 0 ], cropBounds[ 1 ][ 0 ], cropBounds[ 0 ][ 1 ], cropBounds[ 1 ][ 1 ], -cropBounds[ 1 ][ 2 ], -cropBounds[ 0 ][ 2 ] );
#endif
          GL_LoadProjectionMatrix( light->projectionMatrix );
        }

        break;
      }

    default:
      break;
  }

  if ( r_logFile->integer )
  {
    // don't just call LogComment, or we will get
    // a call to va() every frame!
    GLimp_LogComment( va( "----- First Shadow Interaction: %i -----\n", (int)( light->firstInteraction - backEnd.viewParms.interactions ) ) );
  }
}

static void RB_SetupLightForLighting( trRefLight_t *light )
{
  GLimp_LogComment( "--- Rendering lighting ---\n" );

  if ( r_logFile->integer )
  {
    // don't just call LogComment, or we will get
    // a call to va() every frame!
    GLimp_LogComment( va( "----- First Light Interaction: %i -----\n", (int)( light->firstInteraction - backEnd.viewParms.interactions ) ) );
  }

  if ( r_hdrRendering->integer )
  {
    R_BindFBO( tr.deferredRenderFBO );
  }
  else
  {
    R_BindNullFBO();
  }

  // set the window clipping
  GL_Viewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
            backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
        backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  // restore camera matrices
  GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );
  GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

  // reset light view and projection matrices
  switch ( light->l.rlType )
  {
    case RL_OMNI:
      {
        MatrixAffineInverse( light->transformMatrix, light->viewMatrix );
        MatrixSetupScale( light->projectionMatrix, 1.0 / light->l.radius[ 0 ], 1.0 / light->l.radius[ 1 ],
                      1.0 / light->l.radius[ 2 ] );
        break;
      }

    case RL_DIRECTIONAL:
      {
        // draw split frustum shadow maps
        if ( r_showShadowMaps->integer )
        {
          int      frustumIndex;
          float    x, y, w, h;
          matrix_t ortho;
          vec4_t   quadVerts[ 4 ];

          // set 2D virtual screen size
          GL_PushMatrix();
          MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                                backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                                backEnd.viewParms.viewportY,
                                backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999 );
          GL_LoadProjectionMatrix( ortho );
          GL_LoadModelViewMatrix( matrixIdentity );

          for ( frustumIndex = 0; frustumIndex <= r_parallelShadowSplits->integer; frustumIndex++ )
          {
            GL_Cull( CT_TWO_SIDED );
            GL_State( GLS_DEPTHTEST_DISABLE );

            gl_debugShadowMapShader->BindProgram();
            gl_debugShadowMapShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

            GL_BindToTMU( 0, tr.sunShadowMapFBOImage[ frustumIndex ] );

            w = 200;
            h = 200;

            x = 205 * frustumIndex;
            y = 70;

            Vector4Set( quadVerts[ 0 ], x, y, 0.f, 1.f );
            Vector4Set(quadVerts[1], x + w, y, 0.f, 1.f);
            Vector4Set(quadVerts[2], x + w, y + h, 0.f, 1.f);
            Vector4Set(quadVerts[3], x, y + h, 0.f, 1.f);

            Tess_InstantQuad( quadVerts );

            {
              int    j;
              vec4_t splitFrustum[ 6 ];
              vec3_t farCorners[ 4 ];
              vec3_t nearCorners[ 4 ];

              GL_Viewport( x, y, w, h );
              GL_Scissor( x, y, w, h );

              GL_PushMatrix();

              gl_genericShader->DisableVertexSkinning();
              gl_genericShader->DisableVertexAnimation();
              gl_genericShader->DisableDeformVertexes();
              gl_genericShader->DisableTCGenEnvironment();

              gl_genericShader->BindProgram();

              // set uniforms
              gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
              gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
              gl_genericShader->SetUniform_Color( colorBlack );

              GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
              GL_Cull( CT_TWO_SIDED );

              // bind u_ColorMap
              GL_BindToTMU( 0, tr.whiteImage );
              gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

              gl_genericShader->SetUniform_ModelViewProjectionMatrix( light->shadowMatrices[ frustumIndex ] );

              tess.multiDrawPrimitives = 0;
              tess.numIndexes = 0;
              tess.numVertexes = 0;

              for ( j = 0; j < 6; j++ )
              {
                VectorCopy( backEnd.viewParms.frustums[ 1 + frustumIndex ][ j ].normal, splitFrustum[ j ] );
                splitFrustum[ j ][ 3 ] = backEnd.viewParms.frustums[ 1 + frustumIndex ][ j ].dist;
              }

              R_CalcFrustumNearCorners( splitFrustum, nearCorners );
              R_CalcFrustumFarCorners( splitFrustum, farCorners );

              // draw outer surfaces
              for ( j = 0; j < 4; j++ )
              {
                Vector4Set(quadVerts[0], nearCorners[j][0], nearCorners[j][1], nearCorners[j][2], 1.f);
                Vector4Set(quadVerts[1], farCorners[j][0], farCorners[j][1], farCorners[j][2], 1.f);
                Vector4Set(quadVerts[2], farCorners[(j + 1) % 4][0], farCorners[(j + 1) % 4][1], farCorners[(j + 1) % 4][2], 1.f);
                Vector4Set(quadVerts[3], nearCorners[(j + 1) % 4][0], nearCorners[(j + 1) % 4][1], nearCorners[(j + 1) % 4][2], 1.f);
                Tess_AddQuadStamp2( quadVerts, colorCyan );
              }

              // draw far cap
              Vector4Set(quadVerts[0], farCorners[3][0], farCorners[3][1], farCorners[3][2], 1.f);
              Vector4Set(quadVerts[1], farCorners[2][0], farCorners[2][1], farCorners[2][2], 1.f);
              Vector4Set(quadVerts[2], farCorners[1][0], farCorners[1][1], farCorners[1][2], 1.f);
              Vector4Set(quadVerts[3], farCorners[0][0], farCorners[0][1], farCorners[0][2], 1.f);
              Tess_AddQuadStamp2( quadVerts, colorBlue );

              // draw near cap
              Vector4Set(quadVerts[0], nearCorners[0][0], nearCorners[0][1], nearCorners[0][2], 1.f);
              Vector4Set(quadVerts[1], nearCorners[1][0], nearCorners[1][1], nearCorners[1][2], 1.f);
              Vector4Set(quadVerts[2], nearCorners[2][0], nearCorners[2][1], nearCorners[2][2], 1.f);
              Vector4Set(quadVerts[3], nearCorners[3][0], nearCorners[3][1], nearCorners[3][2], 1.f);
              Tess_AddQuadStamp2( quadVerts, colorGreen );

              Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
              Tess_DrawElements();

              // draw light volume
              if ( light->isStatic && light->frustumVBO && light->frustumIBO )
              {
                gl_genericShader->SetUniform_ColorModulate( CGEN_CUSTOM_RGB, AGEN_CUSTOM );
                gl_genericShader->SetUniform_Color( colorYellow );

                R_BindVBO( light->frustumVBO );
                R_BindIBO( light->frustumIBO );

                GL_VertexAttribsState( ATTR_POSITION );

                tess.numVertexes = light->frustumVerts;
                tess.numIndexes = light->frustumIndexes;

                Tess_DrawElements();
              }

              tess.multiDrawPrimitives = 0;
              tess.numIndexes = 0;
              tess.numVertexes = 0;

              GL_PopMatrix();

              GL_Viewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                        backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

              GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                    backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );
            }
          }

          GL_PopMatrix();
        }
      }

    default:
      break;
  }
}

static void RB_BlurShadowMap( const trRefLight_t *light, int i )
{
  vec4_t  verts[ 4 ];
  int     index;
  image_t **images;
  FBO_t   **fbos;
  vec2_t  texScale;
  matrix_t ortho;

  if ( light->l.inverseShadows || r_shadows->integer < SHADOWING_VSM16 || !r_softShadowsPP->integer )
  {
    return;
  }

  if ( light->l.rlType == RL_OMNI )
  {
    return;
  }

  fbos = ( light->l.rlType == RL_DIRECTIONAL ) ? tr.sunShadowMapFBO : tr.shadowMapFBO;
  images = ( light->l.rlType == RL_DIRECTIONAL ) ? tr.sunShadowMapFBOImage : tr.shadowMapFBOImage;
  index = ( light->l.rlType == RL_DIRECTIONAL ) ? i : light->shadowLOD;

  Vector4Set(verts[0], 0.f, 0.f, 0.f, 1.f);
  Vector4Set(verts[1], (float)fbos[index]->width, 0.f, 0.f, 1.f);//hypov8 merge: <float>
  Vector4Set(verts[2], verts[1][0], (float)fbos[index]->height, 0.f, 1.f);//hypov8 merge:<float>
  Vector4Set(verts[3], 0.f, verts[2][1], 0.f, 1.f);

  texScale[ 0 ] = 1.0f / fbos[ index ]->width;
  texScale[ 1 ] = 1.0f / fbos[ index ]->height;

  R_BindFBO( fbos[ index ] );
  R_AttachFBOTexture2D( images[ index + MAX_SHADOWMAPS ]->type, images[ index + MAX_SHADOWMAPS ]->texnum, 0 );

  if ( !r_ignoreGLErrors->integer )
  {
    R_CheckFBO( fbos[ index ] );
  }

  // set the window clipping
  GL_Viewport( 0, 0, verts[ 2 ][ 0 ], verts[ 2 ][ 1 ] );
  GL_Scissor( 0, 0, verts[ 2 ][ 0 ], verts[ 2 ][ 1 ] );

  glClear( GL_COLOR_BUFFER_BIT );

  GL_Cull( CT_TWO_SIDED );
  GL_State( GLS_DEPTHTEST_DISABLE );

  GL_BindToTMU( 0, images[ index ] );

  GL_PushMatrix();
  GL_LoadModelViewMatrix( matrixIdentity );

  MatrixOrthogonalProjection( ortho, 0, verts[ 2 ][ 0 ], 0, verts[ 2 ][ 1 ], -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );

  gl_blurXShader->BindProgram();
  gl_blurXShader->SetUniform_DeformMagnitude( 1 );
  gl_blurXShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  gl_blurXShader->SetUniform_TexScale( texScale );

  Tess_InstantQuad( verts );

  R_AttachFBOTexture2D( images[ index ]->type, images[ index ]->texnum, 0 );

  glClear( GL_COLOR_BUFFER_BIT );

  GL_BindToTMU( 0, images[ index + MAX_SHADOWMAPS ] );

  gl_blurYShader->BindProgram();
  gl_blurYShader->SetUniform_DeformMagnitude( 1 );
  gl_blurYShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  gl_blurYShader->SetUniform_TexScale( texScale );

  Tess_InstantQuad( verts );

  GL_PopMatrix();
}

/*
=================
RB_RenderInteractionsShadowMapped
=================
*/

static void RB_RenderInteractionsShadowMapped()
{
  shader_t       *shader, *oldShader;
  trRefEntity_t  *entity, *oldEntity;
  trRefLight_t   *light;
  const interaction_t  *ia;
  const interaction_t  *iaFirst;
  surfaceType_t  *surface;
  qboolean       depthRange, oldDepthRange;
  qboolean       alphaTest, oldAlphaTest;
  deformType_t   deformType, oldDeformType;
  qboolean       shadowClipFound;

  int            startTime = 0, endTime = 0;
  static const matrix_t bias = { 0.5,     0.0, 0.0, 0.0,
                                 0.0,     0.5, 0.0, 0.0,
                                 0.0,     0.0, 0.5, 0.0,
                                 0.5,     0.5, 0.5, 1.0
                        };

  if ( !glConfig2.framebufferObjectAvailable || !glConfig2.textureFloatAvailable )
  {
    RB_RenderInteractions();
    return;
  }

  GLimp_LogComment( "--- RB_RenderInteractionsShadowMapped ---\n" );

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    startTime = ri.Milliseconds();
  }

  // draw everything
  oldEntity = NULL;
  oldShader = NULL;
  oldDepthRange = depthRange = qfalse;
  oldAlphaTest = alphaTest = qfalse;
  oldDeformType = deformType = DEFORM_TYPE_NONE;

  // if we need to clear the FBO color buffers then it should be white
  GL_ClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

  // render each light
  iaFirst = NULL;

  while ( ( iaFirst = IterateLights( iaFirst ) ) )
  {
    backEnd.currentLight = light = iaFirst->light;

    if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicLightOcclusionCulling->integer && !iaFirst->occlusionQuerySamples )
    {
      // skip this light because it failed the occlusion query
      continue;
    }

    if ( light->l.inverseShadows )
    {
      // handle those lights in RB_RenderInteractionsDeferredInverseShadows
      //continue;
    }

    // begin shadowing
    int numMaps;
    switch( light->l.rlType )
    {
      case RL_OMNI:
        numMaps = 6;
        break;
      case RL_DIRECTIONAL:
        numMaps = std::max( r_parallelShadowSplits->integer + 1, 1 );
        break;
      default:
        numMaps = 1;
        break;
    }

    const interaction_t *iaLast = iaFirst;
    for ( int i = 0; i < numMaps; i++ )
    {
      entity = NULL;
      shader = NULL;
      oldEntity = NULL;
      oldShader = NULL;

      if ( light->l.noShadows || light->shadowLOD < 0 )
      {
        if ( r_logFile->integer )
        {
          // don't just call LogComment, or we will get
          // a call to va() every frame!
          GLimp_LogComment( va( "----- Skipping shadowCube side: %i -----\n", i ) );
        }
        continue;
      }

      RB_SetupLightForShadowing( light, i, qfalse );

      shadowClipFound = qfalse;
      for( ia = iaFirst; ia; ia = ia->next )
      {
        iaLast = ia;
        backEnd.currentEntity = entity = ia->entity;
        surface = ia->surface;
        shader = tr.sortedShaders[ ia->shaderNum ];
        alphaTest = shader->alphaTest;
        deformType = GetDeformType( shader );

        if ( entity->e.renderfx & ( RF_NOSHADOW | RF_DEPTHHACK ) )
        {
          continue;
        }

        if ( shader->isSky )
        {
          continue;
        }

        if ( shader->sort > SS_OPAQUE )
        {
          continue;
        }

        if ( shader->noShadows )
        {
          continue;
        }

        if ( (ia->type & IA_SHADOWCLIP) ) {
          shadowClipFound = qtrue;
        }

        if ( !(ia->type & IA_SHADOW) )
        {
          continue;
        }

        if ( light->l.rlType == RL_OMNI && !( ia->cubeSideBits & ( 1 << i ) ) )
        {
          continue;
        }

        switch ( light->l.rlType )
        {
          case RL_OMNI:
          case RL_PROJ:
          case RL_DIRECTIONAL:
            {
              if ( entity == oldEntity && ( alphaTest ? shader == oldShader : alphaTest == oldAlphaTest ) && deformType == oldDeformType )
              {
                if ( r_logFile->integer )
                {
                  // don't just call LogComment, or we will get
                  // a call to va() every frame!
                  GLimp_LogComment( va( "----- Batching Shadow Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
                }

                // fast path, same as previous
                rb_surfaceTable[ *surface ]( surface );
                continue;
              }
              else
              {
                // draw the contents of the last shader batch
                Tess_End();

                if ( r_logFile->integer )
                {
                  // don't just call LogComment, or we will get
                  // a call to va() every frame!
                  GLimp_LogComment( va( "----- Beginning Shadow Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
                }

                // we don't need tangent space calculations here
                Tess_Begin( Tess_StageIteratorShadowFill, NULL, shader, light->shader, qtrue, qfalse, -1, 0 );
              }

              break;
            }

          default:
            break;
        }

        // change the modelview matrix if needed
        if ( entity != oldEntity )
        {
          depthRange = qfalse;

          if ( entity != &tr.worldEntity )
          {
            // set up the transformation matrix
            R_RotateEntityForLight( entity, light, &backEnd.orientation );

            if ( entity->e.renderfx & RF_DEPTHHACK )
            {
              // hack the depth range to prevent view model from poking into walls
              depthRange = qtrue;
            }
          }
          else
          {
            // set up the transformation matrix
            Com_Memset( &backEnd.orientation, 0, sizeof( backEnd.orientation ) );

            backEnd.orientation.axis[ 0 ][ 0 ] = 1;
            backEnd.orientation.axis[ 1 ][ 1 ] = 1;
            backEnd.orientation.axis[ 2 ][ 2 ] = 1;
            VectorCopy( light->l.origin, backEnd.orientation.viewOrigin );

            MatrixIdentity( backEnd.orientation.transformMatrix );
            //MatrixAffineInverse(backEnd.orientation.transformMatrix, backEnd.orientation.viewMatrix);
            MatrixMultiply( light->viewMatrix, backEnd.orientation.transformMatrix, backEnd.orientation.viewMatrix );
            MatrixCopy( backEnd.orientation.viewMatrix, backEnd.orientation.modelViewMatrix );
          }

          GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

          // change depthrange if needed
          if ( oldDepthRange != depthRange )
          {
            if ( depthRange )
            {
              glDepthRange( 0, 0.3 );
            }
            else
            {
              glDepthRange( 0, 1 );
            }

            oldDepthRange = depthRange;
          }

          RB_SetupLightAttenuationForEntity( light, entity );
        }

        switch ( light->l.rlType )
        {
          case RL_OMNI:
          case RL_PROJ:
          case RL_DIRECTIONAL:
            {
              // add the triangles for this surface
              rb_surfaceTable[ *surface ]( surface );
              break;
            }

          default:
            break;
        }
        oldEntity = entity;
        oldShader = shader;
        oldAlphaTest = alphaTest;
        oldDeformType = deformType;
      }

      if ( r_logFile->integer )
      {
        // don't just call LogComment, or we will get
        // a call to va() every frame!
        GLimp_LogComment( va( "----- Last Interaction: %i -----\n", (int)( iaLast - backEnd.viewParms.interactions ) ) );
      }

      Tess_End();

      if( shadowClipFound )
      {
        entity = NULL;
        shader = NULL;
        oldEntity = NULL;
        oldShader = NULL;

        if ( light->l.noShadows || light->shadowLOD < 0 )
        {
          if ( r_logFile->integer )
          {
            // don't just call LogComment, or we will get
            // a call to va() every frame!
            GLimp_LogComment( va( "----- Skipping shadowCube side: %i -----\n", i ) );
          }
          continue;
        }

        RB_SetupLightForShadowing( light, i, qtrue );

        for( ia = iaFirst; ia; ia = ia->next )
        {
          iaLast = ia;
          backEnd.currentEntity = entity = ia->entity;
          surface = ia->surface;
          shader = tr.sortedShaders[ ia->shaderNum ];
          alphaTest = shader->alphaTest;
          deformType = GetDeformType( shader );

          if ( entity->e.renderfx & ( RF_NOSHADOW | RF_DEPTHHACK ) )
          {
            continue;
          }

          if ( shader->isSky )
          {
            continue;
          }

          if ( shader->sort > SS_OPAQUE )
          {
            continue;
          }

          if ( shader->noShadows )
          {
            continue;
          }

          if ( !(ia->type & IA_SHADOWCLIP) )
          {
            continue;
          }

          if ( light->l.rlType == RL_OMNI && !( ia->cubeSideBits & ( 1 << i ) ) )
          {
            continue;
          }

          switch ( light->l.rlType )
          {
            case RL_OMNI:
            case RL_PROJ:
            case RL_DIRECTIONAL:
              {
                if ( entity == oldEntity && ( alphaTest ? shader == oldShader : alphaTest == oldAlphaTest ) && deformType == oldDeformType )
                {
                  if ( r_logFile->integer )
                  {
                    // don't just call LogComment, or we will get
                    // a call to va() every frame!
                    GLimp_LogComment( va( "----- Batching Shadow Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
                  }

                  // fast path, same as previous
                  rb_surfaceTable[ *surface ]( surface );
                  continue;
                }
                else
                {
                  // draw the contents of the last shader batch
                  Tess_End();

                  if ( r_logFile->integer )
                  {
                    // don't just call LogComment, or we will get
                    // a call to va() every frame!
                    GLimp_LogComment( va( "----- Beginning Shadow Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
                  }

                  // we don't need tangent space calculations here
                  Tess_Begin( Tess_StageIteratorShadowFill, NULL, shader, light->shader, qtrue, qfalse, -1, 0 );
                }

                break;
              }

            default:
              break;
          }

          // change the modelview matrix if needed
          if ( entity != oldEntity )
          {
            depthRange = qfalse;

            if ( entity != &tr.worldEntity )
            {
              // set up the transformation matrix
              R_RotateEntityForLight( entity, light, &backEnd.orientation );

              if ( entity->e.renderfx & RF_DEPTHHACK )
              {
                // hack the depth range to prevent view model from poking into walls
                depthRange = qtrue;
              }
            }
            else
            {
              // set up the transformation matrix
              Com_Memset( &backEnd.orientation, 0, sizeof( backEnd.orientation ) );

              backEnd.orientation.axis[ 0 ][ 0 ] = 1;
              backEnd.orientation.axis[ 1 ][ 1 ] = 1;
              backEnd.orientation.axis[ 2 ][ 2 ] = 1;
              VectorCopy( light->l.origin, backEnd.orientation.viewOrigin );

              MatrixIdentity( backEnd.orientation.transformMatrix );
              //MatrixAffineInverse(backEnd.orientation.transformMatrix, backEnd.orientation.viewMatrix);
              MatrixMultiply( light->viewMatrix, backEnd.orientation.transformMatrix, backEnd.orientation.viewMatrix );
              MatrixCopy( backEnd.orientation.viewMatrix, backEnd.orientation.modelViewMatrix );
            }

            GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

            // change depthrange if needed
            if ( oldDepthRange != depthRange )
            {
              if ( depthRange )
              {
                glDepthRange( 0, 0.3 );
              }
              else
              {
                glDepthRange( 0, 1 );
              }

              oldDepthRange = depthRange;
            }

            RB_SetupLightAttenuationForEntity( light, entity );
          }

          switch ( light->l.rlType )
          {
            case RL_OMNI:
            case RL_PROJ:
            case RL_DIRECTIONAL:
              {
                // add the triangles for this surface
                rb_surfaceTable[ *surface ]( surface );
                break;
              }

            default:
              break;
          }
          oldEntity = entity;
          oldShader = shader;
          oldAlphaTest = alphaTest;
          oldDeformType = deformType;
        }

        if ( r_logFile->integer )
        {
          // don't just call LogComment, or we will get
          // a call to va() every frame!
          GLimp_LogComment( va( "----- Last Interaction: %i -----\n", (int)( iaLast - backEnd.viewParms.interactions ) ) );
        }

        Tess_End();
      }

      // set shadow matrix including scale + offset
      if ( light->l.rlType == RL_DIRECTIONAL )
      {
        MatrixCopy( bias, light->shadowMatricesBiased[ i ] );
        MatrixMultiply2( light->shadowMatricesBiased[ i ], light->projectionMatrix );
        MatrixMultiply2( light->shadowMatricesBiased[ i ], light->viewMatrix );

        MatrixMultiply( light->projectionMatrix, light->viewMatrix, light->shadowMatrices[ i ] );
      }

      RB_BlurShadowMap( light, i );
    }

    // begin lighting
    RB_SetupLightForLighting( light );
    entity = NULL;
    shader = NULL;
    oldEntity = NULL;
    oldShader = NULL;
    for ( ia = iaFirst; ia; ia = ia->next )
    {
      iaLast = ia;
      backEnd.currentEntity = entity = ia->entity;
      surface = ia->surface;
      shader = tr.sortedShaders[ ia->shaderNum ];
      alphaTest = shader->alphaTest;
      deformType = GetDeformType( shader );

      if ( !shader->interactLight )
      {
        continue;
      }

      if ( !(ia->type & IA_LIGHT) )
      {
        continue;
      }

      if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicEntityOcclusionCulling->integer && !entity->occlusionQuerySamples )
      {
        continue;
      }

      if ( entity == oldEntity && shader == oldShader )
      {
        if ( r_logFile->integer )
        {
          // don't just call LogComment, or we will get
          // a call to va() every frame!
          GLimp_LogComment( va( "----- Batching Light Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
        }

        // fast path, same as previous
        rb_surfaceTable[ *surface ]( surface );
        continue;
      }
      else
      {
        // draw the contents of the last shader batch
        Tess_End();

        if ( r_logFile->integer )
        {
          // don't just call LogComment, or we will get
          // a call to va() every frame!
          GLimp_LogComment( va( "----- Beginning Light Interaction: %i -----\n", (int)( ia - backEnd.viewParms.interactions ) ) );
        }

        // begin a new batch
        Tess_Begin( Tess_StageIteratorLighting, NULL, shader, light->shader, light->l.inverseShadows, qfalse, -1, 0 );
      }

      // change the modelview matrix if needed
      if ( entity != oldEntity )
      {
        depthRange = qfalse;

        if ( entity != &tr.worldEntity )
        {
          // set up the transformation matrix
          R_RotateEntityForViewParms( entity, &backEnd.viewParms, &backEnd.orientation );

          if ( entity->e.renderfx & RF_DEPTHHACK )
          {
            // hack the depth range to prevent view model from poking into walls
            depthRange = qtrue;
          }
        }
        else
        {
          // set up the transformation matrix
          // transform by the camera placement
          backEnd.orientation = backEnd.viewParms.world;
        }

        GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

        // change depthrange if needed
        if ( oldDepthRange != depthRange )
        {
          if ( depthRange )
          {
            glDepthRange( 0, 0.3 );
          }
          else
          {
            glDepthRange( 0, 1 );
          }

          oldDepthRange = depthRange;
        }

        RB_SetupLightAttenuationForEntity( light, entity );
      }

      // add the triangles for this surface
      rb_surfaceTable[ *surface ]( surface );
      oldEntity = entity;
      oldShader = shader;
      oldAlphaTest = alphaTest;
      oldDeformType = deformType;
    }

    if ( r_logFile->integer )
    {
      // don't just call LogComment, or we will get
      // a call to va() every frame!
      GLimp_LogComment( va( "----- Last Interaction: %i -----\n", (int)( iaLast - backEnd.viewParms.interactions ) ) );
    }

    Tess_End();
#ifdef VOLUMETRIC_LIGHTING
    // draw the light volume if needed
    if ( light->shader->volumetricLight )
    {
      Render_lightVolume( ia );
    }
#endif
  }

  // draw the contents of the last shader batch
  Tess_End();

  // go back to the world modelview matrix
  GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

  if ( depthRange )
  {
    glDepthRange( 0, 1 );
  }

  // reset scissor clamping
  GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
              backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

  // reset clear color
  GL_ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

  GL_CheckErrors();

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    endTime = ri.Milliseconds();
    backEnd.pc.c_forwardLightingTime = endTime - startTime;
  }
}

#ifdef EXPERIMENTAL
void RB_RenderScreenSpaceAmbientOcclusion( qboolean deferred )
{
#if 0
//  int             i;
//  vec3_t          viewOrigin;
//  static vec3_t   jitter[32];
//  static qboolean jitterInit = qfalse;
//  matrix_t        projectMatrix;
  matrix_t ortho;

  GLimp_LogComment( "--- RB_RenderScreenSpaceAmbientOcclusion ---\n" );

  if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
  {
    return;
  }

  if ( !r_screenSpaceAmbientOcclusion->integer )
  {
    return;
  }

  // enable shader, set arrays
  gl_screenSpaceAmbientOcclusionShader->BindProgram();

  GL_State( GLS_DEPTHTEST_DISABLE );  // | GLS_DEPTHMASK_TRUE);
  GL_Cull( CT_TWO_SIDED );

  glVertexAttrib4fv( ATTR_INDEX_COLOR, colorWhite );

  // set uniforms

  /*
     VectorCopy(backEnd.viewParms.orientation.origin, viewOrigin); // in world space

     if(!jitterInit)
     {
     for(i = 0; i < 32; i++)
     {
     float *jit = &jitter[i][0];

     float rad = crandom() * 1024.0f; // FIXME radius;
     float a = crandom() * M_PI * 2;
     float b = crandom() * M_PI * 2;

     jit[0] = rad * sin(a) * cos(b);
     jit[1] = rad * sin(a) * sin(b);
     jit[2] = rad * cos(a);
     }

     jitterInit = qtrue;
     }


     MatrixCopy(backEnd.viewParms.projectionMatrix, projectMatrix);
     MatrixInverse(projectMatrix);

     glUniform3f(tr.screenSpaceAmbientOcclusionShader.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
     glUniform3fv(tr.screenSpaceAmbientOcclusionShader.u_SSAOJitter, 32, &jitter[0][0]);
     glUniform1f(tr.screenSpaceAmbientOcclusionShader.u_SSAORadius, r_screenSpaceAmbientOcclusionRadius->value);

     glUniformMatrix4fv(tr.screenSpaceAmbientOcclusionShader.u_UnprojectMatrix, 1, GL_FALSE, backEnd.viewParms.unprojectionMatrix);
     glUniformMatrix4fv(tr.screenSpaceAmbientOcclusionShader.u_ProjectMatrix, 1, GL_FALSE, projectMatrix);
   */

  // capture current color buffer for u_CurrentMap
  GL_SelectTexture( 0 );
  GL_Bind( tr.currentRenderImage );
  glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight );

  // bind u_DepthMap
  if ( deferred )
  {
    GL_BindToTMU( 1, tr.deferredPositionFBOImage );
  }
  else
  {
    GL_SelectTexture( 1 );
    GL_Bind( tr.depthRenderImage );
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.depthRenderImage->uploadWidth, tr.depthRenderImage->uploadHeight );
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  gl_screenSpaceAmbientOcclusionShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  // draw viewport
  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
#endif
}

#endif
#ifdef EXPERIMENTAL
void RB_RenderDepthOfField()
{
  matrix_t ortho;

  GLimp_LogComment( "--- RB_RenderDepthOfField ---\n" );

  if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
  {
    return;
  }

  if ( !r_depthOfField->integer )
  {
    return;
  }

  // enable shader, set arrays
  gl_depthOfFieldShader->BindProgram();

  GL_State( GLS_DEPTHTEST_DISABLE );  // | GLS_DEPTHMASK_TRUE);
  GL_Cull( CT_TWO_SIDED );

  glVertexAttrib4fv( ATTR_INDEX_COLOR, colorWhite );

  // set uniforms

  // capture current color buffer for u_CurrentMap
  if ( r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable )
  {
    GL_BindToTMU( 0, tr.deferredRenderFBOImage );
  }
  else
  {
    GL_SelectTexture( 0 );
    GL_Bind( tr.currentRenderImage );
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight );
  }

  // bind u_DepthMap
  if ( r_deferredShading->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable &&
       glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 4 )
  {
    GL_BindToTMU( 1, tr.depthRenderImage );
  }
  else if ( r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable )
  {
    GL_BindToTMU( 1, tr.depthRenderImage );
  }
  else
  {
    // depth texture is not bound to a FBO
    GL_SelectTexture( 1 );
    GL_Bind( tr.depthRenderImage );
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.depthRenderImage->uploadWidth, tr.depthRenderImage->uploadHeight );
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  gl_depthOfFieldShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  // draw viewport
  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
}

#endif

void RB_RenderGlobalFog()
{
  vec4_t		fogDepthVector; //unused, uninitialized use
  vec4_t		fogColor; //unused, uninitialized use
  matrix_t	ortho;

  GLimp_LogComment( "--- RB_RenderGlobalFog ---\n" );

  if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
    return;
  if ( r_noFog->integer )
    return;

#if /*!defined( COMPAT_KPQ3 ) &&*/ defined( COMPAT_ET ) //hypov8 fog: //
  if ( !tr.world || tr.world->globalFog < 0 )
  {
    return;
  }
#else
  if ( r_forceFog->value <= 0 && VectorLength( tr.fogColor ) <= 0 )
  {
    return;
  }

  if ( r_forceFog->value <= 0 && tr.fogDensity <= 0 )
  {
    return;
  }

#endif

  GL_Cull( CT_TWO_SIDED );

  gl_fogGlobalShader->BindProgram();

  // go back to the world modelview matrix
  backEnd.orientation = backEnd.viewParms.world;

  gl_fogGlobalShader->SetUniform_ViewOrigin( backEnd.viewParms.orientation.origin );  // world space

#if /*!defined( COMPAT_KPQ3 ) &&*/ defined( COMPAT_ET ) //hypov8 fog: //
  {
    fog_t	*fog;
    vec3_t   local;
    vec4_t   fogDistanceVector,

    fog = &tr.world->fogs[ tr.world->globalFog ];

    if ( r_logFile->integer )
    {
      GLimp_LogComment( va( "--- RB_RenderGlobalFog( fogNum = %i, originalBrushNumber = %i ) ---\n", tr.world->globalFog, fog->originalBrushNumber ) );
    }

    GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

    // all fogging distance is based on world Z units
    VectorSubtract( backEnd.orientation.origin, backEnd.viewParms.orientation.origin, local );
    fogDistanceVector[ 0 ] = -backEnd.orientation.modelViewMatrix[ 2 ];
    fogDistanceVector[ 1 ] = -backEnd.orientation.modelViewMatrix[ 6 ];
    fogDistanceVector[ 2 ] = -backEnd.orientation.modelViewMatrix[ 10 ];
    fogDistanceVector[ 3 ] = DotProduct( local, backEnd.viewParms.orientation.axis[ 0 ] );

    // scale the fog vectors based on the fog's thickness
    fogDistanceVector[ 0 ] *= fog->tcScale;
    fogDistanceVector[ 1 ] *= fog->tcScale;
    fogDistanceVector[ 2 ] *= fog->tcScale;
    fogDistanceVector[ 3 ] *= fog->tcScale;

    gl_fogGlobalShader->SetUniform_FogDistanceVector( fogDistanceVector );
    gl_fogGlobalShader->SetUniform_Color( fog->color );
  }
#else
  GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA | GLS_DSTBLEND_SRC_ALPHA );

  if ( r_forceFog->value )
  {
    Vector4Set(fogDepthVector, r_forceFog->value, 0.f, 0.f, 0.f);
    VectorCopy( colorMdGrey, fogColor );
  }
  else
  {
    Vector4Set(fogDepthVector, tr.fogDensity, 0.f, 0.f, 0.f);
    VectorCopy( tr.fogColor, fogColor );
  }

  gl_fogGlobalShader->SetUniform_FogDepthVector( fogDepthVector );
  gl_fogGlobalShader->SetUniform_Color( fogColor );
#endif

  gl_fogGlobalShader->SetUniform_ViewMatrix( backEnd.viewParms.world.viewMatrix );
  gl_fogGlobalShader->SetUniform_UnprojectMatrix( backEnd.viewParms.unprojectionMatrix );

  // bind u_ColorMap
  GL_BindToTMU( 0, tr.fogImage );

  // bind u_DepthMap
  GL_SelectTexture( 1 );

 // if ( HDR_ENABLED() )
  {
  //  GL_Bind( tr.depthRenderImage );
  }
  //else
  {
    // depth texture is not bound to a FBO
    //if (!backEnd.depthRenderImageValid) 
		{ //hypov8 merge: .4
      GL_Bind(tr.depthRenderImage);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.depthRenderImage->uploadWidth, tr.depthRenderImage->uploadHeight);
      //backEnd.depthRenderImageValid = true;
    }
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  gl_fogGlobalShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  // draw viewport
  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
}

void RB_RenderBloom()
{
  int      i, j, flip = 0;
  matrix_t ortho;

  GLimp_LogComment( "--- RB_RenderBloom ---\n" );

  if ( ( backEnd.refdef.rdflags & ( RDF_NOWORLDMODEL | RDF_NOBLOOM ) ) || !r_bloom->integer || backEnd.viewParms.isPortal || !glConfig2.framebufferObjectAvailable )
  {
    return;
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  // FIXME
  //if(glConfig.hardwareType != GLHW_ATI && glConfig.hardwareType != GLHW_ATI_DX10)
  {
    GL_State( GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    GL_PushMatrix();
    GL_LoadModelViewMatrix( matrixIdentity );

#if 1
    MatrixOrthogonalProjection( ortho, 0, tr.contrastRenderFBO->width, 0, tr.contrastRenderFBO->height, -99999, 99999 );
    GL_LoadProjectionMatrix( ortho );
#endif

    if ( HDR_ENABLED() )
    {
      gl_toneMappingShader->EnableMacro_BRIGHTPASS_FILTER();
      gl_toneMappingShader->BindProgram();

      gl_toneMappingShader->SetUniform_HDRKey( backEnd.hdrKey );
      gl_toneMappingShader->SetUniform_HDRAverageLuminance( backEnd.hdrAverageLuminance );
      gl_toneMappingShader->SetUniform_HDRMaxLuminance( backEnd.hdrMaxLuminance );

      gl_toneMappingShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      GL_BindToTMU( 0, tr.downScaleFBOImage_quarter );
    }
    else
    {
      // render contrast downscaled to 1/4th of the screen
      gl_contrastShader->BindProgram();

      gl_contrastShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      GL_SelectTexture( 0 );
      //GL_Bind(tr.downScaleFBOImage_quarter);
      GL_Bind( tr.currentRenderImage );
      glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth,
                           tr.currentRenderImage->uploadHeight );
    }

    GL_PopMatrix(); // special 1/4th of the screen contrastRenderFBO ortho

    R_BindFBO( tr.contrastRenderFBO );
    GL_ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    // draw viewport
    Tess_InstantQuad( backEnd.viewParms.viewportVerts );

    // render bloom in multiple passes
    GL_BindToTMU( 0, tr.contrastRenderFBOImage );
    for ( i = 0; i < 2; i++ )
    {
      for ( j = 0; j < r_bloomPasses->integer; j++ )
      {
        vec2_t texScale;

        texScale[ 0 ] = 1.0f / tr.bloomRenderFBO[ flip ]->width;
        texScale[ 1 ] = 1.0f / tr.bloomRenderFBO[ flip ]->height;

        R_BindFBO( tr.bloomRenderFBO[ flip ] );

        GL_ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        GL_State( GLS_DEPTHTEST_DISABLE );

        GL_PushMatrix();
        GL_LoadModelViewMatrix( matrixIdentity );

        MatrixOrthogonalProjection( ortho, 0, tr.bloomRenderFBO[ 0 ]->width, 0, tr.bloomRenderFBO[ 0 ]->height, -99999, 99999 );
        GL_LoadProjectionMatrix( ortho );

        if ( i == 0 )
        {
          gl_blurXShader->BindProgram();

          gl_blurXShader->SetUniform_DeformMagnitude( r_bloomBlur->value );
          gl_blurXShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
          gl_blurXShader->SetUniform_TexScale( texScale );
        }
        else
        {
          gl_blurYShader->BindProgram();

          gl_blurYShader->SetUniform_DeformMagnitude( r_bloomBlur->value );
          gl_blurYShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
          gl_blurYShader->SetUniform_TexScale( texScale );
        }

        GL_PopMatrix();

        Tess_InstantQuad( backEnd.viewParms.viewportVerts );
        GL_BindToTMU( 0, tr.bloomRenderFBOImage[ flip ] );
        flip ^= 1;
      }
    }

    // add offscreen processed bloom to screen
    if ( HDR_ENABLED() )
    {
      R_BindFBO( tr.deferredRenderFBO );

      gl_screenShader->BindProgram();
      GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
      glVertexAttrib4fv( ATTR_INDEX_COLOR, colorWhite );

      gl_screenShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
    }
    else
    {
      R_BindNullFBO();

      gl_screenShader->BindProgram();
      GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
      glVertexAttrib4fv( ATTR_INDEX_COLOR, colorWhite );

      gl_screenShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
    }

    Tess_InstantQuad( backEnd.viewParms.viewportVerts );
  }

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
}

void RB_RenderMotionBlur( void )
{
  static vec4_t quadVerts[4] = {
    { -1.0f, -1.0f, 0.0f, 1.0f },
    {  1.0f, -1.0f, 0.0f, 1.0f },
    {  1.0f,  1.0f, 0.0f, 1.0f },
    { -1.0f,  1.0f, 0.0f, 1.0f }
  };

  GLimp_LogComment( "--- RB_RenderMotionBlur ---\n" );

  if ( ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) || backEnd.viewParms.isPortal || !glConfig2.framebufferObjectAvailable )
  {
    return;
  }

  GL_State( GLS_DEPTHTEST_DISABLE );
  GL_Cull( CT_TWO_SIDED );

  GL_SelectTexture( 0 );
  GL_Bind( tr.currentRenderImage );
  glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0,
           tr.currentRenderImage->uploadWidth,
           tr.currentRenderImage->uploadHeight );

  //if( !backEnd.depthRenderImageValid ) //hypov8 merge: .4
  {
    GL_Bind( tr.depthRenderImage );
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0,
           tr.depthRenderImage->uploadWidth,
           tr.depthRenderImage->uploadHeight );
    //backEnd.depthRenderImageValid = qtrue;
  }

  gl_motionblurShader->BindProgram();
  gl_motionblurShader->SetUniform_blurVec(tr.refdef.blurVec);

  GL_BindToTMU( 0, tr.currentRenderImage );
  GL_BindToTMU( 1, tr.depthRenderImage );

  // draw quad
  Tess_InstantQuad( quadVerts );

  GL_CheckErrors();
}

void RB_RenderRotoscope( void )
{
#if 0 //!defined(GLSL_COMPILE_STARTUP_ONLY)
  matrix_t ortho;

  GLimp_LogComment( "--- RB_CameraPostFX ---\n" );

  if ( ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) || !r_rotoscope->integer || backEnd.viewParms.isPortal )
  {
    return;
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  GL_State( GLS_DEPTHTEST_DISABLE );
  GL_Cull( CT_TWO_SIDED );

  // enable shader, set arrays
  GL_BindProgram( &tr.rotoscopeShader );

  GLSL_SetUniform_ModelViewProjectionMatrix( &tr.rotoscopeShader, glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  glUniform1f( tr.rotoscopeShader.u_BlurMagnitude, r_bloomBlur->value );

  GL_SelectTexture( 0 );
  GL_Bind( tr.currentRenderImage );
  glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight );

  // draw viewport
  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
#endif
}

void RB_FXAA( void )
{
  static vec4_t quadVerts[4] = {
    { -1.0f, -1.0f, 0.0f, 1.0f },
    {  1.0f, -1.0f, 0.0f, 1.0f },
    {  1.0f,  1.0f, 0.0f, 1.0f },
    { -1.0f,  1.0f, 0.0f, 1.0f }
  };

  GLimp_LogComment( "--- RB_FXAA ---\n" );

  if ( ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) ||
       backEnd.viewParms.isPortal )
  {
    return;
  }

  if ( !r_FXAA->integer || !gl_fxaaShader )
  {
    return;
  }

  GL_State( GLS_DEPTHTEST_DISABLE );
  GL_Cull( CT_TWO_SIDED );

  // copy the framebuffer in a texture
  // TODO: it is pretty inefficient
  GL_SelectTexture( 0 );
  GL_Bind( tr.currentRenderImage );
  glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth,
             tr.currentRenderImage->uploadHeight );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // set the shader parameters
  gl_fxaaShader->BindProgram();

  R_BindNullFBO();

  Tess_InstantQuad( quadVerts );

  GL_CheckErrors();
}

void RB_CameraPostFX( void )
{
  matrix_t ortho;
  matrix_t grain;

  GLimp_LogComment( "--- RB_CameraPostFX ---\n" );

  if ( ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) ||
       backEnd.viewParms.isPortal )
  {
    return;
  }

  if ( !r_cameraPostFX->integer )
  {
    return;
  }

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  GL_State( GLS_DEPTHTEST_DISABLE );
  GL_Cull( CT_TWO_SIDED );

  // enable shader, set arrays
  gl_cameraEffectsShader->BindProgram();

  gl_cameraEffectsShader->SetUniform_ColorModulate( backEnd.viewParms.gradingWeights );
  gl_cameraEffectsShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  //glUniform1f(tr.cameraEffectsShader.u_BlurMagnitude, r_bloomBlur->value);

  MatrixIdentity( grain );

  MatrixMultiplyScale( grain, r_cameraFilmGrainScale->value, r_cameraFilmGrainScale->value, 0 );
  MatrixMultiplyTranslation( grain, backEnd.refdef.floatTime * 10, backEnd.refdef.floatTime * 10, 0 );

  MatrixMultiplyTranslation( grain, 0.5, 0.5, 0.0 );
  MatrixMultiplyZRotation( grain, backEnd.refdef.floatTime * ( random() * 7 ) );
  MatrixMultiplyTranslation( grain, -0.5, -0.5, 0.0 );

  gl_cameraEffectsShader->SetUniform_ColorTextureMatrix( grain );

  // bind u_CurrentMap
  GL_SelectTexture( 0 );
  GL_Bind( tr.occlusionRenderFBOImage );

  /*
  if(glConfig.framebufferObjectAvailable && glConfig.textureFloatAvailable)
  {
          // copy depth of the main context to deferredRenderFBO
          glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
          glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, tr.occlusionRenderFBO->frameBuffer);
          glBlitFramebufferEXT(0, 0, glConfig.vidWidth, glConfig.vidHeight,
                                                     0, 0, glConfig.vidWidth, glConfig.vidHeight,
                                                     GL_COLOR_BUFFER_BIT,
                                                     GL_NEAREST);
  }
  else
  */
  {
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.occlusionRenderFBOImage->uploadWidth, tr.occlusionRenderFBOImage->uploadHeight );
  }

  // bind u_GrainMap
  if ( r_cameraFilmGrain->integer && tr.grainImage )
  {
    GL_BindToTMU( 1, tr.grainImage );
  }
  else
  {
    GL_BindToTMU( 1, tr.blackImage );
  }

  // bind u_VignetteMap
  if ( r_cameraVignette->integer && tr.vignetteImage )
  {
    GL_BindToTMU( 2, tr.vignetteImage );
  }
  else
  {
    GL_BindToTMU( 2, tr.whiteImage );
  }

  GL_BindToTMU( 3, tr.colorGradeImage );

  // draw viewport
  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  // go back to 3D
  GL_PopMatrix();

  GL_CheckErrors();
}

static void RB_CalculateAdaptation()
{
  int          i;
  static float image[ 64 * 64 * 4 ];
  float        curTime;
  float        deltaTime;
  float        luminance;
  float        avgLuminance;
  float        maxLuminance;
  double       sum;
  const vec3_t LUMINANCE_VECTOR = { 0.2125f, 0.7154f, 0.0721f };
  vec4_t       color;
  float        newAdaptation;
  float        newMaximum;

  curTime = ri.Milliseconds() / 1000.0f;

  // calculate the average scene luminance
  R_BindFBO( tr.downScaleFBO_64x64 );

  // read back the contents
//	glFinish();
  glReadPixels( 0, 0, 64, 64, GL_RGBA, GL_FLOAT, image );

  sum = 0.0f;
  maxLuminance = 0.0f;

  for ( i = 0; i < ( 64 * 64 * 4 ); i += 4 )
  {
    color[ 0 ] = image[ i + 0 ];
    color[ 1 ] = image[ i + 1 ];
    color[ 2 ] = image[ i + 2 ];
    color[ 3 ] = image[ i + 3 ];

    luminance = DotProduct( color, LUMINANCE_VECTOR ) + 0.0001f;

    if ( luminance > maxLuminance )
    {
      maxLuminance = luminance;
    }

    sum += log( luminance );
  }

  sum /= 64 * 64;
  avgLuminance = exp( sum );

  // the user's adapted luminance level is simulated by closing the gap between
  // adapted luminance and current luminance by 2% every frame, based on a
  // 30 fps rate. This is not an accurate model of human adaptation, which can
  // take longer than half an hour.
  if ( backEnd.hdrTime > curTime )
  {
    backEnd.hdrTime = curTime;
  }

  deltaTime = curTime - backEnd.hdrTime;

  //if(r_hdrMaxLuminance->value)
  {
    backEnd.hdrAverageLuminance = Maths::clamp( backEnd.hdrAverageLuminance, r_hdrMinLuminance->value, r_hdrMaxLuminance->value );
    avgLuminance = Maths::clamp( avgLuminance, r_hdrMinLuminance->value, r_hdrMaxLuminance->value );

    backEnd.hdrMaxLuminance = Maths::clamp( backEnd.hdrMaxLuminance, r_hdrMinLuminance->value, r_hdrMaxLuminance->value );
    maxLuminance = Maths::clamp( maxLuminance, r_hdrMinLuminance->value, r_hdrMaxLuminance->value );
  }

  newAdaptation = backEnd.hdrAverageLuminance + ( avgLuminance - backEnd.hdrAverageLuminance ) * ( 1.0f - powf( 0.98f, 30.0f * deltaTime ) );
  newMaximum = backEnd.hdrMaxLuminance + ( maxLuminance - backEnd.hdrMaxLuminance ) * ( 1.0f - powf( 0.98f, 30.0f * deltaTime ) );

  if ( !Q_isnan( newAdaptation ) && !Q_isnan( newMaximum ) )
  {
#if 1
    backEnd.hdrAverageLuminance = newAdaptation;
    backEnd.hdrMaxLuminance = newMaximum;
#else
    backEnd.hdrAverageLuminance = avgLuminance;
    backEnd.hdrMaxLuminance = maxLuminance;
#endif
  }

  backEnd.hdrTime = curTime;

  // calculate HDR image key
  if ( r_hdrKey->value <= 0 )
  {
    // calculation from: Perceptual Effects in Real-time Tone Mapping - Krawczyk et al.
    backEnd.hdrKey = 1.03 - 2.0 / ( 2.0 + log10f( backEnd.hdrAverageLuminance + 1.0f ) );
  }
  else
  {
    backEnd.hdrKey = r_hdrKey->value;
  }

  if ( r_hdrDebug->integer )
  {
    ri.Printf( PRINT_ALL, "HDR luminance avg = %f, max = %f, key = %f\n", backEnd.hdrAverageLuminance, backEnd.hdrMaxLuminance, backEnd.hdrKey );
  }

  GL_CheckErrors();
}

void RB_RenderDeferredHDRResultToFrameBuffer()
{
  matrix_t ortho;

  GLimp_LogComment( "--- RB_RenderDeferredHDRResultToFrameBuffer ---\n" );

  if ( !r_hdrRendering->integer || !glConfig2.framebufferObjectAvailable || !glConfig2.textureFloatAvailable )
  {
    return;
  }

  GL_CheckErrors();

  R_BindNullFBO();

  // bind u_CurrentMap
  GL_BindToTMU( 0, tr.deferredRenderFBOImage );

  GL_State( GLS_DEPTHTEST_DISABLE );
  GL_Cull( CT_TWO_SIDED );

  // set uniforms

  // set 2D virtual screen size
  GL_PushMatrix();
  MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                              backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                              backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
                              -99999, 99999 );
  GL_LoadProjectionMatrix( ortho );
  GL_LoadModelViewMatrix( matrixIdentity );

  if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
  {
    gl_screenShader->BindProgram();

    glVertexAttrib4fv( ATTR_INDEX_COLOR, colorWhite );

    gl_screenShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  }
  else
  {
    gl_toneMappingShader->DisableMacro_BRIGHTPASS_FILTER();
    gl_toneMappingShader->BindProgram();

    gl_toneMappingShader->SetUniform_HDRKey( backEnd.hdrKey );
    gl_toneMappingShader->SetUniform_HDRAverageLuminance( backEnd.hdrAverageLuminance );
    gl_toneMappingShader->SetUniform_HDRMaxLuminance( backEnd.hdrMaxLuminance );

    gl_toneMappingShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
  }

  GL_CheckErrors();

  Tess_InstantQuad( backEnd.viewParms.viewportVerts );

  GL_PopMatrix();
}

// ================================================================================================
//
// LIGHTS OCCLUSION CULLING
//
// ================================================================================================

static void RenderLightOcclusionVolume( trRefLight_t *light )
{
  GL_CheckErrors();

#if 1

  if ( light->isStatic && light->frustumVBO && light->frustumIBO )
  {
    // render in world space
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    R_BindVBO( light->frustumVBO );
    R_BindIBO( light->frustumIBO );

    GL_VertexAttribsState( ATTR_POSITION );

    tess.numVertexes = light->frustumVerts;
    tess.numIndexes = light->frustumIndexes;

    Tess_DrawElements();
  }
  else
#endif
  {
    // render in light space
    R_RotateLightForViewParms( light, &backEnd.viewParms, &backEnd.orientation );
    GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    tess.multiDrawPrimitives = 0;
    tess.numIndexes = 0;
    tess.numVertexes = 0;

    R_TessLight( light, NULL );

    Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
    Tess_DrawElements();
  }

  tess.multiDrawPrimitives = 0;
  tess.numIndexes = 0;
  tess.numVertexes = 0;

  GL_CheckErrors();
}

static void IssueLightOcclusionQuery( link_t *queue, trRefLight_t *light, qboolean resetMultiQueryLink )
{
  GLimp_LogComment( "--- IssueLightOcclusionQuery ---\n" );

  //ri.Printf(PRINT_ALL, "--- IssueOcclusionQuery(%i) ---\n", node - tr.world->nodes);

  if ( tr.numUsedOcclusionQueryObjects < ( MAX_OCCLUSION_QUERIES - 1 ) )
  {
    light->occlusionQueryObject = tr.occlusionQueryObjects[ tr.numUsedOcclusionQueryObjects++ ];
  }
  else
  {
    light->occlusionQueryObject = 0;
  }

  EnQueue( queue, light );

  // tell GetOcclusionQueryResult that this is not a multi query
  if ( resetMultiQueryLink )
  {
    QueueInit( &light->multiQuery );
  }

  if ( light->occlusionQueryObject > 0 )
  {
    GL_CheckErrors();

    // begin the occlusion query
    glBeginQuery( GL_SAMPLES_PASSED, light->occlusionQueryObject );

    GL_CheckErrors();

    RenderLightOcclusionVolume( light );

    // end the query
    glEndQuery( GL_SAMPLES_PASSED );

#if 1

    if ( !glIsQuery( light->occlusionQueryObject ) )
    {
      ri.Error( ERR_FATAL, "IssueLightOcclusionQuery: light %i has no occlusion query object in slot %i: %i", ( int )( light - tr.world->lights ), backEnd.viewParms.viewCount, light->occlusionQueryObject );
    }

#endif

    //light->occlusionQueryNumbers[backEnd.viewParms.viewCount] = backEnd.pc.c_occlusionQueries;
    backEnd.pc.c_occlusionQueries++;
  }

  GL_CheckErrors();
}

static void IssueLightMultiOcclusionQueries( link_t *multiQueue, link_t *individualQueue )
{
  trRefLight_t *light;
  trRefLight_t *multiQueryLight;
  link_t       *l;

  GLimp_LogComment( "--- IssueLightMultiOcclusionQueries ---\n" );

#if 0
  ri.Printf( PRINT_ALL, "IssueLightMultiOcclusionQueries(" );

  for ( l = multiQueue->prev; l != multiQueue; l = l->prev )
  {
    light = ( trRefLight_t * ) l->data;

    ri.Printf( PRINT_ALL, "%i, ", light - backEnd.refdef.lights );
  }

  ri.Printf( PRINT_ALL, ")\n" );
#endif

  if ( QueueEmpty( multiQueue ) )
  {
    return;
  }

  multiQueryLight = ( trRefLight_t * ) QueueFront( multiQueue )->data;

  if ( tr.numUsedOcclusionQueryObjects < ( MAX_OCCLUSION_QUERIES - 1 ) )
  {
    multiQueryLight->occlusionQueryObject = tr.occlusionQueryObjects[ tr.numUsedOcclusionQueryObjects++ ];
  }
  else
  {
    multiQueryLight->occlusionQueryObject = 0;
  }

  if ( multiQueryLight->occlusionQueryObject > 0 )
  {
    // begin the occlusion query
    GL_CheckErrors();

    glBeginQuery( GL_SAMPLES_PASSED, multiQueryLight->occlusionQueryObject );

    GL_CheckErrors();

    //ri.Printf(PRINT_ALL, "rendering nodes:[");
    for ( l = multiQueue->prev; l != multiQueue; l = l->prev )
    {
      light = ( trRefLight_t * ) l->data;

      //ri.Printf(PRINT_ALL, "%i, ", light - backEnd.refdef.lights);

      RenderLightOcclusionVolume( light );
    }

    //ri.Printf(PRINT_ALL, "]\n");

    backEnd.pc.c_occlusionQueries++;
    backEnd.pc.c_occlusionQueriesMulti++;

    // end the query
    glEndQuery( GL_SAMPLES_PASSED );

    GL_CheckErrors();

#if 0

    if ( !glIsQuery( multiQueryNode->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
    {
      ri.Error( ERR_FATAL, "IssueMultiOcclusionQueries: node %i has no occlusion query object in slot %i: %i", multiQueryNode - tr.world->nodes, backEnd.viewParms.viewCount, multiQueryNode->occlusionQueryObjects[ backEnd.viewParms.viewCount ] );
    }

#endif
  }

  // move queue to node->multiQuery queue
  QueueInit( &multiQueryLight->multiQuery );
  DeQueue( multiQueue );

  while ( !QueueEmpty( multiQueue ) )
  {
    light = ( trRefLight_t * ) DeQueue( multiQueue );
    EnQueue( &multiQueryLight->multiQuery, light );
  }

  EnQueue( individualQueue, multiQueryLight );

  //ri.Printf(PRINT_ALL, "--- IssueMultiOcclusionQueries end ---\n");
}

static qboolean LightOcclusionResultAvailable( trRefLight_t *light )
{
  GLint available;

  if ( light->occlusionQueryObject > 0 )
  {
    glFinish();

    available = 0;
    //if(glIsQuery(light->occlusionQueryObjects[backEnd.viewParms.viewCount]))
    {
      glGetQueryObjectiv( light->occlusionQueryObject, GL_QUERY_RESULT_AVAILABLE, &available );
      GL_CheckErrors();
    }

    return ( qboolean ) available;
  }

  return qtrue;
}

static void GetLightOcclusionQueryResult( trRefLight_t *light )
{
  link_t *l, *sentinel;
  int    ocSamples;
  GLint  available;

  GLimp_LogComment( "--- GetLightOcclusionQueryResult ---\n" );

  if ( light->occlusionQueryObject > 0 )
  {
    glFinish();

#if 0

    if ( !glIsQuery( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
    {
      ri.Error( ERR_FATAL, "GetOcclusionQueryResult: node %i has no occlusion query object in slot %i: %i", node - tr.world->nodes, backEnd.viewParms.viewCount, node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] );
    }

#endif

    available = 0;

    while ( !available )
    {
      //if(glIsQuery(node->occlusionQueryObjects[backEnd.viewParms.viewCount]))
      {
        glGetQueryObjectiv( light->occlusionQueryObject, GL_QUERY_RESULT_AVAILABLE, &available );
        //GL_CheckErrors();
      }
    }

    backEnd.pc.c_occlusionQueriesAvailable++;

    glGetQueryObjectiv( light->occlusionQueryObject, GL_QUERY_RESULT, &ocSamples );

    //ri.Printf(PRINT_ALL, "GetOcclusionQueryResult(%i): available = %i, samples = %i\n", node - tr.world->nodes, available, ocSamples);

    GL_CheckErrors();
  }
  else
  {
    ocSamples = 1;
  }

  light->occlusionQuerySamples = ocSamples;

  // copy result to all nodes that were linked to this multi query node
  sentinel = &light->multiQuery;

  for ( l = sentinel->prev; l != sentinel; l = l->prev )
  {
    light = ( trRefLight_t * ) l->data;

    light->occlusionQuerySamples = ocSamples;
  }
}

static int LightCompare( const void *a, const void *b )
{
  trRefLight_t *l1, *l2;
  float        d1, d2;

  l1 = ( trRefLight_t * ) * ( void ** ) a;
  l2 = ( trRefLight_t * ) * ( void ** ) b;

  d1 = DistanceSquared( backEnd.viewParms.orientation.origin, l1->l.origin );
  d2 = DistanceSquared( backEnd.viewParms.orientation.origin, l2->l.origin );

  if ( d1 < d2 )
  {
    return -1;
  }

  if ( d1 > d2 )
  {
    return 1;
  }

  return 0;
}

void RB_RenderLightOcclusionQueries()
{
  GLimp_LogComment( "--- RB_RenderLightOcclusionQueries ---\n" );

  if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicLightOcclusionCulling->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
  {
    int           i;
    interaction_t *ia;
    interaction_t *iaFirst;
    trRefLight_t  *light, *multiQueryLight;
    link_t        occlusionQueryQueue;
    link_t        invisibleQueue;
    growList_t    invisibleList;
    int           startTime = 0, endTime = 0;

    glVertexAttrib4f( ATTR_INDEX_COLOR, 1.0f, 0.0f, 0.0f, 0.05f );

    if ( r_speeds->integer == RSPEEDS_OCCLUSION_QUERIES )
    {
      glFinish();
      startTime = ri.Milliseconds();
    }

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_Cull( CT_TWO_SIDED );

    GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );
    gl_genericShader->SetRequiredVertexPointers();

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    // don't write to the color buffer or depth buffer
    if ( r_showOcclusionQueries->integer )
    {
      GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
    }
    else
    {
      GL_State( GLS_COLORMASK_BITS );
    }

    tr.numUsedOcclusionQueryObjects = 0;
    QueueInit( &occlusionQueryQueue );
    QueueInit( &invisibleQueue );
    Com_InitGrowList( &invisibleList, 1000 );

    iaFirst = NULL;

    // add each light to the potentially invisible list
    while ( ( iaFirst = IterateLights( iaFirst ) ) )
    {
      backEnd.currentLight = light = iaFirst->light;

      for ( ia = iaFirst; ia; ia = ia->next )
      {
        ia->occlusionQuerySamples = 1;
      }

      if ( !iaFirst->noOcclusionQueries )
      {
        Com_AddToGrowList( &invisibleList, light );
      }
    }

    // sort lights by distance
    qsort( invisibleList.elements, invisibleList.currentElements, sizeof( void * ), LightCompare );

    for ( i = 0; i < invisibleList.currentElements; i++ )
    {
      light = ( trRefLight_t * ) Com_GrowListElement( &invisibleList, i );

      EnQueue( &invisibleQueue, light );

      if ( ( invisibleList.currentElements - i ) <= 100 )
      {
        if ( QueueSize( &invisibleQueue ) >= 10 )
        {
          IssueLightMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );
        }
      }
      else
      {
        if ( QueueSize( &invisibleQueue ) >= 50 )
        {
          IssueLightMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );
        }
      }
    }

    Com_DestroyGrowList( &invisibleList );

    if ( !QueueEmpty( &invisibleQueue ) )
    {
      // remaining previously invisible node queries
      IssueLightMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );

      //ri.Printf(PRINT_ALL, "occlusionQueryQueue.empty() = %i\n", QueueEmpty(&occlusionQueryQueue));
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

    while ( !QueueEmpty( &occlusionQueryQueue ) )
    {
      if ( LightOcclusionResultAvailable( ( trRefLight_t * ) QueueFront( &occlusionQueryQueue )->data ) )
      {
        light = ( trRefLight_t * ) DeQueue( &occlusionQueryQueue );

        // wait if result not available
        GetLightOcclusionQueryResult( light );

        if ( ( signed ) light->occlusionQuerySamples > r_chcVisibilityThreshold->integer )
        {
          // if a query of multiple previously invisible objects became visible, we need to
          // test all the individual objects ...
          if ( !QueueEmpty( &light->multiQuery ) )
          {
            multiQueryLight = light;

            IssueLightOcclusionQuery( &occlusionQueryQueue, multiQueryLight, qfalse );

            while ( !QueueEmpty( &multiQueryLight->multiQuery ) )
            {
              light = ( trRefLight_t * ) DeQueue( &multiQueryLight->multiQuery );

              IssueLightOcclusionQuery( &occlusionQueryQueue, light, qtrue );
            }
          }
        }
        else
        {
          if ( !QueueEmpty( &light->multiQuery ) )
          {
            backEnd.pc.c_occlusionQueriesLightsCulled++;

            multiQueryLight = light;

            while ( !QueueEmpty( &multiQueryLight->multiQuery ) )
            {
              light = ( trRefLight_t * ) DeQueue( &multiQueryLight->multiQuery );

              backEnd.pc.c_occlusionQueriesLightsCulled++;
              backEnd.pc.c_occlusionQueriesSaved++;
            }
          }
          else
          {
            backEnd.pc.c_occlusionQueriesLightsCulled++;
          }
        }
      }
    }

    if ( r_speeds->integer == RSPEEDS_OCCLUSION_QUERIES )
    {
      glFinish();
      endTime = ri.Milliseconds();
      backEnd.pc.c_occlusionQueriesResponseTime = endTime - startTime;

      startTime = ri.Milliseconds();
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

    // reenable writes to depth and color buffers
    GL_State( GLS_DEPTHMASK_TRUE );

    // copy result to all other interactions that belong to the same light
    iaFirst = NULL;
    while( ( iaFirst = IterateLights( iaFirst ) ) )
    {
      backEnd.currentLight = light = iaFirst->light;
      interaction_t *ia = iaFirst;

      for ( ia = iaFirst; ia; ia = ia->next )
      {
        if ( !ia->noOcclusionQueries )
        {
          ia->occlusionQuerySamples = (int)light->occlusionQuerySamples > r_chcVisibilityThreshold->integer;
        }
        else
        {
          ia->occlusionQuerySamples = 1;
        }

        if ( !ia->occlusionQuerySamples )
        {
          backEnd.pc.c_occlusionQueriesInteractionsCulled++;
        }
      }
    }

    if ( r_speeds->integer == RSPEEDS_OCCLUSION_QUERIES )
    {
      glFinish();
      endTime = ri.Milliseconds();
      backEnd.pc.c_occlusionQueriesFetchTime = endTime - startTime;
    }
  }

  GL_CheckErrors();
}

// ================================================================================================
//
// ENTITY OCCLUSION CULLING
//
// ================================================================================================

static void RenderEntityOcclusionVolume( trRefEntity_t *entity )
{
  GL_CheckErrors();

#if 0
  // render in entity space
  R_RotateEntityForViewParms( entity, &backEnd.viewParms, &backEnd.orientation );
  GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
  gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  tess.multiDrawPrimitives = 0;
  tess.numIndexes = 0;
  tess.numVertexes = 0;

  Tess_AddCube( vec3_origin, entity->localBounds[ 0 ], entity->localBounds[ 1 ], colorBlue );

  Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
  Tess_DrawElements();

  tess.multiDrawPrimitives = 0;
  tess.numIndexes = 0;
  tess.numVertexes = 0;
#else

  vec3_t   boundsCenter;
  vec3_t   boundsSize;
  matrix_t rot;
  axis_t   axis;

#if 0
  VectorSubtract( entity->localBounds[ 1 ], entity->localBounds[ 0 ], boundsSize );
#else
  boundsSize[ 0 ] = Q_fabs( entity->localBounds[ 0 ][ 0 ] ) + Q_fabs( entity->localBounds[ 1 ][ 0 ] );
  boundsSize[ 1 ] = Q_fabs( entity->localBounds[ 0 ][ 1 ] ) + Q_fabs( entity->localBounds[ 1 ][ 1 ] );
  boundsSize[ 2 ] = Q_fabs( entity->localBounds[ 0 ][ 2 ] ) + Q_fabs( entity->localBounds[ 1 ][ 2 ] );
#endif

  VectorScale( entity->e.axis[ 0 ], boundsSize[ 0 ] * 0.5f, axis[ 0 ] );
  VectorScale( entity->e.axis[ 1 ], boundsSize[ 1 ] * 0.5f, axis[ 1 ] );
  VectorScale( entity->e.axis[ 2 ], boundsSize[ 2 ] * 0.5f, axis[ 2 ] );

  VectorAdd( entity->localBounds[ 0 ], entity->localBounds[ 1 ], boundsCenter );
  VectorScale( boundsCenter, 0.5f, boundsCenter );

  MatrixFromVectorsFLU( rot, entity->e.axis[ 0 ], entity->e.axis[ 1 ], entity->e.axis[ 2 ] );
  MatrixTransformNormal2( rot, boundsCenter );

  VectorAdd( entity->e.origin, boundsCenter, boundsCenter );

  MatrixSetupTransformFromVectorsFLU( backEnd.orientation.transformMatrix, axis[ 0 ], axis[ 1 ], axis[ 2 ], boundsCenter );

  MatrixAffineInverse( backEnd.orientation.transformMatrix, backEnd.orientation.viewMatrix );
  MatrixMultiply( backEnd.viewParms.world.viewMatrix, backEnd.orientation.transformMatrix, backEnd.orientation.modelViewMatrix );

  GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
  gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  R_BindVBO( tr.unitCubeVBO );
  R_BindIBO( tr.unitCubeIBO );

  GL_VertexAttribsState( ATTR_POSITION );

  tess.multiDrawPrimitives = 0;
  tess.numVertexes = tr.unitCubeVBO->vertexesNum;
  tess.numIndexes = tr.unitCubeIBO->indexesNum;

  Tess_DrawElements();

  tess.multiDrawPrimitives = 0;
  tess.numIndexes = 0;
  tess.numVertexes = 0;

#endif

  GL_CheckErrors();
}

static void IssueEntityOcclusionQuery( link_t *queue, trRefEntity_t *entity, qboolean resetMultiQueryLink )
{
  GLimp_LogComment( "--- IssueEntityOcclusionQuery ---\n" );

  //ri.Printf(PRINT_ALL, "--- IssueEntityOcclusionQuery(%i) ---\n", light - backEnd.refdef.lights);

  if ( tr.numUsedOcclusionQueryObjects < ( MAX_OCCLUSION_QUERIES - 1 ) )
  {
    entity->occlusionQueryObject = tr.occlusionQueryObjects[ tr.numUsedOcclusionQueryObjects++ ];
  }
  else
  {
    entity->occlusionQueryObject = 0;
  }

  EnQueue( queue, entity );

  // tell GetOcclusionQueryResult that this is not a multi query
  if ( resetMultiQueryLink )
  {
    QueueInit( &entity->multiQuery );
  }

  if ( entity->occlusionQueryObject > 0 )
  {
    GL_CheckErrors();

    // begin the occlusion query
    glBeginQuery( GL_SAMPLES_PASSED, entity->occlusionQueryObject );

    GL_CheckErrors();

    RenderEntityOcclusionVolume( entity );

    // end the query
    glEndQuery( GL_SAMPLES_PASSED );

#if 0

    if ( !glIsQuery( entity->occlusionQueryObject ) )
    {
      ri.Error( ERR_FATAL, "IssueOcclusionQuery: entity %i has no occlusion query object in slot %i: %i", light - tr.world->lights, backEnd.viewParms.viewCount, light->occlusionQueryObject );
    }

#endif
    backEnd.pc.c_occlusionQueries++;
  }

  GL_CheckErrors();
}

static void IssueEntityMultiOcclusionQueries( link_t *multiQueue, link_t *individualQueue )
{
  trRefEntity_t *entity;
  trRefEntity_t *multiQueryEntity;
  link_t        *l;

  GLimp_LogComment( "--- IssueEntityMultiOcclusionQueries ---\n" );

#if 0
  ri.Printf( PRINT_ALL, "IssueEntityMultiOcclusionQueries(" );

  for ( l = multiQueue->prev; l != multiQueue; l = l->prev )
  {
    light = ( trRefEntity_t * ) l->data;

    ri.Printf( PRINT_ALL, "%i, ", light - backEnd.refdef.entities );
  }

  ri.Printf( PRINT_ALL, ")\n" );
#endif

  if ( QueueEmpty( multiQueue ) )
  {
    return;
  }

  multiQueryEntity = ( trRefEntity_t * ) QueueFront( multiQueue )->data;

  if ( tr.numUsedOcclusionQueryObjects < ( MAX_OCCLUSION_QUERIES - 1 ) )
  {
    multiQueryEntity->occlusionQueryObject = tr.occlusionQueryObjects[ tr.numUsedOcclusionQueryObjects++ ];
  }
  else
  {
    multiQueryEntity->occlusionQueryObject = 0;
  }

  if ( multiQueryEntity->occlusionQueryObject > 0 )
  {
    // begin the occlusion query
    GL_CheckErrors();

    glBeginQuery( GL_SAMPLES_PASSED, multiQueryEntity->occlusionQueryObject );

    GL_CheckErrors();

    //ri.Printf(PRINT_ALL, "rendering nodes:[");
    for ( l = multiQueue->prev; l != multiQueue; l = l->prev )
    {
      entity = ( trRefEntity_t * ) l->data;

      //ri.Printf(PRINT_ALL, "%i, ", light - backEnd.refdef.lights);

      RenderEntityOcclusionVolume( entity );
    }

    //ri.Printf(PRINT_ALL, "]\n");

    backEnd.pc.c_occlusionQueries++;
    backEnd.pc.c_occlusionQueriesMulti++;

    // end the query
    glEndQuery( GL_SAMPLES_PASSED );

    GL_CheckErrors();

#if 0

    if ( !glIsQuery( multiQueryNode->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
    {
      ri.Error( ERR_FATAL, "IssueEntityMultiOcclusionQueries: node %i has no occlusion query object in slot %i: %i", multiQueryNode - tr.world->nodes, backEnd.viewParms.viewCount, multiQueryNode->occlusionQueryObjects[ backEnd.viewParms.viewCount ] );
    }

#endif
  }

  // move queue to node->multiQuery queue
  QueueInit( &multiQueryEntity->multiQuery );
  DeQueue( multiQueue );

  while ( !QueueEmpty( multiQueue ) )
  {
    entity = ( trRefEntity_t * ) DeQueue( multiQueue );
    EnQueue( &multiQueryEntity->multiQuery, entity );
  }

  EnQueue( individualQueue, multiQueryEntity );

  //ri.Printf(PRINT_ALL, "--- IssueMultiOcclusionQueries end ---\n");
}

static qboolean EntityOcclusionResultAvailable( trRefEntity_t *entity )
{
  GLint available;

  if ( entity->occlusionQueryObject > 0 )
  {
    glFinish();

    available = 0;
    //if(glIsQuery(light->occlusionQueryObjects[backEnd.viewParms.viewCount]))
    {
      glGetQueryObjectiv( entity->occlusionQueryObject, GL_QUERY_RESULT_AVAILABLE, &available );
      GL_CheckErrors();
    }

    return ( qboolean ) available;
  }

  return qtrue;
}

static void GetEntityOcclusionQueryResult( trRefEntity_t *entity )
{
  link_t *l, *sentinel;
  int    ocSamples;
  GLint  available;

  GLimp_LogComment( "--- GetEntityOcclusionQueryResult ---\n" );

  if ( entity->occlusionQueryObject > 0 )
  {
    glFinish();

    available = 0;

    while ( !available )
    {
      //if(glIsQuery(node->occlusionQueryObjects[backEnd.viewParms.viewCount]))
      {
        glGetQueryObjectiv( entity->occlusionQueryObject, GL_QUERY_RESULT_AVAILABLE, &available );
        //GL_CheckErrors();
      }
    }

    backEnd.pc.c_occlusionQueriesAvailable++;

    glGetQueryObjectiv( entity->occlusionQueryObject, GL_QUERY_RESULT, &ocSamples );

    //ri.Printf(PRINT_ALL, "GetOcclusionQueryResult(%i): available = %i, samples = %i\n", node - tr.world->nodes, available, ocSamples);

    GL_CheckErrors();
  }
  else
  {
    ocSamples = 1;
  }

  entity->occlusionQuerySamples = ocSamples;

  // copy result to all nodes that were linked to this multi query node
  sentinel = &entity->multiQuery;

  for ( l = sentinel->prev; l != sentinel; l = l->prev )
  {
    entity = ( trRefEntity_t * ) l->data;

    entity->occlusionQuerySamples = ocSamples;
  }
}

static int EntityCompare( const void *a, const void *b )
{
  trRefEntity_t *e1, *e2;
  float         d1, d2;

  e1 = ( trRefEntity_t * ) * ( void ** ) a;
  e2 = ( trRefEntity_t * ) * ( void ** ) b;

  d1 = DistanceSquared( backEnd.viewParms.orientation.origin, e1->e.origin );
  d2 = DistanceSquared( backEnd.viewParms.orientation.origin, e2->e.origin );

  if ( d1 < d2 )
  {
    return -1;
  }

  if ( d1 > d2 )
  {
    return 1;
  }

  return 0;
}

void RB_RenderEntityOcclusionQueries()
{
  GLimp_LogComment( "--- RB_RenderEntityOcclusionQueries ---\n" );

  if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
  {
    int           i;
    trRefEntity_t *entity, *multiQueryEntity;
    link_t        occlusionQueryQueue;
    link_t        invisibleQueue;
    growList_t    invisibleList;
    int           startTime = 0, endTime = 0;

    glVertexAttrib4f( ATTR_INDEX_COLOR, 1.0f, 0.0f, 0.0f, 0.05f );

    if ( r_speeds->integer == RSPEEDS_OCCLUSION_QUERIES )
    {
      glFinish();
      startTime = ri.Milliseconds();
    }

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_Cull( CT_TWO_SIDED );

    GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_CONST, AGEN_CONST );
    gl_genericShader->SetUniform_Color( colorBlue );
    gl_genericShader->SetRequiredVertexPointers();

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    // don't write to the color buffer or depth buffer
    if ( r_showOcclusionQueries->integer )
    {
      GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
    }
    else
    {
      GL_State( GLS_COLORMASK_BITS );
    }

    tr.numUsedOcclusionQueryObjects = 0;
    QueueInit( &occlusionQueryQueue );
    QueueInit( &invisibleQueue );
    Com_InitGrowList( &invisibleList, 1000 );

    // loop trough all entities and render the entity OBB
    for ( i = 0, entity = backEnd.refdef.entities; i < backEnd.refdef.numEntities; i++, entity++ )
    {
      if ( ( entity->e.renderfx & RF_THIRD_PERSON ) && !backEnd.viewParms.isPortal )
      {
        continue;
      }

      if ( entity->cull == CULL_OUT )
      {
        continue;
      }

      backEnd.currentEntity = entity;

      entity->occlusionQuerySamples = 1;
      entity->noOcclusionQueries = qfalse;

      // check if the entity volume clips against the near plane
      if ( BoxOnPlaneSide( entity->worldBounds[ 0 ], entity->worldBounds[ 1 ], &backEnd.viewParms.frustums[ 0 ][ FRUSTUM_NEAR ] ) == 3 )
      {
        entity->noOcclusionQueries = qtrue;
      }
      else
      {
        Com_AddToGrowList( &invisibleList, entity );
      }
    }

    // sort entities by distance
    qsort( invisibleList.elements, invisibleList.currentElements, sizeof( void * ), EntityCompare );

    for ( i = 0; i < invisibleList.currentElements; i++ )
    {
      entity = ( trRefEntity_t * ) Com_GrowListElement( &invisibleList, i );

      EnQueue( &invisibleQueue, entity );

      if ( ( invisibleList.currentElements - i ) <= 100 )
      {
        if ( QueueSize( &invisibleQueue ) >= 10 )
        {
          IssueEntityMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );
        }
      }
      else
      {
        if ( QueueSize( &invisibleQueue ) >= 50 )
        {
          IssueEntityMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );
        }
      }
    }

    Com_DestroyGrowList( &invisibleList );

    if ( !QueueEmpty( &invisibleQueue ) )
    {
      // remaining previously invisible node queries
      IssueEntityMultiOcclusionQueries( &invisibleQueue, &occlusionQueryQueue );

      //ri.Printf(PRINT_ALL, "occlusionQueryQueue.empty() = %i\n", QueueEmpty(&occlusionQueryQueue));
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

    while ( !QueueEmpty( &occlusionQueryQueue ) )
    {
      if ( EntityOcclusionResultAvailable( ( trRefEntity_t * ) QueueFront( &occlusionQueryQueue )->data ) )
      {
        entity = ( trRefEntity_t * ) DeQueue( &occlusionQueryQueue );

        // wait if result not available
        GetEntityOcclusionQueryResult( entity );

        if ( ( signed ) entity->occlusionQuerySamples > r_chcVisibilityThreshold->integer )
        {
          // if a query of multiple previously invisible objects became visible, we need to
          // test all the individual objects ...
          if ( !QueueEmpty( &entity->multiQuery ) )
          {
            multiQueryEntity = entity;

            IssueEntityOcclusionQuery( &occlusionQueryQueue, multiQueryEntity, qfalse );

            while ( !QueueEmpty( &multiQueryEntity->multiQuery ) )
            {
              entity = ( trRefEntity_t * ) DeQueue( &multiQueryEntity->multiQuery );

              IssueEntityOcclusionQuery( &occlusionQueryQueue, entity, qtrue );
            }
          }
        }
        else
        {
          if ( !QueueEmpty( &entity->multiQuery ) )
          {
            backEnd.pc.c_occlusionQueriesEntitiesCulled++;

            multiQueryEntity = entity;

            while ( !QueueEmpty( &multiQueryEntity->multiQuery ) )
            {
              entity = ( trRefEntity_t * ) DeQueue( &multiQueryEntity->multiQuery );

              backEnd.pc.c_occlusionQueriesEntitiesCulled++;
              backEnd.pc.c_occlusionQueriesSaved++;
            }
          }
          else
          {
            backEnd.pc.c_occlusionQueriesEntitiesCulled++;
          }
        }
      }
    }

    if ( r_speeds->integer == RSPEEDS_OCCLUSION_QUERIES )
    {
      glFinish();
      endTime = ri.Milliseconds();
      backEnd.pc.c_occlusionQueriesResponseTime = endTime - startTime;

      startTime = ri.Milliseconds();
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

    // reenable writes to depth and color buffers
    GL_State( GLS_DEPTHMASK_TRUE );
  }

  GL_CheckErrors();
}

// ================================================================================================
//
// BSP OCCLUSION CULLING
//
// ================================================================================================

#if 0
void RB_RenderBspOcclusionQueries()
{
  GLimp_LogComment( "--- RB_RenderBspOcclusionQueries ---\n" );

  if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicBspOcclusionCulling->integer )
  {
    //int             j;
    bspNode_t *node;
    link_t    *l, *next, *sentinel;

    gl_genericShader->BindProgram();

    GL_Cull( CT_TWO_SIDED );

    GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );

    // set uniforms
    gl_genericShader->SetTCGenEnvironment( qfalse );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetVertexSkinning( qfalse );

    gl_genericShader->SetUniform_AlphaTest( 0 );

    // set up the transformation matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    // don't write to the color buffer or depth buffer
    GL_State( GLS_COLORMASK_BITS );

    sentinel = &tr.occlusionQueryList;

    for ( l = sentinel->next; l != sentinel; l = next )
    {
      next = l->next;
      node = ( bspNode_t * ) l->data;

      // begin the occlusion query
      glBeginQuery( GL_SAMPLES_PASSED, node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] );

      R_BindVBO( node->volumeVBO );
      R_BindIBO( node->volumeIBO );

      GL_VertexAttribsState( ATTR_POSITION );

      tess.numVertexes = node->volumeVerts;
      tess.numIndexes = node->volumeIndexes;

      Tess_DrawElements();

      // end the query
      // don't read back immediately so that we give the query time to be ready
      glEndQuery( GL_SAMPLES_PASSED );

#if 0

      if ( !glIsQuery( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
      {
        ri.Error( ERR_FATAL, "node %i has no occlusion query object in slot %i: %i", j, 0, node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] );
      }

#endif

      backEnd.pc.c_occlusionQueries++;

      tess.multiDrawPrimitives = 0;
      tess.numIndexes = 0;
      tess.numVertexes = 0;
    }
  }

  GL_CheckErrors();
}

void RB_CollectBspOcclusionQueries()
{
  GLimp_LogComment( "--- RB_CollectBspOcclusionQueries ---\n" );

  if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA && r_dynamicBspOcclusionCulling->integer )
  {
    //int             j;
    bspNode_t *node;
    link_t    *l, *next, *sentinel;

    int       ocCount;
    int       avCount;
    GLint     available;

    glFinish();

    ocCount = 0;
    sentinel = &tr.occlusionQueryList;

    for ( l = sentinel->next; l != sentinel; l = l->next )
    {
      node = ( bspNode_t * ) l->data;

      if ( glIsQuery( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
      {
        ocCount++;
      }
    }

    //ri.Printf(PRINT_ALL, "waiting for %i queries...\n", ocCount);

    avCount = 0;

    do
    {
      for ( l = sentinel->next; l != sentinel; l = l->next )
      {
        node = ( bspNode_t * ) l->data;

        if ( node->issueOcclusionQuery )
        {
          available = 0;

          if ( glIsQuery( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
          {
            glGetQueryObjectiv( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ], GL_QUERY_RESULT_AVAILABLE, &available );
            GL_CheckErrors();
          }

          if ( available )
          {
            node->issueOcclusionQuery = qfalse;
            avCount++;

            //if(//avCount % oc)

            //ri.Printf(PRINT_ALL, "%i queries...\n", avCount);
          }
        }
      }
    }
    while ( avCount < ocCount );

    for ( l = sentinel->next; l != sentinel; l = l->next )
    {
      node = ( bspNode_t * ) l->data;

      available = 0;

      if ( glIsQuery( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ] ) )
      {
        glGetQueryObjectiv( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ], GL_QUERY_RESULT_AVAILABLE, &available );
        GL_CheckErrors();
      }

      if ( available )
      {
        backEnd.pc.c_occlusionQueriesAvailable++;

        // get the object and store it in the occlusion bits for the light
        glGetQueryObjectiv( node->occlusionQueryObjects[ backEnd.viewParms.viewCount ], GL_QUERY_RESULT, &node->occlusionQuerySamples[ backEnd.viewParms.viewCount ] );

        if ( node->occlusionQuerySamples[ backEnd.viewParms.viewCount ] <= 0 )
        {
          backEnd.pc.c_occlusionQueriesLeafsCulled++;
        }
      }
      else
      {
        node->occlusionQuerySamples[ backEnd.viewParms.viewCount ] = 1;
      }

      GL_CheckErrors();
    }

    //ri.Printf(PRINT_ALL, "done\n");
  }
}

#endif

static void RB_RenderDebugUtils()
{
  GLimp_LogComment( "--- RB_RenderDebugUtils ---\n" );

  if ( r_showLightTransforms->integer || r_showShadowLod->integer )
  {
    const interaction_t *ia;
    trRefLight_t  *light;
    vec3_t        forward, left, up;
    vec4_t        lightColor; //unused

    static const vec3_t minSize = { -2, -2, -2 };
    static const vec3_t maxSize = { 2,  2,  2 };

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    //GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_CUSTOM_RGB, AGEN_CUSTOM );

    gl_genericShader->SetRequiredVertexPointers();

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    ia = NULL;
    while ( ( ia = IterateLights( ia ) ) )
    {
      backEnd.currentLight = light = ia->light;

      if ( r_showShadowLod->integer )
      {
        if ( light->shadowLOD == 0 )
        {
          Vector4Copy( colorRed, lightColor );
        }
        else if ( light->shadowLOD == 1 )
        {
          Vector4Copy( colorGreen, lightColor );
        }
        else if ( light->shadowLOD == 2 )
        {
          Vector4Copy( colorBlue, lightColor );
        }
        else if ( light->shadowLOD == 3 )
        {
          Vector4Copy( colorYellow, lightColor );
        }
        else if ( light->shadowLOD == 4 )
        {
          Vector4Copy( colorMagenta, lightColor );
        }
        else if ( light->shadowLOD == 5 )
        {
          Vector4Copy( colorCyan, lightColor );
        }
        else
        {
          Vector4Copy( colorMdGrey, lightColor );
        }
      }
      else if ( r_dynamicLightOcclusionCulling->integer )
      {
        if ( !ia->occlusionQuerySamples )
        {
          Vector4Copy( colorRed, lightColor );
        }
        else
        {
          Vector4Copy( colorGreen, lightColor );
        }
      }
      else
      {
        //Vector4Copy(g_color_table[iaCount % 8], lightColor);
        Vector4Copy( colorBlue, lightColor );
      }

      lightColor[ 3 ] = 0.2f;

      gl_genericShader->SetUniform_Color( lightColor );

      MatrixToVectorsFLU( matrixIdentity, forward, left, up );
      VectorMA( vec3_origin, 16, forward, forward );
      VectorMA( vec3_origin, 16, left, left );
      VectorMA( vec3_origin, 16, up, up );

      Tess_Begin( Tess_StageIteratorDebug, NULL, NULL, NULL, qtrue, qtrue, 0, 0 );

      if ( light->isStatic && light->frustumVBO && light->frustumIBO )
      {
        // go back to the world modelview matrix
        backEnd.orientation = backEnd.viewParms.world;
        GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
        gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
        gl_genericShader->SetUniform_ModelMatrix( backEnd.orientation.transformMatrix );

        R_BindVBO( light->frustumVBO );
        R_BindIBO( light->frustumIBO );

        GL_VertexAttribsState( ATTR_POSITION );

        tess.numVertexes = light->frustumVerts;
        tess.numIndexes = light->frustumIndexes;
      }
      else
      {

        // set up the transformation matrix
        R_RotateLightForViewParms( light, &backEnd.viewParms, &backEnd.orientation );

        GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
        gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
        gl_genericShader->SetUniform_ModelMatrix( backEnd.orientation.transformMatrix );

        R_TessLight( light, lightColor );

        switch ( light->l.rlType )
        {
          case RL_OMNI:
          case RL_DIRECTIONAL:
            {
              if ( !VectorCompare( light->l.center, vec3_origin ) )
              {
                Tess_AddCube( light->l.center, minSize, maxSize, colorYellow );
              }
              break;
            }

          case RL_PROJ:
            {
              // draw light_target
              Tess_AddCube( light->l.projTarget, minSize, maxSize, colorRed );
              Tess_AddCube( light->l.projRight, minSize, maxSize, colorGreen );
              Tess_AddCube( light->l.projUp, minSize, maxSize, colorBlue );

              if ( !VectorCompare( light->l.projStart, vec3_origin ) )
              {
                Tess_AddCube( light->l.projStart, minSize, maxSize, colorYellow );
              }

              if ( !VectorCompare( light->l.projEnd, vec3_origin ) )
              {
                Tess_AddCube( light->l.projEnd, minSize, maxSize, colorMagenta );
              }
              break;
            }

          default:
            break;
        }
      }

      Tess_End();
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showLightInteractions->integer )
  {
    int           i;
    int           cubeSides;
    interaction_t *ia;
    int           iaCount;
    trRefLight_t  *light;
    trRefEntity_t *entity;
    surfaceType_t *surface;
    vec4_t        lightColor;

    static const vec3_t mins = { -1, -1, -1 };
    static const vec3_t maxs = { 1, 1, 1 };

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    for ( iaCount = 0, ia = &backEnd.viewParms.interactions[ 0 ]; iaCount < backEnd.viewParms.numInteractions; ia++, iaCount++ )
    {
      backEnd.currentEntity = entity = ia->entity;
      light = ia->light;
      surface = ia->surface;

      if ( entity != &tr.worldEntity )
      {
        // set up the transformation matrix
        R_RotateEntityForViewParms( backEnd.currentEntity, &backEnd.viewParms, &backEnd.orientation );
      }
      else
      {
        backEnd.orientation = backEnd.viewParms.world;
      }

      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
      gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      if ( r_shadows->integer >= SHADOWING_ESM16 && light->l.rlType == RL_OMNI )
      {
#if 0
        Vector4Copy( colorMdGrey, lightColor );

        if ( ia->cubeSideBits & CUBESIDE_PX )
        {
          Vector4Copy( colorBlack, lightColor );
        }

        if ( ia->cubeSideBits & CUBESIDE_PY )
        {
          Vector4Copy( colorRed, lightColor );
        }

        if ( ia->cubeSideBits & CUBESIDE_PZ )
        {
          Vector4Copy( colorGreen, lightColor );
        }

        if ( ia->cubeSideBits & CUBESIDE_NX )
        {
          Vector4Copy( colorYellow, lightColor );
        }

        if ( ia->cubeSideBits & CUBESIDE_NY )
        {
          Vector4Copy( colorBlue, lightColor );
        }

        if ( ia->cubeSideBits & CUBESIDE_NZ )
        {
          Vector4Copy( colorCyan, lightColor );
        }

        if ( ia->cubeSideBits == CUBESIDE_CLIPALL )
        {
          Vector4Copy( colorMagenta, lightColor );
        }

#else
        // count how many cube sides are in use for this interaction
        cubeSides = 0;

        for ( i = 0; i < 6; i++ )
        {
          if ( ia->cubeSideBits & ( 1 << i ) )
          {
            cubeSides++;
          }
        }

        Vector4Copy( g_color_table[ cubeSides ], lightColor );
#endif
      }
      else
      {
        Vector4Copy( colorMdGrey, lightColor );
      }

      lightColor[ 0 ] *= 0.5f;
      lightColor[ 1 ] *= 0.5f;
      lightColor[ 2 ] *= 0.5f;
      //lightColor[3] *= 0.2f;

      Vector4Copy( colorWhite, lightColor );

      tess.numVertexes = 0;
      tess.numIndexes = 0;
      tess.multiDrawPrimitives = 0;

      if ( *surface == SF_FACE || *surface == SF_GRID || *surface == SF_TRIANGLES )
      {
        srfGeneric_t *gen;

        gen = ( srfGeneric_t * ) surface;

        if ( *surface == SF_FACE )
        {
          Vector4Copy( colorMdGrey, lightColor );
        }
        else if ( *surface == SF_GRID )
        {
          Vector4Copy( colorCyan, lightColor );
        }
        else if ( *surface == SF_TRIANGLES )
        {
          Vector4Copy( colorMagenta, lightColor );
        }
        else
        {
          Vector4Copy( colorMdGrey, lightColor );
        }

        Tess_AddCube( vec3_origin, gen->bounds[ 0 ], gen->bounds[ 1 ], lightColor );

        Tess_AddCube( gen->origin, mins, maxs, colorWhite );
      }
      else if ( *surface == SF_VBO_MESH )
      {
        srfVBOMesh_t *srf = ( srfVBOMesh_t * ) surface;
        Tess_AddCube( vec3_origin, srf->bounds[ 0 ], srf->bounds[ 1 ], lightColor );
      }
      else if ( *surface == SF_MDV )
      {
        Tess_AddCube( vec3_origin, entity->localBounds[ 0 ], entity->localBounds[ 1 ], lightColor );
      }

      Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
      Tess_DrawElements();

      tess.multiDrawPrimitives = 0;
      tess.numIndexes = 0;
      tess.numVertexes = 0;
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showEntityTransforms->integer )
  {
    trRefEntity_t *ent;
    int           i;
    static const vec3_t mins = { -1, -1, -1 };
    static const vec3_t maxs = { 1, 1, 1 };

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    ent = backEnd.refdef.entities;

    for ( i = 0; i < backEnd.refdef.numEntities; i++, ent++ )
    {
      if ( ( ent->e.renderfx & RF_THIRD_PERSON ) && !backEnd.viewParms.isPortal )
      {
        continue;
      }

      if ( ent->cull == CULL_OUT )
      {
        continue;
      }

      // set up the transformation matrix
      R_RotateEntityForViewParms( ent, &backEnd.viewParms, &backEnd.orientation );
      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
      gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      //R_DebugAxis(vec3_origin, matrixIdentity);
      //R_DebugBoundingBox(vec3_origin, ent->localBounds[0], ent->localBounds[1], colorMagenta);

      tess.multiDrawPrimitives = 0;
      tess.numIndexes = 0;
      tess.numVertexes = 0;

      if ( r_dynamicEntityOcclusionCulling->integer )
      {
        if ( !ent->occlusionQuerySamples )
        {
          Tess_AddCube( vec3_origin, ent->localBounds[ 0 ], ent->localBounds[ 1 ], colorRed );
        }
        else
        {
          Tess_AddCube( vec3_origin, ent->localBounds[ 0 ], ent->localBounds[ 1 ], colorGreen );
        }
      }
      else
      {
        Tess_AddCube( vec3_origin, ent->localBounds[ 0 ], ent->localBounds[ 1 ], colorBlue );
      }

      Tess_AddCube( vec3_origin, mins, maxs, colorWhite );

      Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
      Tess_DrawElements();

      tess.multiDrawPrimitives = 0;
      tess.numIndexes = 0;
      tess.numVertexes = 0;

      // go back to the world modelview matrix
      //backEnd.orientation = backEnd.viewParms.world;
      //GL_LoadModelViewMatrix(backEnd.viewParms.world.modelViewMatrix);

      //R_DebugBoundingBox(vec3_origin, ent->worldBounds[0], ent->worldBounds[1], colorCyan);
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showSkeleton->integer )
  {
    int                  i, j, k, parentIndex;
    trRefEntity_t        *ent;
    vec3_t               origin, offset;
    vec3_t               forward, right, up;
    vec3_t               diff, tmp, tmp2, tmp3;
    vec_t                length;
    vec4_t               tetraVerts[ 4 ];
    static refSkeleton_t skeleton;
    refSkeleton_t        *skel;

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.charsetImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    ent = backEnd.refdef.entities;

    for ( i = 0; i < backEnd.refdef.numEntities; i++, ent++ )
    {
      if ( ( ent->e.renderfx & RF_THIRD_PERSON ) && !backEnd.viewParms.isPortal )
      {
        continue;
      }

      // set up the transformation matrix
      R_RotateEntityForViewParms( ent, &backEnd.viewParms, &backEnd.orientation );
      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
      gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      tess.multiDrawPrimitives = 0;
      tess.numVertexes = 0;
      tess.numIndexes = 0;

      skel = NULL;

      if ( ent->e.skeleton.type == SK_ABSOLUTE )
      {
        skel = &ent->e.skeleton;
      }
      else
      {
        model_t   *model;
        refBone_t *bone;

        model = R_GetModelByHandle( ent->e.hModel );

        if ( model )
        {
          switch ( model->type )
          {
            case MOD_MD5:
              {
                // copy absolute bones
                skeleton.numBones = model->md5->numBones;

                for ( j = 0, bone = &skeleton.bones[ 0 ]; j < skeleton.numBones; j++, bone++ )
                {
#if defined( REFBONE_NAMES )
                  //if ( r_showSkeletonNames->integer )
                  Q_strncpyz( bone->name, model->md5->bones[ j ].name, sizeof( bone->name ) );
#endif

                  bone->parentIndex = model->md5->bones[ j ].parentIndex;
                  TransInitRotationQuat( model->md5->bones[ j ].rotation, &bone->t );
                  TransAddTranslation( model->md5->bones[ j ].origin, &bone->t );
                }

                skel = &skeleton;
                break;
              }

            default:
              break;
          }
        }
      }

      if ( skel )
      {
        static vec3_t worldOrigins[ MAX_BONES ];

        GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );

        for ( j = 0; j < skel->numBones; j++ )
        {
          parentIndex = skel->bones[ j ].parentIndex;

          if ( parentIndex < 0 )
          {
            VectorClear( origin );
          }
          else
          {
            VectorCopy( skel->bones[ parentIndex ].t.trans, origin );
          }

          VectorCopy( skel->bones[ j ].t.trans, offset );
          QuatToVectorsFRU( skel->bones[ j ].t.rot, forward, right, up );

          VectorSubtract( offset, origin, diff );

          if ( ( length = VectorNormalize( diff ) ) )
          {
            vec4_t boneColor;
            Vector4Scale(g_color_table[ColorIndex(j)], 0.5f, boneColor);
            boneColor[0] += 0.5f;
            boneColor[1] += 0.5f;
            boneColor[2] += 0.5f;
            boneColor[3] = 1.0f;

            PerpendicularVector( tmp, diff );
            //VectorCopy(up, tmp);

            VectorScale( tmp, length * 0.1, tmp2 );
            VectorMA( tmp2, length * 0.2, diff, tmp2 );

            for ( k = 0; k < 3; k++ )
            {
              RotatePointAroundVector( tmp3, diff, tmp2, k * 120 );
              VectorAdd( tmp3, origin, tmp3 );
              VectorScale( tmp3, skel->scale, tetraVerts[ k ] );
              tetraVerts[ k ][ 3 ] = 1;
            }

            VectorScale( origin, skel->scale, tetraVerts[ 3 ] );
            tetraVerts[ 3 ][ 3 ] = 1;
            Tess_AddTetrahedron( tetraVerts, boneColor );

            VectorScale( offset, skel->scale, tetraVerts[ 3 ] );
            tetraVerts[ 3 ][ 3 ] = 1;
            Tess_AddTetrahedron( tetraVerts, boneColor );
          }

          VectorScale(skel->bones[ j ].t.trans, skel->scale, tmp); //add scale
          MatrixTransformPoint( backEnd.orientation.transformMatrix, tmp, worldOrigins[ j ] ); 

          //MatrixTransformPoint( backEnd.orientation.transformMatrix, skel->bones[ j ].t.trans, worldOrigins[ j ] );

        }

        Tess_UpdateVBOs( ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR );

        Tess_DrawElements();

        tess.multiDrawPrimitives = 0;
        tess.numVertexes = 0;
        tess.numIndexes = 0;

#if defined( REFBONE_NAMES )
        if ( r_showSkeletonNames->integer )
        {
          GL_State( GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

          // go back to the world modelview matrix
          backEnd.orientation = backEnd.viewParms.world;
          GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
          gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

          // draw names
          for ( j = 0; j < skel->numBones; j++ )
          {
            vec3_t left, up;
            float  radius;
            vec3_t origin;

            // calculate the xyz locations for the four corners
            radius = 0.4f;
            VectorScale( backEnd.viewParms.orientation.axis[ 1 ], radius, left );
            VectorScale( backEnd.viewParms.orientation.axis[ 2 ], radius, up );

            if ( backEnd.viewParms.isMirror )
            {
              VectorSubtract( vec3_origin, left, left );
            }

            for ( k = 0; ( unsigned ) k < strlen( skel->bones[ j ].name ); k++ )
            {
              int   ch;
              int   row, col;
              float frow, fcol;
              float size;

              ch = skel->bones[ j ].name[ k ];
              ch &= 255;

              if ( ch == ' ' )
              {
                break;
              }

              row = ch >> 4;
              col = ch & 15;

              frow = row * 0.0625;
              fcol = col * 0.0625;
              size = 0.0625;

              VectorMA( worldOrigins[ j ], - ( k + 2.0f ), left, origin );
              Tess_AddQuadStampExt( origin, left, up, colorWhite, fcol, frow, fcol + size, frow + size );
            }

            Tess_UpdateVBOs( ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR );

            Tess_DrawElements();

            tess.multiDrawPrimitives = 0;
            tess.numVertexes = 0;
            tess.numIndexes = 0;
          }
        }
#endif // REFBONE_NAMES
      }

      tess.multiDrawPrimitives = 0;
      tess.numVertexes = 0;
      tess.numIndexes = 0;
    }
  }

  if ( r_showLightScissors->integer )
  {
    interaction_t *ia;
    int           iaCount;
    matrix_t      ortho;
    vec4_t        quadVerts[ 4 ];

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_CUSTOM_RGB, AGEN_CUSTOM );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    // set 2D virtual screen size
    GL_PushMatrix();
    MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                                backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                                backEnd.viewParms.viewportY,
                                backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999 );
    GL_LoadProjectionMatrix( ortho );
    GL_LoadModelViewMatrix( matrixIdentity );

    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    for ( iaCount = 0, ia = &backEnd.viewParms.interactions[ 0 ]; iaCount < backEnd.viewParms.numInteractions; )
    {
      if ( glConfig2.occlusionQueryBits && glConfig.driverType != GLDRV_MESA )
      {
        if ( !ia->occlusionQuerySamples )
        {
          gl_genericShader->SetUniform_Color( colorRed );
        }
        else
        {
          gl_genericShader->SetUniform_Color( colorGreen );
        }

        Vector4Set(quadVerts[0], (float)ia->scissorX, (float)ia->scissorY, 0.f, 1.f);
        Vector4Set(quadVerts[1], (float)(ia->scissorX + ia->scissorWidth - 1), ia->scissorY, 0.f, 1.f);
        Vector4Set(quadVerts[2], (float)(ia->scissorX + ia->scissorWidth - 1), (ia->scissorY + ia->scissorHeight - 1), 0.f, 1.f);
        Vector4Set(quadVerts[3], (float)ia->scissorX, (float)(ia->scissorY + ia->scissorHeight - 1), 0.f, 1.f);
        Tess_InstantQuad( quadVerts );
      }
      else
      {
        gl_genericShader->SetUniform_Color( colorWhite );

        Vector4Set(quadVerts[0], (float)ia->scissorX, (float)ia->scissorY, 0.f, 1.f);
        Vector4Set(quadVerts[1], (float)(ia->scissorX + ia->scissorWidth - 1), (float)ia->scissorY, 0.f, 1.f);
        Vector4Set(quadVerts[2], (float)(ia->scissorX + ia->scissorWidth - 1), (float)(ia->scissorY + ia->scissorHeight - 1), 0.f, 1.f);
        Vector4Set(quadVerts[3], (float)ia->scissorX, (float)(ia->scissorY + ia->scissorHeight - 1), 0.f, 1.f);
        Tess_InstantQuad( quadVerts );
      }

      if ( !ia->next )
      {
        if ( iaCount < ( backEnd.viewParms.numInteractions - 1 ) )
        {
          // jump to next interaction and continue
          ia++;
          iaCount++;
        }
        else
        {
          // increase last time to leave for loop
          iaCount++;
        }
      }
      else
      {
        // just continue
        ia = ia->next;
        iaCount++;
      }
    }

    GL_PopMatrix();
  }

  if ( r_showCubeProbes->integer )
  {
    cubemapProbe_t *cubeProbe;
    int            j;
//		vec4_t          quadVerts[4];
    static const vec3_t mins = { -8, -8, -8 };
    static const vec3_t maxs = { 8,  8,  8 };
    //vec3_t      viewOrigin;

    if ( backEnd.refdef.rdflags & ( RDF_NOWORLDMODEL | RDF_NOCUBEMAP ) )
    {
      return;
    }

    // choose right shader program ----------------------------------
    gl_reflectionShader->SetVertexSkinning( false );
    gl_reflectionShader->SetVertexAnimation( false );

    gl_reflectionShader->SetDeformVertexes( false );

    gl_reflectionShader->SetNormalMapping( false );
//		gl_reflectionShader->DisableMacro_TWOSIDED();

    gl_reflectionShader->BindProgram();

    // end choose right shader program ------------------------------

    gl_reflectionShader->SetUniform_ViewOrigin( backEnd.viewParms.orientation.origin );  // in world space

    GL_State( 0 );
    GL_Cull( CT_FRONT_SIDED );

    // set up the transformation matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );

    gl_reflectionShader->SetUniform_ModelMatrix( backEnd.orientation.transformMatrix );
    gl_reflectionShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    Tess_Begin( Tess_StageIteratorDebug, NULL, NULL, NULL, qtrue, qfalse, -1, 0 );

    for ( j = 0; j < tr.cubeProbes.currentElements; j++ )
    {
      cubeProbe = ( cubemapProbe_t * ) Com_GrowListElement( &tr.cubeProbes, j );

      // bind u_ColorMap
      GL_BindToTMU( 0, cubeProbe->cubemap );

      Tess_AddCubeWithNormals( cubeProbe->origin, mins, maxs, colorWhite );
    }

    Tess_End();

    if (r_showCubeProbes->integer == 1) //2 disable reg/green nearest cube
    {
      cubemapProbe_t *cubeProbeNearest;
      cubemapProbe_t *cubeProbeSecondNearest;

      gl_genericShader->DisableVertexSkinning();
      gl_genericShader->DisableVertexAnimation();
      gl_genericShader->DisableDeformVertexes();
      gl_genericShader->DisableTCGenEnvironment();

      gl_genericShader->BindProgram();

      gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
      gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
      gl_genericShader->SetUniform_Color( colorBlack );

      gl_genericShader->SetRequiredVertexPointers();

      GL_State( GLS_DEFAULT );
      GL_Cull( CT_TWO_SIDED );

      // set uniforms

      // set up the transformation matrix
      backEnd.orientation = backEnd.viewParms.world;
      GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
      gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

      // bind u_ColorMap
      GL_BindToTMU( 0, tr.whiteImage );
      gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

      GL_CheckErrors();

      R_FindTwoNearestCubeMaps( backEnd.viewParms.orientation.origin, &cubeProbeNearest, &cubeProbeSecondNearest );

      Tess_Begin( Tess_StageIteratorDebug, NULL, NULL, NULL, qtrue, qfalse, -1, 0 );

      if ( cubeProbeNearest == NULL && cubeProbeSecondNearest == NULL )
      {
        // bad
      }
      else if ( cubeProbeNearest == NULL )
      {
        Tess_AddCubeWithNormals( cubeProbeSecondNearest->origin, mins, maxs, colorBlue );
      }
      else if ( cubeProbeSecondNearest == NULL )
      {
        Tess_AddCubeWithNormals( cubeProbeNearest->origin, mins, maxs, colorYellow );
      }
      else
      {
        Tess_AddCubeWithNormals( cubeProbeNearest->origin, mins, maxs, colorGreen );
        Tess_AddCubeWithNormals( cubeProbeSecondNearest->origin, mins, maxs, colorRed );
      }

      Tess_End();
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showLightGrid->integer && tr.world->numLightGridPoints > 0 
      && tr.world->lightGridData) //failsafe
  {
    bspGridPoint_t *gridPoint;
    int            j, k;
    vec3_t         offset;
    vec3_t         lightDirection;
    vec3_t         tmp, tmp2, tmp3;
    vec_t          length;
    vec4_t         tetraVerts[ 4 ];

    if ( backEnd.refdef.rdflags & ( RDF_NOWORLDMODEL | RDF_NOCUBEMAP ) )
    {
      return;
    }

    GLimp_LogComment( "--- r_showLightGrid > 0: Rendering light grid\n" );

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );

    gl_genericShader->SetRequiredVertexPointers();

    GL_State( GLS_DEFAULT );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms

    // set up the transformation matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    GL_CheckErrors();

    Tess_Begin( Tess_StageIteratorDebug, NULL, NULL, NULL, qtrue, qfalse, -1, 0 );

    for ( j = 0; j < tr.world->numLightGridPoints; j++ )
    {
      gridPoint = &tr.world->lightGridData[ j ];

      if (!gridPoint)
      { //hypov8 fix null lightgrid
        R_FallbackLight(gridPoint->ambientColor, gridPoint->directedColor, gridPoint->direction);
        VectorCopy(backEnd.viewParms.orientation.origin,  gridPoint->origin);
      }

      if ( DistanceSquared( gridPoint->origin, backEnd.viewParms.orientation.origin ) > Square( 1024 ) )
      {
        continue;
      }

      VectorNegate( gridPoint->direction, lightDirection );

      length = 8;
      VectorMA( gridPoint->origin, 8, lightDirection, offset );

      PerpendicularVector( tmp, lightDirection );
      //VectorCopy(up, tmp);

      VectorScale( tmp, length * 0.1, tmp2 );
      VectorMA( tmp2, length * 0.2, lightDirection, tmp2 );

      for ( k = 0; k < 3; k++ )
      {
        RotatePointAroundVector( tmp3, lightDirection, tmp2, k * 120 );
        VectorAdd( tmp3, gridPoint->origin, tmp3 );
        VectorCopy( tmp3, tetraVerts[ k ] );
        tetraVerts[ k ][ 3 ] = 1;
      }

      VectorCopy( gridPoint->origin, tetraVerts[ 3 ] );
      tetraVerts[ 3 ][ 3 ] = 1;
      Tess_AddTetrahedron( tetraVerts, gridPoint->ambientColor ); //top 1/2 ambient color

      VectorCopy( offset, tetraVerts[ 3 ] );
      tetraVerts[ 3 ][ 3 ] = 1;
      Tess_AddTetrahedron( tetraVerts, gridPoint->directedColor ); //lower 1/2 directed color
    }

    Tess_End();

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showBspNodes->integer )
  {
    bspNode_t *node;
    link_t    *l, *sentinel;

    if ( ( backEnd.refdef.rdflags & ( RDF_NOWORLDMODEL ) ) || !tr.world )
    {
      return;
    }

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_CUSTOM_RGB, AGEN_CUSTOM );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    GL_CheckErrors();

    for ( int i = 0; i < 2; i++ )
    {
      float    x, y, w, h;
      matrix_t ortho;
      vec4_t   quadVerts[ 4 ];

      if ( i == 1 )
      {
        // set 2D virtual screen size
        GL_PushMatrix();
        MatrixOrthogonalProjection( ortho, backEnd.viewParms.viewportX,
                                    backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
                                    backEnd.viewParms.viewportY,
                                    backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999 );
        GL_LoadProjectionMatrix( ortho );
        GL_LoadModelViewMatrix( matrixIdentity );

        GL_Cull( CT_TWO_SIDED );
        GL_State( GLS_DEPTHTEST_DISABLE );

        gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
        gl_genericShader->SetUniform_Color( colorBlack );

        w = 300;
        h = 300;

        x = 20;
        y = 90;

        Vector4Set(quadVerts[0], x, y, 0.f, 1.f);
        Vector4Set(quadVerts[1], x + w, y, 0.f, 1.f);
        Vector4Set(quadVerts[2], x + w, y + h, 0.f, 1.f);
        Vector4Set(quadVerts[3], x, y + h, 0.f, 1.f);

        Tess_InstantQuad( quadVerts );

        {
          int    j;
          vec4_t splitFrustum[ 6 ];
          vec3_t farCorners[ 4 ];
          vec3_t nearCorners[ 4 ];
          vec3_t cropBounds[ 2 ];
          vec4_t point, transf;

          GL_Viewport( x, y, w, h );
          GL_Scissor( x, y, w, h );

          GL_PushMatrix();

          // calculate top down view projection matrix
          {
            vec3_t                                                       forward = { 0, 0, -1 };
            vec3_t                                                       up = { 1, 0, 0 };

            matrix_t /*rotationMatrix, transformMatrix,*/ viewMatrix, projectionMatrix;

            // Quake -> OpenGL view matrix from light perspective
            MatrixLookAtRH( viewMatrix, backEnd.viewParms.orientation.origin, forward, up );

            ClearBounds( cropBounds[ 0 ], cropBounds[ 1 ] );

            for ( j = 0; j < 8; j++ )
            {
              point[ 0 ] = tr.world->models[ 0 ].bounds[ j & 1 ][ 0 ];
              point[ 1 ] = tr.world->models[ 0 ].bounds[( j >> 1 ) & 1 ][ 1 ];
              point[ 2 ] = tr.world->models[ 0 ].bounds[( j >> 2 ) & 1 ][ 2 ];
              point[ 3 ] = 1;

              MatrixTransform4( viewMatrix, point, transf );
              transf[ 0 ] /= transf[ 3 ];
              transf[ 1 ] /= transf[ 3 ];
              transf[ 2 ] /= transf[ 3 ];

              AddPointToBounds( transf, cropBounds[ 0 ], cropBounds[ 1 ] );
            }

            MatrixOrthogonalProjectionRH( projectionMatrix, cropBounds[ 0 ][ 0 ], cropBounds[ 1 ][ 0 ], cropBounds[ 0 ][ 1 ], cropBounds[ 1 ][ 1 ], -cropBounds[ 1 ][ 2 ], -cropBounds[ 0 ][ 2 ] );

            GL_LoadModelViewMatrix( viewMatrix );
            GL_LoadProjectionMatrix( projectionMatrix );
          }

          // set uniforms
          gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
          gl_genericShader->SetUniform_Color( colorBlack );

          GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
          GL_Cull( CT_TWO_SIDED );

          // bind u_ColorMap
          GL_BindToTMU( 0, tr.whiteImage );
          gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

          gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

          tess.multiDrawPrimitives = 0;
          tess.numIndexes = 0;
          tess.numVertexes = 0;

          for ( j = 0; j < 6; j++ )
          {
            VectorCopy( backEnd.viewParms.frustums[ 0 ][ j ].normal, splitFrustum[ j ] );
            splitFrustum[ j ][ 3 ] = backEnd.viewParms.frustums[ 0 ][ j ].dist;
          }

          // calculate split frustum corner points
          R_CalcFrustumNearCorners( splitFrustum, nearCorners );
          R_CalcFrustumFarCorners( splitFrustum, farCorners );

          // draw outer surfaces
          for ( j = 0; j < 4; j++ )
          {
            Vector4Set(quadVerts[0], nearCorners[j][0], nearCorners[j][1], nearCorners[j][2], 1.f);
            Vector4Set(quadVerts[1], farCorners[j][0], farCorners[j][1], farCorners[j][2], 1.f);
            Vector4Set(quadVerts[2], farCorners[(j + 1) % 4][0], farCorners[(j + 1) % 4][1], farCorners[(j + 1) % 4][2], 1.f);
            Vector4Set(quadVerts[3], nearCorners[(j + 1) % 4][0], nearCorners[(j + 1) % 4][1], nearCorners[(j + 1) % 4][2], 1.f);
            Tess_AddQuadStamp2( quadVerts, colorCyan );
          }

          // draw far cap
          Vector4Set(quadVerts[0], farCorners[3][0], farCorners[3][1], farCorners[3][2], 1.f);
          Vector4Set(quadVerts[1], farCorners[2][0], farCorners[2][1], farCorners[2][2], 1.f);
          Vector4Set(quadVerts[2], farCorners[1][0], farCorners[1][1], farCorners[1][2], 1.f);
          Vector4Set(quadVerts[3], farCorners[0][0], farCorners[0][1], farCorners[0][2], 1.f);
          Tess_AddQuadStamp2( quadVerts, colorBlue );

          // draw near cap
          Vector4Set(quadVerts[0], nearCorners[0][0], nearCorners[0][1], nearCorners[0][2], 1.f);
          Vector4Set(quadVerts[1], nearCorners[1][0], nearCorners[1][1], nearCorners[1][2], 1.f);
          Vector4Set(quadVerts[2], nearCorners[2][0], nearCorners[2][1], nearCorners[2][2], 1.f);
          Vector4Set(quadVerts[3], nearCorners[3][0], nearCorners[3][1], nearCorners[3][2], 1.f);
          Tess_AddQuadStamp2( quadVerts, colorGreen );

          Tess_UpdateVBOs( ATTR_POSITION | ATTR_COLOR );
          Tess_DrawElements();

          gl_genericShader->SetUniform_ColorModulate( CGEN_CUSTOM_RGB, AGEN_CUSTOM );
        }
      } // i == 1
      else
      {
        GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
        GL_Cull( CT_TWO_SIDED );

        // render in world space
        backEnd.orientation = backEnd.viewParms.world;

        GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );
        GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );

        gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );
      }

      // draw BSP nodes
      sentinel = &tr.traversalStack;

      for ( l = sentinel->next; l != sentinel; l = l->next )
      {
        node = ( bspNode_t * ) l->data;

        if ( !r_dynamicBspOcclusionCulling->integer )
        {
          if ( node->contents != -1 )
          {
            if ( r_showBspNodes->integer == 3 )
            {
              continue;
            }

            if ( node->numMarkSurfaces <= 0 )
            {
              continue;
            }

            //if(node->shrinkedAABB)
            //  gl_genericShader->SetUniform_Color(colorBlue);
            //else
            if ( node->visCounts[ tr.visIndex ] == tr.visCounts[ tr.visIndex ] )
            {
              gl_genericShader->SetUniform_Color( colorGreen );
            }
            else
            {
              gl_genericShader->SetUniform_Color( colorRed );
            }
          }
          else
          {
            if ( r_showBspNodes->integer == 2 )
            {
              continue;
            }

            if ( node->visCounts[ tr.visIndex ] == tr.visCounts[ tr.visIndex ] )
            {
              gl_genericShader->SetUniform_Color( colorYellow );
            }
            else
            {
              gl_genericShader->SetUniform_Color( colorBlue );
            }
          }
        }
        else
        {
          if ( node->lastVisited[ backEnd.viewParms.viewCount ] != backEnd.viewParms.frameCount )
          {
            continue;
          }

          if ( r_showBspNodes->integer == 5 && node->lastQueried[ backEnd.viewParms.viewCount ] != backEnd.viewParms.frameCount )
          {
            continue;
          }

          if ( node->contents != -1 )
          {
            if ( r_showBspNodes->integer == 3 )
            {
              continue;
            }

            //if(node->occlusionQuerySamples[backEnd.viewParms.viewCount] > 0)
            if ( node->visible[ backEnd.viewParms.viewCount ] )
            {
              gl_genericShader->SetUniform_Color( colorGreen );
            }
            else
            {
              gl_genericShader->SetUniform_Color( colorRed );
            }
          }
          else
          {
            if ( r_showBspNodes->integer == 2 )
            {
              continue;
            }

            //if(node->occlusionQuerySamples[backEnd.viewParms.viewCount] > 0)
            if ( node->visible[ backEnd.viewParms.viewCount ] )
            {
              gl_genericShader->SetUniform_Color( colorYellow );
            }
            else
            {
              gl_genericShader->SetUniform_Color( colorBlue );
            }
          }

          if ( r_showBspNodes->integer == 4 )
          {
            gl_genericShader->SetUniform_Color( g_color_table[ ColorIndex( node->occlusionQueryNumbers[ backEnd.viewParms.viewCount ] ) ] );
          }

          GL_CheckErrors();
        }

        if ( node->contents != -1 )
        {
          glEnable( GL_POLYGON_OFFSET_FILL );
          GL_PolygonOffset( r_offsetFactor->value, r_offsetUnits->value );
        }

        R_BindVBO( node->volumeVBO );
        R_BindIBO( node->volumeIBO );

        GL_VertexAttribsState( ATTR_POSITION );

        tess.multiDrawPrimitives = 0;
        tess.numVertexes = node->volumeVerts;
        tess.numIndexes = node->volumeIndexes;

        Tess_DrawElements();

        tess.numIndexes = 0;
        tess.numVertexes = 0;

        if ( node->contents != -1 )
        {
          glDisable( GL_POLYGON_OFFSET_FILL );
        }
      }

      if ( i == 1 )
      {
        tess.multiDrawPrimitives = 0;
        tess.numIndexes = 0;
        tess.numVertexes = 0;

        GL_PopMatrix();

        GL_Viewport( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                     backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

        GL_Scissor( backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
                    backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight );

        GL_PopMatrix();
      }
    }

    // go back to the world modelview matrix
    backEnd.orientation = backEnd.viewParms.world;

    GL_LoadProjectionMatrix( backEnd.viewParms.projectionMatrix );
    GL_LoadModelViewMatrix( backEnd.viewParms.world.modelViewMatrix );
  }

  if ( r_showDecalProjectors->integer )
  {
    int              i;
    decalProjector_t *dp;
    srfDecal_t       *srfDecal;
    static const vec3_t mins = { -1, -1, -1 };
    static const vec3_t maxs = { 1, 1, 1 };

    if ( backEnd.refdef.rdflags & ( RDF_NOWORLDMODEL ) )
    {
      return;
    }

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();

    gl_genericShader->BindProgram();

    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE );
    GL_Cull( CT_TWO_SIDED );

    // set uniforms
    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
    gl_genericShader->SetUniform_Color( colorBlack );

    // set up the transformation matrix
    backEnd.orientation = backEnd.viewParms.world;
    GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

    GL_CheckErrors();

    Tess_Begin( Tess_StageIteratorDebug, NULL, NULL, NULL, qtrue, qfalse, -1, 0 );

    for ( i = 0, dp = backEnd.refdef.decalProjectors; i < backEnd.refdef.numDecalProjectors; i++, dp++ )
    {
      if ( DistanceSquared( dp->center, backEnd.viewParms.orientation.origin ) > Square( 1024 ) )
      {
        continue;
      }

      Tess_AddCube( dp->center, mins, maxs, colorRed );
      Tess_AddCube( vec3_origin, dp->mins, dp->maxs, colorBlue );
    }

    glEnable( GL_POLYGON_OFFSET_FILL );
    GL_PolygonOffset( r_offsetFactor->value, r_offsetUnits->value );

    for ( i = 0, srfDecal = backEnd.refdef.decals; i < backEnd.refdef.numDecals; i++, srfDecal++ )
    {
      rb_surfaceTable[ SF_DECAL ]( srfDecal );
    }

    glDisable( GL_POLYGON_OFFSET_FILL );

    Tess_End();

    // go back to the world modelview matrix
    //backEnd.orientation = backEnd.viewParms.world;
    //GL_LoadModelViewMatrix(backEnd.viewParms.world.modelViewMatrix);
  }

  GL_CheckErrors();
}
#if 0
static unsigned int drawMode;
static int maxDebugVerts;
static float currentDebugSize;

static unsigned int drawMode;
static debugDrawMode_t currentDebugDrawMode;
static int maxDebugVerts;
static float currentDebugSize;

void DebugDrawBegin( debugDrawMode_t mode, float size ) {

  if ( tess.numVertexes )
  {
    Tess_End();
  }

  const vec4_t colorClear = { 0, 0, 0, 0 };
  currentDebugDrawMode = mode;
  currentDebugSize = size;
  switch(mode) {
    case D_DRAW_POINTS:
      glPointSize( size );
      drawMode = GL_POINTS;
      maxDebugVerts = SHADER_MAX_VERTEXES - 1;
      break;
    case D_DRAW_LINES:
      glLineWidth( size );
      drawMode = GL_LINES;
      maxDebugVerts = ( SHADER_MAX_VERTEXES - 1 )/2*2;
      break;
    case D_DRAW_TRIS:
      drawMode = GL_TRIANGLES;
      maxDebugVerts = ( SHADER_MAX_VERTEXES - 1 )/3*3;
      break;
    case D_DRAW_QUADS:
      drawMode = GL_QUADS;
      maxDebugVerts = ( SHADER_MAX_VERTEXES - 1 )/4*4;
      break;
  }

  gl_genericShader->DisableVertexSkinning();
  gl_genericShader->DisableVertexAnimation();
  gl_genericShader->DisableDeformVertexes();
  gl_genericShader->DisableTCGenEnvironment();
  gl_genericShader->DisableTCGenLightmap();
  gl_genericShader->BindProgram();

  GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
  GL_Cull( CT_FRONT_SIDED );

  GL_VertexAttribsState( ATTR_POSITION | ATTR_COLOR | ATTR_TEXCOORD );

  // set uniforms
  gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
  gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
  gl_genericShader->SetUniform_Color( colorClear );

  // bind u_ColorMap
  GL_SelectTexture( 0 );
  GL_Bind( tr.whiteImage );
  gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

  // render in world space
  backEnd.orientation = backEnd.viewParms.world;
  GL_LoadModelViewMatrix( backEnd.orientation.modelViewMatrix );
  gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  GL_CheckErrors();
}


void DebugDrawDepthMask(qboolean state)
{
  GL_DepthMask( state ? GL_TRUE : GL_FALSE );
}

void DebugDrawVertex(const vec3_t pos, unsigned int color, const vec2_t uv) {
  vec4_t colors = {
    static_cast<vec_t>(color & 0xFF),
    static_cast<vec_t>((color >> 8) & 0xFF),
    static_cast<vec_t>((color >> 16) & 0xFF),
    static_cast<vec_t>((color >> 24) & 0xFF)
  };
  Vector4Scale(colors, 1.0f/255.0f, colors);

  //we have reached the maximum number of verts we can batch
  if( tess.numVertexes == maxDebugVerts ) {
    //draw the geometry we already have
    DebugDrawEnd();
    //start drawing again
    DebugDrawBegin(currentDebugDrawMode, currentDebugSize);
  }

  tess.xyz[ tess.numVertexes ][ 0 ] = pos[ 0 ];
  tess.xyz[ tess.numVertexes ][ 1 ] = pos[ 1 ];
  tess.xyz[ tess.numVertexes ][ 2 ] = pos[ 2 ];
  tess.xyz[ tess.numVertexes ][ 3 ] = 1;
  Vector4Copy(colors, tess.lightColor[ tess.numVertexes ]);
  if( uv ) {
    tess.texCoords[ tess.numVertexes ][ 0 ] = uv[ 0 ];
    tess.texCoords[ tess.numVertexes ][ 1 ] = uv[ 1 ];
  }
  tess.indexes[ tess.numIndexes ] = tess.numVertexes;
  tess.numVertexes++;
  tess.numIndexes++;
}

void DebugDrawEnd( void ) {

  Tess_UpdateVBOs( ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR );

  if ( glState.currentVBO && glState.currentIBO )
  {
    glDrawElements( drawMode, tess.numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET( 0 ) );

    backEnd.pc.c_drawElements++;

    backEnd.pc.c_vboVertexes += tess.numVertexes;
    backEnd.pc.c_vboIndexes += tess.numIndexes;

    backEnd.pc.c_indexes += tess.numIndexes;
    backEnd.pc.c_vertexes += tess.numVertexes;
  }
  tess.numVertexes = 0;
  tess.numIndexes = 0;

  glLineWidth( 1.0f );
  glPointSize( 1.0f );
}
#endif
/*
==================
RB_RenderView
==================
*/
static void RB_RenderView( void )
{
  int clearBits = 0;
  int startTime = 0, endTime = 0;

  if ( r_logFile->integer )
  {
    // don't just call LogComment, or we will get a call to va() every frame!
    GLimp_LogComment( va
                      ( "--- RB_RenderView( %i surfaces, %i interactions ) ---\n", backEnd.viewParms.numDrawSurfs,
                        backEnd.viewParms.numInteractions ) );
  }

  //ri.Error(ERR_FATAL, "test");

  GL_CheckErrors();

  backEnd.pc.c_surfaces += backEnd.viewParms.numDrawSurfs;

  // sync with gl if needed
  if ( r_finish->integer == 1 && !glState.finishCalled )
  {
    glFinish();
    glState.finishCalled = qtrue;
  }

  if ( r_finish->integer == 0 )
  {
    glState.finishCalled = qtrue;
  }

  // disable offscreen rendering
  if ( glConfig2.framebufferObjectAvailable )
  {
    if ( r_hdrRendering->integer && glConfig2.textureFloatAvailable )
    {
      R_BindFBO( tr.deferredRenderFBO );
    }
    else
    {
      R_BindNullFBO();
    }
  }

  // we will need to change the projection matrix before drawing
  // 2D images again
  backEnd.projection2D = qfalse;

  // set the modelview matrix for the viewer
  SetViewportAndScissor();

  // ensures that depth writes are enabled for the depth clear
  GL_State( GLS_DEFAULT );

  // clear relevant buffers
  clearBits = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

#if /*!defined( COMPAT_KPQ3 ) &&*/ defined( COMPAT_ET )  //hypov8 todo: fog?
  // ydnar: global q3 fog volume
  if ( tr.world && tr.world->globalFog >= 0 )
  {
    if ( !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
    {
      clearBits |= GL_COLOR_BUFFER_BIT;

      GL_ClearColor( tr.world->fogs[ tr.world->globalFog ].color[ 0 ],
                    tr.world->fogs[ tr.world->globalFog ].color[ 1 ],
                    tr.world->fogs[ tr.world->globalFog ].color[ 2 ], 1.0 );
    }
  }
  else if ( tr.world && tr.world->hasSkyboxPortal )
  {
    if ( backEnd.refdef.rdflags & RDF_SKYBOXPORTAL )
    {
      // portal scene, clear whatever is necessary

      if ( r_fastsky->integer || backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
      {
        // fastsky: clear color

        // try clearing first with the portal sky fog color, then the world fog color, then finally a default
        clearBits |= GL_COLOR_BUFFER_BIT;

        if ( tr.glfogsettings[ FOG_PORTALVIEW ].registered )
        {
          GL_ClearColor( tr.glfogsettings[ FOG_PORTALVIEW ].color[ 0 ], tr.glfogsettings[ FOG_PORTALVIEW ].color[ 1 ],
                        tr.glfogsettings[ FOG_PORTALVIEW ].color[ 2 ], tr.glfogsettings[ FOG_PORTALVIEW ].color[ 3 ] );
        }
        else if ( tr.glfogNum > FOG_NONE && tr.glfogsettings[ FOG_CURRENT ].registered )
        {
          GL_ClearColor( tr.glfogsettings[ FOG_CURRENT ].color[ 0 ], tr.glfogsettings[ FOG_CURRENT ].color[ 1 ],
                        tr.glfogsettings[ FOG_CURRENT ].color[ 2 ], tr.glfogsettings[ FOG_CURRENT ].color[ 3 ] );
        }
        else
        {
          //                  GL_ClearColor ( 1.0, 0.0, 0.0, 1.0 );   // red clear for testing portal sky clear
          GL_ClearColor( 0.5, 0.5, 0.5, 1.0 );
        }
      }
      else
      {
        // rendered sky (either clear color or draw quake sky)
        if ( tr.glfogsettings[ FOG_PORTALVIEW ].registered )
        {
          GL_ClearColor( tr.glfogsettings[ FOG_PORTALVIEW ].color[ 0 ], tr.glfogsettings[ FOG_PORTALVIEW ].color[ 1 ],
                        tr.glfogsettings[ FOG_PORTALVIEW ].color[ 2 ], tr.glfogsettings[ FOG_PORTALVIEW ].color[ 3 ] );

          if ( tr.glfogsettings[ FOG_PORTALVIEW ].clearscreen )
          {
            // portal fog requests a screen clear (distance fog rather than quake sky)
            clearBits |= GL_COLOR_BUFFER_BIT;
          }
        }
      }
    }
    else
    {
      // world scene with portal sky, don't clear any buffers, just set the fog color if there is one

      if ( tr.glfogNum > FOG_NONE && tr.glfogsettings[ FOG_CURRENT ].registered )
      {
        if ( backEnd.refdef.rdflags & RDF_UNDERWATER )
        {
          if ( tr.glfogsettings[ FOG_CURRENT ].mode == GL_LINEAR )
          {
            clearBits |= GL_COLOR_BUFFER_BIT;
          }
        }
        else if ( !r_portalSky->integer )
        {
          // portal skies have been manually turned off, clear bg color
          clearBits |= GL_COLOR_BUFFER_BIT;
        }

        GL_ClearColor( tr.glfogsettings[ FOG_CURRENT ].color[ 0 ], tr.glfogsettings[ FOG_CURRENT ].color[ 1 ],
                      tr.glfogsettings[ FOG_CURRENT ].color[ 2 ], tr.glfogsettings[ FOG_CURRENT ].color[ 3 ] );
      }
      else if ( !r_portalSky->integer )
      {
        // ydnar: portal skies have been manually turned off, clear bg color
        clearBits |= GL_COLOR_BUFFER_BIT;
        GL_ClearColor( 0.5, 0.5, 0.5, 1.0 );
      }
    }
  }
  else
  {
    // world scene with no portal sky

    // NERVE - SMF - we don't want to clear the buffer when no world model is specified
    if ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
    {
      clearBits &= ~GL_COLOR_BUFFER_BIT;
    }
    // -NERVE - SMF
    else if ( r_fastsky->integer || backEnd.refdef.rdflags & RDF_NOWORLDMODEL )
    {
      clearBits |= GL_COLOR_BUFFER_BIT;

      if ( tr.glfogsettings[ FOG_CURRENT ].registered )
      {
        // try to clear fastsky with current fog color
        GL_ClearColor( tr.glfogsettings[ FOG_CURRENT ].color[ 0 ], tr.glfogsettings[ FOG_CURRENT ].color[ 1 ],
                      tr.glfogsettings[ FOG_CURRENT ].color[ 2 ], tr.glfogsettings[ FOG_CURRENT ].color[ 3 ] );
      }
      else
      {
        //              GL_ClearColor ( 0.0, 0.0, 1.0, 1.0 );   // blue clear for testing world sky clear
        GL_ClearColor( 0.05f, 0.05f, 0.05f, 1.0f );  // JPW NERVE changed per id req was 0.5s
      }
    }
    else
    {
      // world scene, no portal sky, not fastsky, clear color if fog says to, otherwise, just set the clearcolor
      if ( tr.glfogsettings[ FOG_CURRENT ].registered )
      {
        // try to clear fastsky with current fog color
        GL_ClearColor( tr.glfogsettings[ FOG_CURRENT ].color[ 0 ], tr.glfogsettings[ FOG_CURRENT ].color[ 1 ],
                      tr.glfogsettings[ FOG_CURRENT ].color[ 2 ], tr.glfogsettings[ FOG_CURRENT ].color[ 3 ] );

        if ( tr.glfogsettings[ FOG_CURRENT ].clearscreen )
        {
          // world fog requests a screen clear (distance fog rather than quake sky)
          clearBits |= GL_COLOR_BUFFER_BIT;
        }
      }
    }

    if ( HDR_ENABLED() )
    {
      // copy color of the main context to deferredRenderFBO
      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.deferredRenderFBO->frameBuffer );
      glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            GL_COLOR_BUFFER_BIT,
                            GL_NEAREST );
    }
  }

#else /*if !defined( COMPAT_KPQ3 )*/ //se

  if ( !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
  {
    clearBits |= GL_COLOR_BUFFER_BIT; // FIXME: only if sky shaders have been used
    GL_ClearColor( 0.0f, 0.0f, 0.0f, 1.0f );  // FIXME: get color of sky
  }
  else
  {
    if ( HDR_ENABLED() )
    {
      // copy color of the main context to deferredRenderFBO
      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.deferredRenderFBO->frameBuffer );
      glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            GL_COLOR_BUFFER_BIT,
                            GL_NEAREST );
    }
  }

#endif

  glClear( clearBits );
  //backEnd.depthRenderImageValid = qfalse;

  if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
  {
    RB_Hyperspace();
    return;
  }
  else
  {
    backEnd.isHyperspace = qfalse;
  }
  glState.faceCulling = -1;	// force face culling to set next time //beta1
  // we will only draw a sun if there was sky rendered in this view
  backEnd.skyRenderedThisView = qfalse;

  GL_CheckErrors();

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    startTime = ri.Milliseconds();
  }

  if ( r_dynamicEntityOcclusionCulling->integer )
  {
    // draw everything from world that is opaque into black so we can benefit from early-z rejections later
    //RB_RenderOpaqueSurfacesIntoDepth(true);

    //RB_RenderDrawSurfaces( true, DRAWSURFACES_WORLD ); //hypov8 merge: disable, use above

    // try to cull entities using hardware occlusion queries
    RB_RenderEntityOcclusionQueries();

    // draw everything that is opaque
    RB_RenderDrawSurfaces( true, DRAWSURFACES_ALL_ENTITIES );
  }
  else if( tr.refdef.blurVec[0] != 0.0f ||
      tr.refdef.blurVec[1] != 0.0f ||
      tr.refdef.blurVec[2] != 0.0f )
  {
    // draw everything that is not the gun
    RB_RenderDrawSurfaces( true, DRAWSURFACES_ALL_FAR );

    RB_RenderMotionBlur();

    // draw the gun and other "near" stuff
    RB_RenderDrawSurfaces( true, DRAWSURFACES_NEAR_ENTITIES );
  }
  else
  {
    // draw everything that is opaque
    RB_RenderDrawSurfaces( true, DRAWSURFACES_ALL );

    // try to cull entities using hardware occlusion queries
    //RB_RenderEntityOcclusionQueries();
  }

  // try to cull bsp nodes for the next frame using hardware occlusion queries
  //RB_RenderBspOcclusionQueries();

  if ( r_speeds->integer == RSPEEDS_SHADING_TIMES )
  {
    glFinish();
    endTime = ri.Milliseconds();
    backEnd.pc.c_forwardAmbientTime = endTime - startTime;
  }

  // try to cull lights using hardware occlusion queries
  RB_RenderLightOcclusionQueries();

  if ( r_shadows->integer >= SHADOWING_ESM16 )
  {
    // render dynamic shadowing and lighting using shadow mapping
    RB_RenderInteractionsShadowMapped();

    // render player shadows if any
    //RB_RenderInteractionsDeferredInverseShadows();
  }
  else
  {
    // render dynamic lighting
    RB_RenderInteractions();
  }

  // render ambient occlusion process effect
  // Tr3B: needs way more work RB_RenderScreenSpaceAmbientOcclusion(qfalse);

  if ( HDR_ENABLED() )
  {
    R_BindFBO( tr.deferredRenderFBO );
  }

  // render global fog post process effect
  RB_RenderGlobalFog();

  // draw everything that is translucent
  RB_RenderDrawSurfaces( false, DRAWSURFACES_ALL );

  // scale down rendered HDR scene to 1 / 4th
  if ( HDR_ENABLED() )
  {
    if ( glConfig2.framebufferBlitAvailable )
    {
      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, tr.deferredRenderFBO->frameBuffer );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.downScaleFBO_quarter->frameBuffer );
      glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            0, 0, glConfig.vidWidth * 0.25f, glConfig.vidHeight * 0.25f,
                            GL_COLOR_BUFFER_BIT,
                            GL_LINEAR );

      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, tr.deferredRenderFBO->frameBuffer );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.downScaleFBO_64x64->frameBuffer );
      glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            0, 0, 64, 64,
                            GL_COLOR_BUFFER_BIT,
                            GL_LINEAR );
    }
    else
    {
      // FIXME add non EXT_framebuffer_blit code
    }

    RB_CalculateAdaptation();
  }
  else
  {
    /*
    Tr3B: FIXME this causes: caught OpenGL error:
    GL_INVALID_OPERATION in file code/renderer/tr_backend.c line 6479

    if(glConfig2.framebufferBlitAvailable)
    {
          // copy deferredRenderFBO to downScaleFBO_quarter
          glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
          glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, tr.downScaleFBO_quarter->frameBuffer);
          glBlitFramebufferEXT(0, 0, glConfig.vidWidth, glConfig.vidHeight,
                                                          0, 0, glConfig.vidWidth * 0.25f, glConfig.vidHeight * 0.25f,
                                                          GL_COLOR_BUFFER_BIT,
                                                          GL_NEAREST);
    }
    else
    {
          // FIXME add non EXT_framebuffer_blit code
    }
    */
  }

  GL_CheckErrors();
#ifdef EXPERIMENTAL
  // render depth of field post process effect
  RB_RenderDepthOfField();
#endif
  // render bloom post process effect
  RB_RenderBloom();

  // copy offscreen rendered HDR scene to the current OpenGL context
  RB_RenderDeferredHDRResultToFrameBuffer();

#if 0
  // add the sun flare
  RB_DrawSun();
#endif

#if 0
  // add light flares on lights that aren't obscured
  RB_RenderFlares();
#endif

  // wait until all bsp node occlusion queries are back
  //RB_CollectBspOcclusionQueries();

  // render debug information
  RB_RenderDebugUtils();

  if ( backEnd.viewParms.isPortal )
  {
#if 0

    if ( r_hdrRendering->integer && glConfig.textureFloatAvailable && glConfig.framebufferObjectAvailable && glConfig.framebufferBlitAvailable )
    {
      // copy deferredRenderFBO to portalRenderFBO
      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, tr.deferredRenderFBO->frameBuffer );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.portalRenderFBO->frameBuffer );
      glBlitFramebufferEXT( 0, 0, tr.deferredRenderFBO->width, tr.deferredRenderFBO->height,
                            0, 0, tr.portalRenderFBO->width, tr.portalRenderFBO->height,
                            GL_COLOR_BUFFER_BIT,
                            GL_NEAREST );
    }

#endif
#if 0

    // FIXME: this trashes the OpenGL context for an unknown reason
    if ( glConfig2.framebufferObjectAvailable && glConfig2.framebufferBlitAvailable )
    {
      // copy main context to portalRenderFBO
      glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
      glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.portalRenderFBO->frameBuffer );
      glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            0, 0, glConfig.vidWidth, glConfig.vidHeight,
                            GL_COLOR_BUFFER_BIT,
                            GL_NEAREST );
    }

#endif
    //else
    {
      // capture current color buffer
      GL_SelectTexture( 0 );
      GL_Bind( tr.portalRenderImage );
      glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.portalRenderImage->uploadWidth, tr.portalRenderImage->uploadHeight );
    }
    backEnd.pc.c_portals++;
  }

#if 0

  if ( r_dynamicBspOcclusionCulling->integer )
  {
    // copy depth of the main context to deferredRenderFBO
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, tr.occlusionRenderFBO->frameBuffer );
    glBlitFramebufferEXT( 0, 0, glConfig.vidWidth, glConfig.vidHeight,
                          0, 0, glConfig.vidWidth, glConfig.vidHeight,
                          GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST );
  }

#endif

  RB_FXAA();

  // render chromatric aberration
  RB_CameraPostFX();

  // copy to given byte buffer that is NOT a FBO
  if ( tr.refdef.pixelTarget != NULL )
  {
    int i;

    // need to convert Y axis
#if 0
    glReadPixels( 0, 0, tr.refdef.pixelTargetWidth, tr.refdef.pixelTargetHeight, GL_RGBA, GL_UNSIGNED_BYTE, tr.refdef.pixelTarget );
#else
    // Bugfix: drivers absolutely hate running in high res and using glReadPixels near the top or bottom edge.
    // Sooo... let's do it in the middle.
    glReadPixels( glConfig.vidWidth / 2, glConfig.vidHeight / 2, tr.refdef.pixelTargetWidth, tr.refdef.pixelTargetHeight, GL_RGBA,
                  GL_UNSIGNED_BYTE, tr.refdef.pixelTarget );
#endif

    for ( i = 0; i < tr.refdef.pixelTargetWidth * tr.refdef.pixelTargetHeight; i++ )
    {
      tr.refdef.pixelTarget[( i * 4 ) + 3 ] = 255;  //set the alpha pure white
    }
  }

  GL_CheckErrors();

  backEnd.pc.c_views++;
}

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty )
{
  int i, j;
  int start, end;

  if ( !tr.registered )
  {
    return;
  }

  R_SyncRenderThread();

  // we definitely want to sync every frame for the cinematics
  glFinish();

  start = end = 0;

  if ( r_speeds->integer )
  {
    glFinish();
    start = ri.Milliseconds();
  }

  // make sure rows and cols are powers of 2
  for ( i = 0; ( 1 << i ) < cols; i++ )
  {
  }

  for ( j = 0; ( 1 << j ) < rows; j++ )
  {
  }

  if ( ( 1 << i ) != cols || ( 1 << j ) != rows )
  {
    ri.Error( ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows );
  }

  RB_SetGL2D();

  glVertexAttrib4f( ATTR_INDEX_NORMAL, 0, 0, 1, 1 );
  glVertexAttrib4f( ATTR_INDEX_COLOR, tr.identityLight, tr.identityLight, tr.identityLight, 1 );

  gl_genericShader->DisableVertexSkinning();
  gl_genericShader->DisableVertexAnimation();
  gl_genericShader->DisableDeformVertexes();
  gl_genericShader->DisableTCGenEnvironment();
  ////////////gl_genericShader->DisableTCGenLightmap(); //daemon .5
  //gl_genericShader->DisableDepthFade(); //daemon .5
  //gl_genericShader->DisableAlphaTesting(); //daemon .5

  gl_genericShader->BindProgram();

  // set uniforms
  //gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
  gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
  gl_genericShader->SetUniform_Color( colorBlack );

  gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

  // bind u_ColorMap
  GL_BindToTMU( 0, tr.scratchImage[ client ] );
  gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

  // if the scratchImage isn't in the format we want, specify it as a new texture
  if ( cols != tr.scratchImage[ client ]->width || rows != tr.scratchImage[ client ]->height )
  {
    tr.scratchImage[ client ]->width = tr.scratchImage[ client ]->uploadWidth = cols;
    tr.scratchImage[ client ]->height = tr.scratchImage[ client ]->uploadHeight = rows;

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  }
  else
  {
    if ( dirty )
    {
      // otherwise, just subimage upload it so that drivers can tell we are going to be changing
      // it and don't try and do a texture compression
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
    }
  }

  if ( r_speeds->integer )
  {
    glFinish();
    end = ri.Milliseconds();
    ri.Printf( PRINT_DEVELOPER, "glTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
  }

  /*
     glBegin(GL_QUADS);
     glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 0.5f / cols, 0.5f / rows, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_POSITION, x, y, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, (cols - 0.5f) / cols, 0.5f / rows, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_POSITION, x + w, y, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, (cols - 0.5f) / cols, (rows - 0.5f) / rows, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_POSITION, x + w, y + h, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 0.5f / cols, (rows - 0.5f) / rows, 0, 1);
     glVertexAttrib4f(ATTR_INDEX_POSITION, x, y + h, 0, 1);
     glEnd();
   */

  tess.multiDrawPrimitives = 0;
  tess.numVertexes = 0;
  tess.numIndexes = 0;

  tess.xyz[ tess.numVertexes ][ 0 ] = x;
  tess.xyz[ tess.numVertexes ][ 1 ] = y;
  tess.xyz[ tess.numVertexes ][ 2 ] = 0;
  tess.xyz[ tess.numVertexes ][ 3 ] = 1;
  tess.texCoords[ tess.numVertexes ][ 0 ] = 0.5f / cols;
  tess.texCoords[ tess.numVertexes ][ 1 ] = 0.5f / rows;
  tess.numVertexes++;

  tess.xyz[ tess.numVertexes ][ 0 ] = x + w;
  tess.xyz[ tess.numVertexes ][ 1 ] = y;
  tess.xyz[ tess.numVertexes ][ 2 ] = 0;
  tess.xyz[ tess.numVertexes ][ 3 ] = 1;
  tess.texCoords[ tess.numVertexes ][ 0 ] = ( cols - 0.5f ) / cols;
  tess.texCoords[ tess.numVertexes ][ 1 ] = 0.5f / rows;
  tess.numVertexes++;

  tess.xyz[ tess.numVertexes ][ 0 ] = x + w;
  tess.xyz[ tess.numVertexes ][ 1 ] = y + h;
  tess.xyz[ tess.numVertexes ][ 2 ] = 0;
  tess.xyz[ tess.numVertexes ][ 3 ] = 1;
  tess.texCoords[ tess.numVertexes ][ 0 ] = ( cols - 0.5f ) / cols;
  tess.texCoords[ tess.numVertexes ][ 1 ] = ( rows - 0.5f ) / rows;
  tess.numVertexes++;

  tess.xyz[ tess.numVertexes ][ 0 ] = x;
  tess.xyz[ tess.numVertexes ][ 1 ] = y + h;
  tess.xyz[ tess.numVertexes ][ 2 ] = 0;
  tess.xyz[ tess.numVertexes ][ 3 ] = 1;
  tess.texCoords[ tess.numVertexes ][ 0 ] = 0.5f / cols;
  tess.texCoords[ tess.numVertexes ][ 1 ] = ( rows - 0.5f ) / rows;
  tess.numVertexes++;

  tess.indexes[ tess.numIndexes++ ] = 0;
  tess.indexes[ tess.numIndexes++ ] = 1;
  tess.indexes[ tess.numIndexes++ ] = 2;
  tess.indexes[ tess.numIndexes++ ] = 0;
  tess.indexes[ tess.numIndexes++ ] = 2;
  tess.indexes[ tess.numIndexes++ ] = 3;

  Tess_UpdateVBOs( ATTR_POSITION | ATTR_TEXCOORD );

  Tess_DrawElements();

  tess.multiDrawPrimitives = 0;
  tess.numVertexes = 0;
  tess.numIndexes = 0;

  GL_CheckErrors();
}

void RE_UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty )
{
  R_SyncRenderThread();

  GL_Bind( tr.scratchImage[ client ] );

  // if the scratchImage isn't in the format we want, specify it as a new texture
  if ( cols != tr.scratchImage[ client ]->width || rows != tr.scratchImage[ client ]->height )
  {
    tr.scratchImage[ client ]->width = tr.scratchImage[ client ]->uploadWidth = cols;
    tr.scratchImage[ client ]->height = tr.scratchImage[ client ]->uploadHeight = rows;

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colorBlack );
  }
  else
  {
    if ( dirty )
    {
      // otherwise, just subimage upload it so that drivers can tell we are going to be changing
      // it and don't try and do a texture compression
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
    }
  }

  GL_CheckErrors();
}

/*
=============
RB_SetColor
=============
*/
const void     *RB_SetColor( const void *data )
{
  const setColorCommand_t *cmd;

  GLimp_LogComment( "--- RB_SetColor ---\n" );

  cmd = ( const setColorCommand_t * ) data;

  backEnd.color2D[ 0 ] = cmd->color[ 0 ];
  backEnd.color2D[ 1 ] = cmd->color[ 1 ];
  backEnd.color2D[ 2 ] = cmd->color[ 2 ];
  backEnd.color2D[ 3 ] = cmd->color[ 3 ];

  return ( const void * )( cmd + 1 );
}

/*
=============
RB_SetColorGrading
=============
*/
const void *RB_SetColorGrading( const void *data )
{
  const setColorGradingCommand_t *cmd;

  GLimp_LogComment( "--- RB_SetColorGrading ---\n" );

  cmd = ( const setColorGradingCommand_t * ) data;

  GL_Bind( cmd->image );

  glBindBuffer( GL_PIXEL_PACK_BUFFER, tr.colorGradePBO );

  glGetTexImage( cmd->image->type, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
  glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

  glBindBuffer( GL_PIXEL_UNPACK_BUFFER, tr.colorGradePBO );

  GL_Bind( tr.colorGradeImage );

  if ( cmd->image->width == REF_COLORGRADEMAP_SIZE )
  {
    glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, cmd->slot * REF_COLORGRADEMAP_SIZE,
                     REF_COLORGRADEMAP_SIZE, REF_COLORGRADEMAP_SIZE, REF_COLORGRADEMAP_SIZE,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL );
  }
  else
  {
    int i;

    glPixelStorei( GL_UNPACK_ROW_LENGTH, REF_COLORGRADEMAP_SIZE * REF_COLORGRADEMAP_SIZE );

    for ( i = 0; i < 16; i++ )
    {
      glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, i + cmd->slot * REF_COLORGRADEMAP_SIZE,
                       REF_COLORGRADEMAP_SIZE, REF_COLORGRADEMAP_SIZE, 1,
                       GL_RGBA, GL_UNSIGNED_BYTE, ( ( color4ub_t * ) NULL ) + REF_COLORGRADEMAP_SIZE );
    }

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
  }

  glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

  return ( const void * ) ( cmd + 1 );
}

/*
=============
RB_StretchPic
=============
*/
const void     *RB_StretchPic( const void *data )
{
  int                       i;
  const stretchPicCommand_t *cmd;
  shader_t                  *shader;
  int                       numVerts, numIndexes;

  GLimp_LogComment( "--- RB_StretchPic ---\n" );

  cmd = ( const stretchPicCommand_t * ) data;

  if ( !backEnd.projection2D )
  {
    RB_SetGL2D();
  }

  shader = cmd->shader;

  if ( shader != tess.surfaceShader )
  {
    if ( tess.numIndexes )
    {
      Tess_End();
    }

    backEnd.currentEntity = &backEnd.entity2D;
    Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, -1, 0 );
  }
#if 0
  if( !tess.indexes ) { //daemon .50
    Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, -1, 0 );
  }
#endif

  Tess_CheckOverflow( 4, 6 );
  numVerts = tess.numVertexes;
  numIndexes = tess.numIndexes;

  tess.numVertexes += 4;
  tess.numIndexes += 6;

  tess.indexes[ numIndexes ] = numVerts + 3;
  tess.indexes[ numIndexes + 1 ] = numVerts + 0;
  tess.indexes[ numIndexes + 2 ] = numVerts + 2;
  tess.indexes[ numIndexes + 3 ] = numVerts + 2;
  tess.indexes[ numIndexes + 4 ] = numVerts + 0;
  tess.indexes[ numIndexes + 5 ] = numVerts + 1;

  for ( i = 0; i < 4; i++ )
  {
    tess.lightColor[ numVerts + i ][ 0 ] = backEnd.color2D[ 0 ];
    tess.lightColor[ numVerts + i ][ 1 ] = backEnd.color2D[ 1 ];
    tess.lightColor[ numVerts + i ][ 2 ] = backEnd.color2D[ 2 ];
    tess.lightColor[ numVerts + i ][ 3 ] = backEnd.color2D[ 3 ];
  }

  tess.xyz[ numVerts ][ 0 ] = cmd->x;
  tess.xyz[ numVerts ][ 1 ] = cmd->y;
  tess.xyz[ numVerts ][ 2 ] = 0;
  tess.xyz[ numVerts ][ 3 ] = 1;

  tess.texCoords[ numVerts ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 1 ][ 0 ] = cmd->x + cmd->w;
  tess.xyz[ numVerts + 1 ][ 1 ] = cmd->y;
  tess.xyz[ numVerts + 1 ][ 2 ] = 0;
  tess.xyz[ numVerts + 1 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 1 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 1 ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 2 ][ 0 ] = cmd->x + cmd->w;
  tess.xyz[ numVerts + 2 ][ 1 ] = cmd->y + cmd->h;
  tess.xyz[ numVerts + 2 ][ 2 ] = 0;
  tess.xyz[ numVerts + 2 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 2 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 2 ][ 1 ] = cmd->t2;

  tess.xyz[ numVerts + 3 ][ 0 ] = cmd->x;
  tess.xyz[ numVerts + 3 ][ 1 ] = cmd->y + cmd->h;
  tess.xyz[ numVerts + 3 ][ 2 ] = 0;
  tess.xyz[ numVerts + 3 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 3 ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts + 3 ][ 1 ] = cmd->t2;

  tess.attribsSet |= ATTR_POSITION | ATTR_COLOR | ATTR_TEXCOORD;

  return ( const void * )( cmd + 1 );
}

const void     *RB_ScissorEnable( const void *data )
{
  const scissorEnableCommand_t *cmd;

  cmd = ( const scissorEnableCommand_t * ) data;

  tr.scissor.status = cmd->enable;

  if ( !cmd->enable )
  {
    Tess_End();
    GL_Scissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
  }
  else
  {
    Tess_End();
    GL_Scissor( tr.scissor.x, tr.scissor.y, tr.scissor.w, tr.scissor.h );
  }

  return ( const void * )( cmd + 1 );
}

const void     *RB_ScissorSet( const void *data )
{
  const scissorSetCommand_t *cmd;

  cmd = ( const scissorSetCommand_t * ) data;

  tr.scissor.x = cmd->x;
  tr.scissor.y = cmd->y;
  tr.scissor.w = cmd->w;
  tr.scissor.h = cmd->h;

  if (tr.scissor.status )
  {
      Tess_End();
      GL_Scissor( cmd->x, cmd->y, cmd->w, cmd->h );
  }

    return ( const void * )( cmd + 1 );
}

const void     *RB_Draw2dPolys( const void *data )
{
  const poly2dCommand_t *cmd;
  shader_t              *shader;
  int                   i;

  cmd = ( const poly2dCommand_t * ) data;

  if ( !backEnd.projection2D )
  {
    RB_SetGL2D();
  }

  shader = cmd->shader;

  if ( shader != tess.surfaceShader )
  {
    if ( tess.numIndexes )
    {
      Tess_End();
    }

    backEnd.currentEntity = &backEnd.entity2D;
    Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, -1, 0 );
  }

  Tess_CheckOverflow( cmd->numverts, ( cmd->numverts - 2 ) * 3 );

  for ( i = 0; i < cmd->numverts - 2; i++ )
  {
    tess.indexes[ tess.numIndexes + 0 ] = tess.numVertexes;
    tess.indexes[ tess.numIndexes + 1 ] = tess.numVertexes + i + 1;
    tess.indexes[ tess.numIndexes + 2 ] = tess.numVertexes + i + 2;
    tess.numIndexes += 3;
  }

  for ( i = 0; i < cmd->numverts; i++ )
  {
    tess.xyz[ tess.numVertexes ][ 0 ] = cmd->verts[ i ].xyz[ 0 ];
    tess.xyz[ tess.numVertexes ][ 1 ] = cmd->verts[ i ].xyz[ 1 ];
    tess.xyz[ tess.numVertexes ][ 2 ] = 0;
    tess.xyz[ tess.numVertexes ][ 3 ] = 1;

    tess.texCoords[ tess.numVertexes ][ 0 ] = cmd->verts[ i ].st[ 0 ];
    tess.texCoords[ tess.numVertexes ][ 1 ] = cmd->verts[ i ].st[ 1 ];

    tess.lightColor[ tess.numVertexes ][ 0 ] = cmd->verts[ i ].modulate[ 0 ] * ( 1.0 / 255.0f );
    tess.lightColor[ tess.numVertexes ][ 1 ] = cmd->verts[ i ].modulate[ 1 ] * ( 1.0 / 255.0f );
    tess.lightColor[ tess.numVertexes ][ 2 ] = cmd->verts[ i ].modulate[ 2 ] * ( 1.0 / 255.0f );
    tess.lightColor[ tess.numVertexes ][ 3 ] = cmd->verts[ i ].modulate[ 3 ] * ( 1.0 / 255.0f );
    tess.numVertexes++;
  }

  tess.attribsSet |= ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR;
  return ( const void * )( cmd + 1 );
}

// NERVE - SMF

/*
=============
RB_RotatedPic
=============
*/
const void     *RB_RotatedPic( const void *data )
{
  const stretchPicCommand_t *cmd;
  shader_t                  *shader;
  int                       numVerts, numIndexes;
  float                     mx, my, cosA, sinA, cw, ch, sw, sh;

  cmd = ( const stretchPicCommand_t * ) data;

  if ( !backEnd.projection2D )
  {
    RB_SetGL2D();
  }

  shader = cmd->shader;

  if ( shader != tess.surfaceShader )
  {
    if ( tess.numIndexes )
    {
      Tess_End();
    }

    backEnd.currentEntity = &backEnd.entity2D;
    Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, -1, 0 );
  }

  Tess_CheckOverflow( 4, 6 );
  numVerts = tess.numVertexes;
  numIndexes = tess.numIndexes;

  tess.numVertexes += 4;
  tess.numIndexes += 6;

  tess.indexes[ numIndexes ] = numVerts + 3;
  tess.indexes[ numIndexes + 1 ] = numVerts + 0;
  tess.indexes[ numIndexes + 2 ] = numVerts + 2;
  tess.indexes[ numIndexes + 3 ] = numVerts + 2;
  tess.indexes[ numIndexes + 4 ] = numVerts + 0;
  tess.indexes[ numIndexes + 5 ] = numVerts + 1;

  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 0 ] );
  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 1 ] );
  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 2 ] );
  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 3 ] );

  mx = cmd->x + ( cmd->w / 2 );
  my = cmd->y + ( cmd->h / 2 );
  cosA = cos( DEG2RAD( cmd->angle ) );
  sinA = sin( DEG2RAD( cmd->angle ) );
  cw = cosA * ( cmd->w / 2 );
  ch = cosA * ( cmd->h / 2 );
  sw = sinA * ( cmd->w / 2 );
  sh = sinA * ( cmd->h / 2 );

  tess.xyz[ numVerts ][ 0 ] = mx - cw - sh;
  tess.xyz[ numVerts ][ 1 ] = my + sw - ch;
  tess.xyz[ numVerts ][ 2 ] = 0;
  tess.xyz[ numVerts ][ 3 ] = 1;

  tess.texCoords[ numVerts ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 1 ][ 0 ] = mx + cw - sh;
  tess.xyz[ numVerts + 1 ][ 1 ] = my - sw - ch;
  tess.xyz[ numVerts + 1 ][ 2 ] = 0;
  tess.xyz[ numVerts + 1 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 1 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 1 ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 2 ][ 0 ] = mx + cw + sh;
  tess.xyz[ numVerts + 2 ][ 1 ] = my - sw + ch;
  tess.xyz[ numVerts + 2 ][ 2 ] = 0;
  tess.xyz[ numVerts + 2 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 2 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 2 ][ 1 ] = cmd->t2;

  tess.xyz[ numVerts + 3 ][ 0 ] = mx - cw + sh;
  tess.xyz[ numVerts + 3 ][ 1 ] = my + sw + ch;
  tess.xyz[ numVerts + 3 ][ 2 ] = 0;
  tess.xyz[ numVerts + 3 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 3 ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts + 3 ][ 1 ] = cmd->t2;

  tess.attribsSet |= ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR;

  return ( const void * )( cmd + 1 );
}

// -NERVE - SMF

/*
==============
RB_StretchPicGradient
==============
*/
const void     *RB_StretchPicGradient( const void *data )
{
  const stretchPicCommand_t *cmd;
  shader_t                  *shader;
  int                       numVerts, numIndexes;
  int                       i;

  cmd = ( const stretchPicCommand_t * ) data;

  if ( !backEnd.projection2D )
  {
    RB_SetGL2D();
  }

  shader = cmd->shader;

  if ( shader != tess.surfaceShader )
  {
    if ( tess.numIndexes )
    {
      Tess_End();
    }

    backEnd.currentEntity = &backEnd.entity2D;
    Tess_Begin( Tess_StageIteratorGeneric, NULL, shader, NULL, qfalse, qfalse, -1, 0 );
  }

  Tess_CheckOverflow( 4, 6 );
  numVerts = tess.numVertexes;
  numIndexes = tess.numIndexes;

  tess.numVertexes += 4;
  tess.numIndexes += 6;

  tess.indexes[ numIndexes ] = numVerts + 3;
  tess.indexes[ numIndexes + 1 ] = numVerts + 0;
  tess.indexes[ numIndexes + 2 ] = numVerts + 2;
  tess.indexes[ numIndexes + 3 ] = numVerts + 2;
  tess.indexes[ numIndexes + 4 ] = numVerts + 0;
  tess.indexes[ numIndexes + 5 ] = numVerts + 1;

  //*(int *)tess.vertexColors[numVerts].v = *(int *)tess.vertexColors[numVerts + 1].v = *(int *)backEnd.color2D;
  //*(int *)tess.vertexColors[numVerts + 2].v = *(int *)tess.vertexColors[numVerts + 3].v = *(int *)cmd->gradientColor;

  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 0 ] );
  Vector4Copy( backEnd.color2D, tess.lightColor[ numVerts + 1 ] );

  for ( i = 0; i < 4; i++ )
  {
    tess.lightColor[ numVerts + 2 ][ i ] = cmd->gradientColor[ i ] * ( 1.0f / 255.0f );
    tess.lightColor[ numVerts + 3 ][ i ] = cmd->gradientColor[ i ] * ( 1.0f / 255.0f );
  }

  tess.xyz[ numVerts ][ 0 ] = cmd->x;
  tess.xyz[ numVerts ][ 1 ] = cmd->y;
  tess.xyz[ numVerts ][ 2 ] = 0;
  tess.xyz[ numVerts ][ 3 ] = 1;

  tess.texCoords[ numVerts ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 1 ][ 0 ] = cmd->x + cmd->w;
  tess.xyz[ numVerts + 1 ][ 1 ] = cmd->y;
  tess.xyz[ numVerts + 1 ][ 2 ] = 0;
  tess.xyz[ numVerts + 1 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 1 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 1 ][ 1 ] = cmd->t1;

  tess.xyz[ numVerts + 2 ][ 0 ] = cmd->x + cmd->w;
  tess.xyz[ numVerts + 2 ][ 1 ] = cmd->y + cmd->h;
  tess.xyz[ numVerts + 2 ][ 2 ] = 0;
  tess.xyz[ numVerts + 2 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 2 ][ 0 ] = cmd->s2;
  tess.texCoords[ numVerts + 2 ][ 1 ] = cmd->t2;

  tess.xyz[ numVerts + 3 ][ 0 ] = cmd->x;
  tess.xyz[ numVerts + 3 ][ 1 ] = cmd->y + cmd->h;
  tess.xyz[ numVerts + 3 ][ 2 ] = 0;
  tess.xyz[ numVerts + 3 ][ 3 ] = 1;

  tess.texCoords[ numVerts + 3 ][ 0 ] = cmd->s1;
  tess.texCoords[ numVerts + 3 ][ 1 ] = cmd->t2;

  tess.attribsSet |= ATTR_POSITION | ATTR_TEXCOORD | ATTR_COLOR;
  return ( const void * )( cmd + 1 );
}

/*
=============
RB_DrawView
=============
*/
const void     *RB_DrawView( const void *data )
{
  const drawViewCommand_t *cmd;

  GLimp_LogComment( "--- RB_DrawView ---\n" );

  // finish any 2D drawing if needed
  if ( tess.numIndexes )
  {
    Tess_End();
  }

  cmd = ( const drawViewCommand_t * ) data;

  backEnd.refdef = cmd->refdef;
  backEnd.viewParms = cmd->viewParms;

  RB_RenderView();

  return ( const void * )( cmd + 1 );
}

/*
=============
RB_RunVisTests

=============
*/
const void *RB_RunVisTests( const void *data )
{
  const runVisTestsCommand_t *cmd;
  int i;

  // finish any 2D drawing if needed
  if ( tess.numIndexes )
  {
    Tess_End();
  }

  cmd = ( const runVisTestsCommand_t * ) data;

  backEnd.refdef = cmd->refdef;
  backEnd.viewParms = cmd->viewParms;

  for ( i = 0; i < backEnd.refdef.numVisTests; i++ )
  {
    vec3_t           diff;
    vec3_t           center, left, up;
    visTestResult_t  *test = &backEnd.refdef.visTests[ i ];
    visTestQueries_t *testState = &backEnd.visTestQueries[ test->visTestHandle - 1 ];

    if ( testState->running && !test->discardExisting )
    {
      GLint  available;
      GLuint result, resultRef;

      glGetQueryObjectiv( testState->hQuery,
              GL_QUERY_RESULT_AVAILABLE,
              &available );
      if( !available )
      {
        continue;
      }

      glGetQueryObjectiv( testState->hQueryRef,
              GL_QUERY_RESULT_AVAILABLE,
              &available );
      if ( !available )
      {
        continue;
      }

      glGetQueryObjectuiv( testState->hQueryRef, GL_QUERY_RESULT,
               &resultRef );
      glGetQueryObjectuiv( testState->hQuery, GL_QUERY_RESULT,
               &result );

      if ( resultRef > 0 )
      {
        test->lastResult = (float)result / (float)resultRef;
      }
      else
      {
        test->lastResult = 0.0f;
      }

      testState->running = qfalse;
    }

    VectorSubtract( backEnd.orientation.viewOrigin,
        test->position, diff );
    VectorNormalize( diff );
    VectorMA( test->position, test->depthAdjust, diff, center );

    VectorScale( backEnd.viewParms.orientation.axis[ 1 ],
           test->area, left );
    VectorScale( backEnd.viewParms.orientation.axis[ 2 ],
           test->area, up );

    tess.xyz[ 0 ][ 0 ] = center[ 0 ] + left[ 0 ] + up[ 0 ];
    tess.xyz[ 0 ][ 1 ] = center[ 1 ] + left[ 1 ] + up[ 1 ];
    tess.xyz[ 0 ][ 2 ] = center[ 2 ] + left[ 2 ] + up[ 2 ];
    tess.xyz[ 0 ][ 3 ] = 1.0f;
    tess.xyz[ 1 ][ 0 ] = center[ 0 ] - left[ 0 ] + up[ 0 ];
    tess.xyz[ 1 ][ 1 ] = center[ 1 ] - left[ 1 ] + up[ 1 ];
    tess.xyz[ 1 ][ 2 ] = center[ 2 ] - left[ 2 ] + up[ 2 ];
    tess.xyz[ 1 ][ 3 ] = 1.0f;
    tess.xyz[ 2 ][ 0 ] = center[ 0 ] - left[ 0 ] - up[ 0 ];
    tess.xyz[ 2 ][ 1 ] = center[ 1 ] - left[ 1 ] - up[ 1 ];
    tess.xyz[ 2 ][ 2 ] = center[ 2 ] - left[ 2 ] - up[ 2 ];
    tess.xyz[ 2 ][ 3 ] = 1.0f;
    tess.xyz[ 3 ][ 0 ] = center[ 0 ] + left[ 0 ] - up[ 0 ];
    tess.xyz[ 3 ][ 1 ] = center[ 1 ] + left[ 1 ] - up[ 1 ];
    tess.xyz[ 3 ][ 2 ] = center[ 2 ] + left[ 2 ] - up[ 2 ];
    tess.xyz[ 3 ][ 3 ] = 1.0f;
    tess.numVertexes = 4;

    tess.indexes[ 0 ] = 0;
    tess.indexes[ 1 ] = 1;
    tess.indexes[ 2 ] = 2;
    tess.indexes[ 3 ] = 0;
    tess.indexes[ 4 ] = 2;
    tess.indexes[ 5 ] = 3;
    tess.numIndexes = 6;

    gl_genericShader->DisableVertexSkinning();
    gl_genericShader->DisableVertexAnimation();
    gl_genericShader->DisableDeformVertexes();
    gl_genericShader->DisableTCGenEnvironment();
    //gl_genericShader->DisableTCGenLightmap();

    gl_genericShader->BindProgram();

    gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
    gl_genericShader->SetUniform_Color( colorWhite );

    gl_genericShader->SetUniform_ColorModulate( CGEN_CONST, AGEN_CONST );

    gl_genericShader->SetUniform_ModelMatrix( backEnd.orientation.transformMatrix );
    gl_genericShader->SetUniform_ModelViewProjectionMatrix( glState.modelViewProjectionMatrix[ glState.stackIndex ] );

    // bind u_ColorMap
    GL_BindToTMU( 0, tr.whiteImage );
    gl_genericShader->SetUniform_ColorTextureMatrix( tess.svars.texMatrices[ TB_COLORMAP ] );

    Tess_UpdateVBOs( ATTR_POSITION );

    GL_State( GLS_DEPTHTEST_DISABLE | GLS_COLORMASK_BITS );
    glBeginQuery( GL_SAMPLES_PASSED, testState->hQueryRef );
    Tess_DrawElements();
    glEndQuery( GL_SAMPLES_PASSED );

    GL_State( GLS_COLORMASK_BITS );
    glBeginQuery( GL_SAMPLES_PASSED, testState->hQuery );
    Tess_DrawElements();
    glEndQuery( GL_SAMPLES_PASSED );

    tess.numIndexes = 0;
    tess.numVertexes = 0;
    tess.multiDrawPrimitives = 0;
    testState->running = qtrue;
  }

  return ( const void * )( cmd + 1 );
}

/*
=============
RB_DrawBuffer
=============
*/
const void     *RB_DrawBuffer( const void *data )
{
  const drawBufferCommand_t *cmd;

  GLimp_LogComment( "--- RB_DrawBuffer ---\n" );

  cmd = ( const drawBufferCommand_t * ) data;

  GL_DrawBuffer( cmd->buffer );

  // clear screen for debugging
  if ( r_clear->integer )
  {
//      GL_ClearColor(1, 0, 0.5, 1);
    GL_ClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //backEnd.depthRenderImageValid = qfalse;
  }

  glState.finishCalled = qfalse;
  return ( const void * )( cmd + 1 );
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void )
{
  int     i;
  image_t *image;
  float   x, y, w, h;
  vec4_t  quadVerts[ 4 ];
  int     start, end;

  GLimp_LogComment( "--- RB_ShowImages ---\n" );

  if ( !backEnd.projection2D )
  {
    RB_SetGL2D();
  }

  glClear( GL_COLOR_BUFFER_BIT );

  glFinish();

  gl_genericShader->DisableVertexSkinning();
  gl_genericShader->DisableVertexAnimation();
  gl_genericShader->DisableDeformVertexes();
  gl_genericShader->DisableTCGenEnvironment();
  //////////////gl_genericShader->DisableTCGenLightmap();//daemon 0.50
  //gl_genericShader->DisableDepthFade();;//daemon 0.50
  //gl_genericShader->DisableAlphaTesting();;//daemon 0.50

  gl_genericShader->BindProgram();

  GL_Cull( CT_TWO_SIDED );

  // set uniforms
  gl_genericShader->SetUniform_AlphaTest( GLS_ATEST_NONE );
  gl_genericShader->SetUniform_ColorModulate( CGEN_VERTEX, AGEN_VERTEX );
  gl_genericShader->SetUniform_ColorTextureMatrix( matrixIdentity );

  GL_SelectTexture( 0 );

  start = ri.Milliseconds();

  for ( i = 0; i < tr.images.currentElements; i++ )
  {
    image = ( image_t * ) Com_GrowListElement( &tr.images, i );

    /*
       if(image->bits & (IF_RGBA16F | IF_RGBA32F | IF_LA16F | IF_LA32F))
       {
       // don't render float textures using FFP
       continue;
       }
     */

    w = glConfig.vidWidth / 20;
    h = glConfig.vidHeight / 15;
    x = i % 20 * w;
    y = i / 20 * h;

    // show in proportional size in mode 2
    if ( r_showImages->integer == 2 )
    {
      w *= image->uploadWidth / 512.0f;
      h *= image->uploadHeight / 512.0f;
    }

    // bind u_ColorMap
    GL_Bind( image );

    Vector4Set(quadVerts[0], x, y, 0.f, 1.f);
    Vector4Set(quadVerts[1], x + w, y, 0.f, 1.f);
    Vector4Set(quadVerts[2], x + w, y + h, 0.f, 1.f);
    Vector4Set(quadVerts[3], x, y + h, 0.f, 1.f);

    Tess_InstantQuad( quadVerts );

    /*
       glBegin(GL_QUADS);
       glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 0, 0, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_POSITION, x, y, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 1, 0, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_POSITION, x + w, y, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 1, 1, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_POSITION, x + w, y + h, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_TEXCOORD0, 0, 1, 0, 1);
       glVertexAttrib4f(ATTR_INDEX_POSITION, x, y + h, 0, 1);
       glEnd();
     */
  }

  glFinish();

  end = ri.Milliseconds();
  ri.Printf( PRINT_DEVELOPER, "%i msec to draw all images\n", end - start );

  GL_CheckErrors();
}

/*
=============
RB_SwapBuffers
=============
*/
const void     *RB_SwapBuffers( const void *data )
{
  const swapBuffersCommand_t *cmd;

  // finish any 2D drawing if needed
  if ( tess.numIndexes )
  {
    Tess_End();
  }

  // texture swapping test
  if ( r_showImages->integer )
  {
    RB_ShowImages();
  }

  cmd = ( const swapBuffersCommand_t * ) data;

  // we measure overdraw by reading back the stencil buffer and
  // counting up the number of increments that have happened
  if ( r_measureOverdraw->integer )
  {
    int           i;
    long          sum = 0;
    unsigned char *stencilReadback;

    stencilReadback = ( unsigned char * ) ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
    glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

    for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ )
    {
      sum += stencilReadback[ i ];
    }

    backEnd.pc.c_overDraw += sum;
    ri.Hunk_FreeTempMemory( stencilReadback );
  }
#if 1
  if ( !glState.finishCalled )
  {
    glFinish();
  }
#endif
  GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

  GLimp_EndFrame();

  backEnd.projection2D = qfalse;

  return ( const void * )( cmd + 1 );
}

//bani

/*
=============
RB_RenderToTexture
=============
*/
const void     *RB_RenderToTexture( const void *data )
{
  const renderToTextureCommand_t *cmd;

//  ri.Printf( PRINT_ALL, "RB_RenderToTexture\n" );

  cmd = ( const renderToTextureCommand_t * ) data;

  GL_Bind( cmd->image );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
  glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, cmd->x, cmd->y, cmd->w, cmd->h, 0 );
//  glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cmd->x, cmd->y, cmd->w, cmd->h );

  return ( const void * )( cmd + 1 );
}

//bani

/*
=============
RB_Finish
=============
*/
const void     *RB_Finish( const void *data )
{
  const renderFinishCommand_t *cmd;

//  ri.Printf( PRINT_ALL, "RB_Finish\n" );

  cmd = ( const renderFinishCommand_t * ) data;

  glFinish();

  return ( const void * )( cmd + 1 );
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data )
{
  int t1, t2;

  GLimp_LogComment( "--- RB_ExecuteRenderCommands ---\n" );

  t1 = ri.Milliseconds();

  if ( !r_smp->integer || data == backEndData[ 0 ]->commands.cmds )
  {
    backEnd.smpFrame = 0;
  }
  else
  {
    backEnd.smpFrame = 1;
  }

  while ( 1 )
  {
    switch ( * ( const int * ) data )
    {
      case RC_SET_COLORGRADING:
        data = RB_SetColorGrading( data );
        break;

      case RC_SET_COLOR:
        data = RB_SetColor( data );
        break;

      case RC_STRETCH_PIC:
        data = RB_StretchPic( data );
        break;

      case RC_2DPOLYS:
        data = RB_Draw2dPolys( data );
        break;

      case RC_ROTATED_PIC:
        data = RB_RotatedPic( data );
        break;

      case RC_STRETCH_PIC_GRADIENT:
        data = RB_StretchPicGradient( data );
        break;

      case RC_DRAW_VIEW:
        data = RB_DrawView( data );
        break;

      case RC_RUN_VISTESTS:
        data = RB_RunVisTests( data );
        break;

      case RC_DRAW_BUFFER:
        data = RB_DrawBuffer( data );
        break;

      case RC_SWAP_BUFFERS:
        data = RB_SwapBuffers( data );
        break;

      case RC_SCREENSHOT:
        data = RB_TakeScreenshotCmd( data );
        break;

      case RC_VIDEOFRAME:
        data = RB_TakeVideoFrameCmd( data );
        break;

      case RC_RENDERTOTEXTURE:
        data = RB_RenderToTexture( data );
        break;

      case RC_FINISH:
        data = RB_Finish( data );
        break;

      case RC_SCISSORENABLE:
        data = RB_ScissorEnable( data );
        break;

      case RC_SCISSORSET:
        data = RB_ScissorSet( data );
        break;

      case RC_END_OF_LIST:
      default:
        // stop rendering on this thread
        t2 = ri.Milliseconds();
        backEnd.pc.msec = t2 - t1;
        return;
    }
  }
}

#if 1 //daemon
/*
=============
R_ShutdownBackend
=============
*/
void R_ShutdownBackend()
{
  int i;

  for ( i = 0; i < ATTR_INDEX_MAX; i++ )
  {
    glDisableVertexAttribArray( i );
  }
  glState.vertexAttribsState = 0;
}
#endif


/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void )
{
  const void *data;

  // wait for either a rendering command or a quit command
  while ( 1 )
  {
    // sleep until we have work to do
    data = GLimp_RendererSleep();

    if ( !data )
    {
      return; // all done, renderer is shutting down
    }

    renderThreadActive = qtrue;

    RB_ExecuteRenderCommands( data );

    renderThreadActive = qfalse;
  }
}
