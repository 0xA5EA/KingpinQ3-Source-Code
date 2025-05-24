/*
===========================================================================
Copyright (C) 2008-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

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

/* vertexLighting_DBS_world_vp.glsl */

attribute vec3 		attr_Position;
attribute vec2 		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;
attribute vec4		attr_Color;
attribute vec3		attr_AmbientLight;
attribute vec3		attr_DirectedLight;
attribute vec3		attr_LightDirection;

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
uniform mat4		u_GlowTextureMatrix;
uniform mat4		u_ModelViewProjectionMatrix;

uniform float		u_Time;

uniform vec4		u_ColorModulate;
uniform vec4		u_Color;
uniform vec3		u_ViewOrigin;

varying vec4		var_TexDiffuseGlow;
varying vec3		var_Position; //beta1

#if defined(USE_NORMAL_MAPPING)
varying vec4		var_TexNormalSpecular;
varying vec3		var_ViewDir;
varying vec3		var_AmbientLight;
varying vec3		var_Tangent; //hypov8 add
varying vec3		var_Binormal; //hypov8 add
#endif
varying vec3		var_Normal;
varying vec4		var_LightColor;
varying vec3		var_DirectedLight;
varying vec3		var_LightDirection;



void	main()
{
	vec4 position = vec4(attr_Position, 1.0);
#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition2(	position,
								attr_Normal,
								attr_TexCoord0.st,
								u_Time);
#endif

	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * position;

	// transform diffusemap texcoords
	var_TexDiffuseGlow.st = (u_DiffuseTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

#if defined(USE_NORMAL_MAPPING)
	// transform normalmap texcoords
	var_TexNormalSpecular.st = (u_NormalTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

	// transform specularmap texture coords
	var_TexNormalSpecular.pq = (u_SpecularTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0)).st;

	var_AmbientLight = attr_AmbientLight;
	var_DirectedLight = attr_DirectedLight;
	
	
	// assign color
	#if 1 //hypov8 add. light direction seems buggy from kmap?
        var_LightDirection = attr_Normal.xyz; //(attr_Normal.xyx + 1.0) / 2;    
    #else
        var_LightDirection = attr_LightDirection.rgb;
	#endif

	
	var_Normal = attr_Normal.xyz;	//add hypov8
	var_Tangent = attr_Tangent.xyz;	//add hypov8
	var_Binormal = attr_Binormal.xyz;	//add hypov8	
	var_Position = position.xyz; // add hypov8
	
#else //!USE_NORMAL_MAPPING
	// assign color

	

	#if defined(r_showDeluxeMaps)
	var_LightDirection = attr_Normal.xyz; //(attr_Normal.xyz + 1.0) / 2; //attr_DirectedLight;
	#endif
	
	var_Normal = attr_Normal.xyz;
#endif  //end !USE_NORMAL_MAPPING

#if defined(USE_GLOW_MAPPING)
	var_TexDiffuseGlow.pq = ( u_GlowTextureMatrix * vec4(attr_TexCoord0, 0.0, 1.0) ).st;
#endif

	var_LightColor = attr_Color * u_ColorModulate + u_Color;
}
