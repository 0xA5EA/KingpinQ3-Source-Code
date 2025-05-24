/*
===========================================================================
Copyright (C) 2006-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/* vertexLighting_DBS_entity_vp.glsl */

attribute vec3 		attr_Position;
attribute vec2 		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;

attribute vec3 		attr_Position2;
attribute vec3		attr_Tangent2;
attribute vec3		attr_Binormal2;
attribute vec3		attr_Normal2;

uniform float		u_VertexInterpolation;

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
uniform mat4		u_GlowTextureMatrix;
uniform mat4		u_ModelMatrix;
uniform mat4		u_ModelViewProjectionMatrix;
uniform mat4		u_ColorTextureMatrix; //hypov8 pbr

uniform float		u_Time;

varying vec3		var_Position;
varying vec2		var_TexDiffuse;
varying vec2		var_TexGlow;

// light/lightgrid..
//uniform vec3		u_AmbientColor; // currently calculated in bsp. ambiant light
uniform vec3		u_LightDir;     // pointlight dir(todo multiply by matrix)
//uniform vec3		u_LightColor;   // lightgrid color(match lightmap)
uniform vec3		u_ViewOrigin; //current eye position

#if defined(USE_NORMAL_MAPPING)
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec3		var_Tangent;
varying vec3		var_Binormal;
 #if defined(USE_REFLECTIVE_SPECULAR)
  varying vec2		var_TexColor; //hypov8 pbr
 #endif
#endif

varying vec3		var_Normal;


void	main()
{
	vec4 position;
	vec3 tangent;
	vec3 binormal;
	vec3 normal;

#if defined(USE_VERTEX_SKINNING)

	#if defined(USE_NORMAL_MAPPING)
	VertexSkinning_P_TBN(	attr_Position, attr_Tangent, attr_Binormal, attr_Normal,
							position, tangent, binormal, normal);
	#else
	VertexSkinning_P_N(	attr_Position, attr_Normal,
						position, normal);
	#endif

#elif defined(USE_VERTEX_ANIMATION)

	#if defined(USE_NORMAL_MAPPING)
	VertexAnimation_P_TBN(	attr_Position, attr_Position2,
							attr_Tangent, attr_Tangent2,
							attr_Binormal, attr_Binormal2,
							attr_Normal, attr_Normal2,
							u_VertexInterpolation,
							position, tangent, binormal, normal);
	#else
	VertexAnimation_P_N(attr_Position, attr_Position2,
						attr_Normal, attr_Normal2,
						u_VertexInterpolation,
						position, normal);
	#endif

#else //!USE_VERTEX_SKINNING && !USE_VERTEX_ANIMATION
	position = vec4(attr_Position, 1.0);

	#if defined(USE_NORMAL_MAPPING)
	tangent = attr_Tangent;
	binormal = attr_Binormal;
	#endif

	normal = attr_Normal;
#endif //end !USE_VERTEX_SKINNING && !USE_VERTEX_ANIMATION

#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition2(	position,
								normal,
								attr_TexCoord0.st,
								u_Time);
#endif

	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * vec4(position.xyz, 1.0);


	// transform position into world space
	var_Position = (u_ModelMatrix * vec4(position.xyz, 1.0)).xyz;


	var_Normal.xyz = (u_ModelMatrix * vec4(normal, 0.0)).xyz;

	// transform diffusemap texcoords
	var_TexDiffuse = (u_DiffuseTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

#if defined(USE_NORMAL_MAPPING)
	// transform normalmap texcoords
	var_TexNormal = (u_NormalTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

	// transform specularmap texture coords
	var_TexSpecular = (u_SpecularTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

	var_Tangent.xyz = (u_ModelMatrix * vec4(tangent, 0.0)).xyz;
	var_Binormal.xyz = (u_ModelMatrix * vec4(binormal, 0.0)).xyz;

	#if defined(USE_REFLECTIVE_SPECULAR) //hypov8 pbr
		var_TexColor = (u_ColorTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;
	#endif
#endif

#if defined(USE_GLOW_MAPPING)
	var_TexGlow = (u_GlowTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;
#endif
}
