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

/* lightMapping_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_GlowMap;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_DeluxeMap;
uniform float		u_AlphaThreshold;
uniform vec3		u_ViewOrigin;
uniform float		u_DepthScale;
uniform vec2		u_SpecularExponent;

varying vec3		var_Position;
varying vec4		var_TexDiffuseGlow;
varying vec4		var_TexNormalSpecular;
varying vec2		var_TexLight;

varying vec3		var_Tangent;
varying vec3		var_Binormal;
varying vec3		var_Normal;

varying vec4		var_LightColor;


void	main()
{
	vec2 texDiffuse = var_TexDiffuseGlow.st;
	vec2 texNormal = var_TexNormalSpecular.st;
	vec2 texSpecular = var_TexNormalSpecular.pq;
    vec2 texGlow = var_TexDiffuseGlow.pq;

	// compute view direction in world space
	vec3 vDir = normalize(u_ViewOrigin - var_Position);

#if defined(USE_NORMAL_MAPPING)

	// invert tangent space for two sided surfaces
	mat3 tangentToWorldMatrix = mat3(var_Tangent.xyz, var_Binormal.xyz, var_Normal.xyz);
	#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		tangentToWorldMatrix = -tangentToWorldMatrix;
	}
	#endif


    #if defined(USE_PARALLAX_MAPPING)
        // ray intersect in view direction

        // compute view direction in tangent space
        vec3 V = vDir * tangentToWorldMatrix;
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

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse).rgba;

	if( abs(diffuse.a + u_AlphaThreshold) <= 1.0 )
	{
		discard;
		return;
	}


	// compute normal in world space from normalmap
	vec3 N = (2.0 * (texture2D(u_NormalMap, texNormal).xyz - 0.5));
	N = normalize(tangentToWorldMatrix * N);

	// compute light direction in world space
#if 1
	vec3 L = normalize((2.0 * (texture2D(u_DeluxeMap, var_TexLight).xyz - 0.5)));
#else
	//fix black deluxmap. use face dir instead.
	//todo: fix kmap instead
	vec3 deluxMap = texture2D(u_DeluxeMap, var_TexLight).xyz;
	float avg = clamp(deluxMap.x + deluxMap.x + deluxMap.z, 0.0, 1.0);
	vec3 L = (2.0 * (deluxMap - 0.5));
	L = normalize((var_Normal.xyz*avg) + (deluxMap*avg));
#endif

	// compute light color from world space lightmap
	vec3 lightColor = texture2D(u_LightMap, var_TexLight).rgb;
	vec3 FN = normalize(var_Normal);
	vec3 H = normalize(L+FN);

	float NdotL = clamp(dot( N, L ), 0.0, 1.0);
	float FdotL = clamp(dot(H, L), 0.1, 1.0);

	// compute the specular term
	vec4 specBase = texture2D(u_SpecularMap, texSpecular).rgba;
	float specFactor = clamp(dot(normalize(reflect(-L, N)),vDir), 0.04, 1.0);
	specFactor = pow(specFactor, u_SpecularExponent.x * specBase.a + u_SpecularExponent.y);
	specFactor *= r_SpecularScale;

	//normalize light intensity
	lightColor.rgb /= FdotL;

	// compute final color
	vec4 color = diffuse;
	color.rgb *= lightColor.rgb * NdotL;
	color.rgb += specBase.rgb * lightColor.rgb * specFactor;
	color.a *= var_LightColor.a;	// for terrain blending

#else // !USE_NORMAL_MAPPING

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse);

	if( abs(diffuse.a + u_AlphaThreshold) <= 1.0 )
	{
		discard;
		return;
	}

	vec3 N = normalize(var_Normal);

	#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		N = -N;
	}
	#endif

	vec3 specBase = vec3(0.0, 0.0, 0.0); //r_DeferredShading

	// compute light color from object space lightmap
	vec3 lightColor = texture2D(u_LightMap, var_TexLight).rgb;

	vec4 color = diffuse;
	color.rgb *= lightColor;
	color.a *= var_LightColor.a;	// for terrain blending

#endif //end !USE_NORMAL_MAPPING


#if defined(USE_GLOW_MAPPING)
	color.rgb += texture2D(u_GlowMap, texGlow).rgb;
#endif


#if defined(r_DeferredShading)
	// convert normal to [0,1] color space
	N = N * 0.5 + 0.5; //r_DeferredShading
	gl_FragData[0] = color; 							// var_LightColor;
	gl_FragData[1] = vec4(diffuse.rgb, var_LightColor.a);	// vec4(var_LightColor.rgb, 1.0 - var_LightColor.a);
	gl_FragData[2] = vec4(N, var_LightColor.a);
	gl_FragData[3] = vec4(specBase, var_LightColor.a);
#else
	gl_FragColor = color;
#endif

#if defined(r_showLightMaps)
	gl_FragColor = texture2D(u_LightMap, var_TexLight);
#elif defined(r_showDeluxeMaps)
	gl_FragColor = texture2D(u_DeluxeMap, var_TexLight);
#endif


#if 0
#if defined(USE_PARALLAX_MAPPING)
	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), diffuse.a);
#elif defined(USE_NORMAL_MAPPING)
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), diffuse.a);
#else
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), diffuse.a);
#endif
#endif

}
