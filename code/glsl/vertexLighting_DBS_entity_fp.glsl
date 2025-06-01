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

/* vertexLighting_DBS_entity_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_GlowMap;
uniform sampler2D	u_ColorMap; //pbr LUT image

uniform samplerCube	u_EnvironmentMap0;
uniform samplerCube	u_EnvironmentMap1;
uniform float		u_EnvironmentInterpolation;

uniform float		u_AlphaThreshold;
uniform vec3		u_ViewOrigin;

// light/lightgrid..
uniform vec3		u_AmbientColor; // currently calculated in bsp. ambiant light
uniform vec3		u_LightDir;     // pointlight dir
uniform vec3		u_LightColor;   // lightgrid color(match lightmap)

uniform float		u_DepthScale;
uniform vec2		u_SpecularExponent;

varying vec3		var_Position;
varying vec2		var_TexDiffuse;
#if defined(USE_NORMAL_MAPPING)
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec3		var_Tangent;
varying vec3		var_Binormal;
#if defined(USE_REFLECTIVE_SPECULAR)
  uniform sampler2D u_SpecHDRI; //pbr spec reflections from 2d env image
#endif
#endif
varying vec2		var_TexGlow;
varying vec3		var_Normal;

const float PI = 3.14159265359;
const float TwoPI = (2.0 * PI);

//////////// start pbr //////////////
//https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/pbr_fs.glsl
const float Epsilon = 0.00001;
const int NumLights = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.1); //was 0.04

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;
	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float NdotL, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(NdotL, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float blendAO(float aoTex, float metalTex, float roughTex)
{
	float r = (1.0 - roughTex);
	return mix(aoTex, 1.0, metalTex * r * r);
}


//hypov8 sample 2d env
//http://marcinignac.com/blog/pragmatic-pbr-hdr/
vec2 pbrViewTo2d(vec3 vDir)
{
	float phi = acos(vDir.z);
	float theta = atan(-1.0 * vDir.x, vDir.y) + PI;
	return vec2(theta / TwoPI, phi / PI);
}
/////////// end pbr ////////////


//light angle/falloff modes.
float smooth_NdotL(float NdotL)
{
	// compute the light term
	#if defined(r_HalfLambertLighting)
		// http://developer.valvesoftware.com/wiki/Half_Lambert
		//NdotL = NdotL * 0.5 + 0.5; //change range (-1.0 <> 1.0) to (0.0 <> 1.0)
		//NdotL *= NdotL; //darkens values...

		//above is causing player to be darker than it should be. below adds some slight negative light value
		NdotL = ((NdotL+0.625) * 0.616); //change range (-1.0 <> 1.0) to (-0.23 <> 1.00)
	#elif defined(r_WrapAroundLighting)
		NdotL = clamp((NdotL + r_WrapAroundLighting) / (1.0 + r_WrapAroundLighting), 0.0, 1.0);
	#else
		NdotL = clamp(NdotL, 0.0, 1.0);
	#endif

	return NdotL;
}

vec3 colorTint(vec3 diffuse, vec3 lightIn)
{
	//add "color" if its not gray
	float intensity = (diffuse.r+diffuse.g+diffuse.b) / 3;
	vec3 lightOut = normalize(lightIn)*intensity;
	float diff1 = (lightOut.r+lightOut.g+lightOut.b)/3;
	float diff2 = max(lightOut.r, max(lightOut.g, lightOut.b));
	if (diff2 > 0.0)
		return mix(diffuse, lightOut, 1-pow(diff1/diff2, 5));
	else
		return diffuse; //black
}


void main()
{
	// compute light direction in world space
	vec3 L = normalize(u_LightDir);

	// compute view direction in world space
	vec3 vDir = normalize(u_ViewOrigin - var_Position);

	vec2 texDiffuse = var_TexDiffuse.st;

	//rebuild ambient/light values (todo: move to client..)
	float ambientScale = 2* clamp((u_AmbientColor.r + u_AmbientColor.g+ u_AmbientColor.b) / 3.0, 0.0, 0.25);
	vec3 ambientColor = normalize(u_LightColor) * ambientScale;
	vec3 lightColor = u_LightColor * (1.5 - ambientScale);

	//set min light value. (RF_MINLIGHT)
	vec3 lightComp = max(u_AmbientColor, u_LightColor);

#if defined(USE_NORMAL_MAPPING)
	// invert tangent space for two sided surfaces
	mat3 tangentToWorldMatrix = mat3(var_Tangent.xyz, var_Binormal.xyz, var_Normal.xyz);

	#if defined(TWOSIDED)
		if(gl_FrontFacing)
		{
			tangentToWorldMatrix = -tangentToWorldMatrix;
		}
	#endif

	vec2 texNormal = var_TexNormal.st;
	vec2 texSpecular = var_TexSpecular.st;
	vec2 texGlow = var_TexGlow.st;

	#if defined(USE_PARALLAX_MAPPING)

		// ray intersect in view direction

		// compute view direction in tangent space
		vec3 Vts = vDir * tangentToWorldMatrix;
		Vts = normalize(Vts);

		// size and start position of search in texture space
		vec2 S = Vts.xy * -u_DepthScale / Vts.z;
		float depth = RayIntersectDisplaceMap(texNormal, S, u_NormalMap);

		// compute texcoords offset
		vec2 texOffset = S * depth;


		texDiffuse.st += texOffset;
		texNormal.st += texOffset;
		texSpecular.st += texOffset;
		texGlow.st += texOffset;
		//texCoords += texOffset; //todo: merge texCoords
	#endif // USE_PARALLAX_MAPPING

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse);

	// Apply vertex blend operation like: alphaGen vertex.
	//diffuse *= var_Color; //todo: blend modes

	if( abs(diffuse.a + u_AlphaThreshold) <= 1.0 )
	{
		discard;
		return;
	}

	vec4 specBase = texture2D(u_SpecularMap, texSpecular).rgba;

	#if defined(USE_GLOW_MAPPING)
		vec3 glowTex = texture2D(u_GlowMap, texGlow).rgb;
	#endif

	// compute normal in world space from normalmap
	vec3 N = normalize(tangentToWorldMatrix * (2.0 * (texture2D(u_NormalMap, texNormal).xyz - 0.5)));

	// compute half angle in world space
	vec3 H = normalize(L + vDir);

	// compute the light term
	float NdotL = dot(N, L);
	float NdotL_90deg = clamp( NdotL, 0.0, 1.0);
	float NdotL_Smooth = smooth_NdotL(NdotL);


	//hypov8 cull low angle reflection
	float specCullBackFace = clamp(NdotL_90deg * 50, 0.0, 1.0);

	// compute the specular term
	#if defined(USE_REFLECTIVE_SPECULAR) //|| defined(USE_PBR_SPECULAR) //force pbr?

		vec3 reflectDir = normalize(reflect(-vDir, N));

		#if !defined(USE_PBR_SPECULAR) //use regular spec reflections

			if (u_EnvironmentInterpolation < 0.0) //regular specmap. with reflections
			{
				vec2 vCords = pbrViewTo2d(reflectDir);
				specBase.rgb = mix(specBase.rgb, textureLod(u_SpecHDRI, vCords, 1.0).rgb, specBase.rgb);
			}
			else  //(u_EnvironmentInterpolation < 1.0) //map has cubemaps. used for regular specmap, with reflections
			{
				vec4 envColor0 = textureCube(u_EnvironmentMap0, reflectDir).rgba;
				vec4 envColor1 = textureCube(u_EnvironmentMap1, reflectDir).rgba;
				specBase.rgb *= mix(envColor0, envColor1, u_EnvironmentInterpolation).rgb;
			}

		#else //pbr USE_PBR_SPECULAR
			////////start pbr ///////////////////

			//daemon 0.52 RGB=ARM (AO, Rough, Metalic)
			vec3 diffuseTex = diffuse.rgb;
			float aoTex    = specBase.r; //ambient occlusion
			float roughTex = specBase.g; //roughness values
			float metalTex = specBase.b; //metalic value
			float mipCount = u_EnvironmentInterpolation;

			// Angle between surface normal and outgoing light direction.
			float NdotV = max(0.0, dot(N, vDir));

			// Fresnel reflectance at normal incidence (for metals use albedo color).
			vec3 F0 = mix(Fdielectric, diffuseTex, metalTex);

			// Direct lighting calculation for analytical lights.
			vec3 directLighting = vec3(0.0);
			//for(int i=0; i<NumLights; ++i)
			{
				//vec3 L = L;                  // -lights[i].direction;
				vec3 Lradiance = lightColor; // lights[i].radiance;

				// Half-vector between LiDir and vDir.
				vec3 Lh = normalize(L + vDir);

				// Calculate angles between surface normal and various light vectors.
				//float cosLi = max(0.0, dot(N, L));
				float cosLh = max(0.0, dot(N, Lh));

				// Calculate Fresnel term for direct lighting.
				vec3 F  = fresnelSchlick(F0, max(0.04, dot(Lh, vDir)));
				// Calculate normal distribution for specular BRDF.
				float D = ndfGGX(cosLh, roughTex);
				// Calculate geometric attenuation for specular BRDF.
				float G = gaSchlickGGX(NdotL_90deg, NdotV, roughTex);

				// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
				// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
				// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
				vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalTex);

				// Lambert diffuse BRDF.
				// We don't scale by 1/PI for lighting & material units to be more convenient.
				// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
				vec3 diffuseBRDF = kd * diffuseTex ;

				// Cook-Torrance specular microfacet BRDF.
				vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotL_90deg * NdotV);

				//hypov8 add AO
				specularBRDF *= blendAO(aoTex, metalTex, roughTex);
				//hypov8 cull light hitting backface.
				specularBRDF *= specCullBackFace;

				// Total contribution for this light.
				directLighting = (diffuseBRDF + specularBRDF) * Lradiance * NdotL_Smooth; //NdotL;
			}

			// Ambient lighting (IBL).
			vec3 ambientLighting;
			{
				vec2 vCords = pbrViewTo2d(reflectDir);
				vec3 ambLightMix = mix(ambientColor, lightColor, NdotL_90deg);

				// Sample diffuse irradiance at normal direction.
				vec3 irradiance = (textureLod(u_SpecHDRI, vCords, mipCount*0.75).rgb);
				irradiance *= ambLightMix; //add map light

				// Calculate Fresnel term for ambient lighting.
				// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
				// use cosLo(NdotV) instead of angle with light's half-vector (cosLh above).
				// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
				vec3 F = fresnelSchlick(F0, NdotV);

				// Get diffuse contribution factor (as with direct lighting).
				vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalTex);

				// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
				vec3 diffuseIBL = kd * diffuseTex * irradiance;

				diffuseIBL *= aoTex; //hypov8 add AO

				// Sample pre-filtered specular reflection environment at correct mipmap level.
				vec3 specularIrradiance = textureLod(u_SpecHDRI, vCords, roughTex * mipCount).rgb;

				//hypov8 add light color to cubemap/reflections
				specularIrradiance = colorTint(specularIrradiance, lightColor);

				// Split-sum approximation factors for Cook-Torrance specular BRDF.
				vec2 specularBRDF = texture(u_ColorMap, vec2(NdotV, roughTex)).rg; //specularBRDF_LUT

				// Total specular IBL contribution.
				vec3 specularIBL = (F0 * vec3(specularBRDF.x) + vec3(specularBRDF.y)) * specularIrradiance;

				// Total ambient lighting contribution.
				ambientLighting = diffuseIBL + specularIBL;
			}

			// Final fragment color.
			vec3 color2 = vec3(directLighting + ambientLighting);
			///////// end pbr ////////

			gl_FragColor = vec4(color2.rgb, 1.0);
			return;
			//hypov8 todo: set below
		#endif

	#else //!USE_REFLECTIVE_SPECULAR

		//try convert bpr to regular spec.
		#if defined(USE_PBR_SPECULAR)
			float aoTex    = specBase.r; //ambient occlusion
			float roughTex = specBase.g; //roughness values
			float metalTex = specBase.b; //metalic value

			diffuse.rgb *=  aoTex*0.5 +0.5;
			specBase.rgb = vec3(1-roughTex); //roughess->gloss
			specBase.a = 1 - (metalTex* 0.5);
		#endif

	#endif // END USE_REFLECTIVE_SPECULAR

	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	float specularExpo =  u_SpecularExponent.x * specBase.a + u_SpecularExponent.y; //vec2(16, 0)
	specBase.rgb = lightComp * specBase.rgb * pow(NdotH, specularExpo) * r_SpecularScale;

	//////////////////////////////////////////////////////////////
	//cull light hitting backface.
	specBase.rgb *= specCullBackFace;
	//////////////////////////////////////////////////////////////

#else // !USE_NORMAL_MAPPING

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse);

	#if defined(USE_GLOW_MAPPING)
		vec3 glowTex = texture2D(u_GlowMap, var_TexGlow.st).rgb;
	#endif

	if( abs(diffuse.a + u_AlphaThreshold) <= 1.0 )
	{
		discard;
		return;
	}

	// use vertex normal
	vec3 N = normalize(var_Normal);

	#if defined(TWOSIDED)
		if(gl_FrontFacing)
		{
			N = -N;
		}
	#endif

	vec4 specBase = vec4(0.0);

	// compute the light term
	float NdotL = dot(N, L);

#endif // !USE_NORMAL_MAPPING

	float NdotL_smooth = smooth_NdotL(NdotL);

	// compute final color
	vec4 color = diffuse;
	color.rgb *= mix(ambientColor, ambientColor + lightColor, NdotL_smooth);
	color.rgb += specBase.rgb;

// add Rim Lighting to highlight the edges
#if defined(r_RimLighting)
	float rim = pow(1.0 - clamp(dot(N, vDir), 0.0, 1.0), r_RimExponent);
	vec3 emission = ambientColor * rim * rim * 0.2;
	color.rgb += 0.7 * emission;
#endif
#if defined(USE_GLOW_MAPPING)
	color.rgb += glowTex;
#endif

#if defined(r_DeferredShading)
	// convert normal to [0,1] color space
	N = N * 0.5 + 0.5;
	gl_FragData[0] = color;
	gl_FragData[1] = vec4(diffuse.rgb, 0.0);
	gl_FragData[2] = vec4(N, 0.0);
	gl_FragData[3] = vec4(specBase.rgb, 0.0);
#else
	gl_FragColor = color;
#endif


#if defined(r_showLightMaps)
	vec4 slm_color = vec4(1.0);
	slm_color.rgb *=  mix(ambientColor, ambientColor + lightColor, smooth_NdotL(NdotL));
	//slm_color.rgb += specBase.rgb;
	gl_FragColor = vec4(slm_color.rgb, 1.0);
#elif defined(r_showDeluxeMaps)
	vec3 dirToRGB = (var_Normal + 1.0) * 0.5;
	gl_FragColor = vec4(dirToRGB, 1.0);
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

