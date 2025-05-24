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

/* vertexLighting_DBS_world_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_GlowMap;

uniform float		u_AlphaThreshold;
uniform float		u_DepthScale;
uniform	float		u_LightWrapAround;
uniform vec2		u_SpecularExponent;

varying vec4		var_TexDiffuseGlow;
varying vec4		var_TexNormalSpecular;
varying vec3		var_Position; //beta1
uniform vec3		u_ViewOrigin; //hypov8 add

#if defined(USE_NORMAL_MAPPING)
varying vec3		var_ViewDir; // direction from surface to viewer
varying vec3		var_AmbientLight;
varying vec3		var_DirectedLight; //lightgrid angle
varying vec3		var_Tangent;
varying vec3		var_Binormal;
#endif
varying vec4		var_LightColor;
varying vec3		var_Normal;
varying vec3		var_LightDirection; // direction from surface to light

void	main()
{
	vec2 texDiffuse = var_TexDiffuseGlow.st;
	vec2 texNormal = var_TexNormalSpecular.st;
	vec2 texSpecular = var_TexNormalSpecular.pq;    
	vec2 texGlow = var_TexDiffuseGlow.pq;	
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse);

	if( abs(diffuse.a + u_AlphaThreshold) <= 1.0 )
	{
		discard;
		return;
	}	

#if defined(USE_NORMAL_MAPPING)

	// construct object-space-to-tangent-space 3x3 matrix
	mat3 objectToTangentMatrix = mat3( 	var_Tangent.xyx, var_Binormal.xyz, var_Normal.xyz);
	#if defined(TWOSIDED)
		if(gl_FrontFacing)
		{	
			objectToTangentMatrix = -objectToTangentMatrix;
		}
	#endif			
			
	// compute view direction in world space
	vec3 vDir = normalize(u_ViewOrigin - var_Position); //hypov8 most of this is dupe ightMapping

	#if defined(USE_PARALLAX_MAPPING)

		// ray intersect in view direction
        vec3 V = vDir * objectToTangentMatrix; 
        V = normalize(V);        
		
		// size and start position of search in texture space
        vec2 S = V.xy * -u_DepthScale / V.z;
		#if 0
			vec2 texOffset = vec2(0.0);
			for(int i = 0; i < 4; i++) {
				vec4 Normal = texture2D(u_NormalMap, texNormal.st + texOffset);
				float height = Normal.a * 0.2 - 0.0125;
				texOffset += height * Normal.z * S;
			}
		#else
			float depth = RayIntersectDisplaceMap(texNormal, S, u_NormalMap);

			// compute texcoords offset
			vec2 texOffset = S * depth;
		#endif

		texDiffuse.st += texOffset;
		texNormal.st += texOffset;
		texSpecular.st += texOffset;
	#endif // USE_PARALLAX_MAPPING


	// compute normal in tangent space from normalmap
	vec3 N = normalize(objectToTangentMatrix *(2.0 * (texture2D(u_NormalMap, texNormal).xyz - 0.5)));

	
	// compute light direction in tangent space
	vec3 L = var_LightDirection;
	L = normalize(L);	
	
 	// compute half angle in tangent space
	vec3 H = normalize(L + vDir);
    
	// compute light color from vertex color
	vec3 lightColor = var_LightColor.rgb;    
    
	// compute the specular term
	vec4 specBase = texture2D(u_SpecularMap, texSpecular).rgba;
	
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
		
	float specFactor = clamp(dot(normalize(reflect(-L, N)),vDir), 0.04, 1.0);
	specFactor = pow(specFactor, u_SpecularExponent.x * specBase.a + u_SpecularExponent.y);
	specFactor *= r_SpecularScale;	


	// compute final color
	vec4 color = diffuse;
	color.rgb *= var_LightColor.rgb * NdotL;
	color.rgb += specBase.rgb * var_LightColor.rgb * specFactor;
	color.a *= var_LightColor.a;	// for terrain blending
	
	

#else // !USE_NORMAL_MAPPING

	vec3 N = normalize(var_Normal);    

	#if defined(TWOSIDED)
		if(gl_FrontFacing)
		{
			N = -N;
		}
	#endif

	// compute light color from vertex light
	//vec4 color = vec4(diffuse.rgb * var_LightColor.rgb, var_LightColor.a);    
	vec4 color = diffuse;
	color.rgb *= var_LightColor.rgb;
	color.a = var_LightColor.a;	// for terrain blending    
    
    //hypov8 debug missing bumpmaps (red)
  	//color.rgb = vec3(1.0, 0.5, 0.5) * lightColor;

#endif //end !USE_NORMAL_MAPPING

#if defined(USE_GLOW_MAPPING)
    color.rgb += texture2D(u_GlowMap, texGlow).rgb;
#endif



#if defined(r_DeferredShading)
	gl_FragData[0] = color;
	gl_FragData[1] = vec4(diffuse.rgb, var_LightColor.a);
	gl_FragData[2] = vec4(N, var_LightColor.a);
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, var_LightColor.a);
#else
	gl_FragColor = color;
#endif


//hypov8 debug
#if defined(r_showLightMaps)
	gl_FragColor = vec4(var_LightColor.rgb, 1.0);
#elif defined(r_showDeluxeMaps)
    vec3 dirToRGB = (var_LightDirection + 1.0) / 2;
	gl_FragColor = vec4(dirToRGB, 1.0); 
#endif


#if 0
#if defined(USE_PARALLAX_MAPPING)
	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
#elif defined(USE_NORMAL_MAPPING)
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
#else
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
#endif
#endif
}
