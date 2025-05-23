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

/* forwardLighting_vp.glsl */

attribute vec3 		attr_Position;
attribute vec2 		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;
attribute vec4		attr_Color;

attribute vec3 		attr_Position2;
attribute vec3		attr_Tangent2;
attribute vec3		attr_Binormal2;
attribute vec3		attr_Normal2;

uniform float		u_VertexInterpolation;

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
uniform mat4		u_GlowTextureMatrix; //hypov8 add
uniform vec4		u_ColorModulate;
uniform vec4		u_Color;

uniform mat4		u_LightAttenuationMatrix;
uniform mat4		u_ModelMatrix;
uniform mat4		u_ModelViewProjectionMatrix;

uniform float		u_Time;

varying vec3		var_Position;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
varying vec2		var_TexGlow; //hypov8 add
#if defined(USE_NORMAL_MAPPING)
varying vec2		var_TexSpecular;
#endif

varying vec4		var_TexAttenuation;

#if defined(USE_NORMAL_MAPPING)
varying vec4		var_Tangent;
varying vec4		var_Binormal;
#endif
varying vec4		var_Normal;
//varying vec4		var_Color;	// Tr3B - maximum vars reached


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

#else
	position = vec4(attr_Position, 1.0);

	#if defined(USE_NORMAL_MAPPING)
	tangent = attr_Tangent;
	binormal = attr_Binormal;
	#endif

	normal = attr_Normal;
#endif

#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition2(	position,
								normal,
								attr_TexCoord0.st,
								u_Time);
#endif

	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * position;

	// transform position into world space
	var_Position = (u_ModelMatrix * position).xyz;

#if defined(USE_NORMAL_MAPPING)
	var_Tangent.xyz = (u_ModelMatrix * vec4(tangent, 0.0)).xyz;
	var_Binormal.xyz = (u_ModelMatrix * vec4(binormal, 0.0)).xyz;
#endif

	var_Normal.xyz = mat3(u_ModelMatrix) * normal;

	// calc light xy,z attenuation in light space
	var_TexAttenuation = u_LightAttenuationMatrix * position;

	// transform diffusemap texcoords
	var_TexDiffuse.xy = (u_DiffuseTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

#if defined(USE_NORMAL_MAPPING)
	// transform normalmap texcoords
	var_TexNormal.xy = (u_NormalTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

	// transform specularmap texture coords
	var_TexSpecular = (u_SpecularTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;
#endif

	// assign color
	vec4 color = attr_Color * u_ColorModulate + u_Color;

	var_TexDiffuse.p = color.r;
	var_TexNormal.pq = color.gb;

#if defined(USE_GLOW_MAPPING)
	var_TexGlow = ( u_GlowTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0) ).st;
#endif
}
