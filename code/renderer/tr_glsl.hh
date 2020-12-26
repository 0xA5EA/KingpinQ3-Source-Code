#ifndef TR_GLSL_HH_
#define TR_GLSL_HH_

#define LOG_GLSL_UNIFORMS 0
static const float epsilon = 0.0001f;
#if defined(LOG_GLSL_UNIFORMS)
extern cvar_t  *r_logFile;		// number of frames to emit GL logs
#endif
extern force_inline void GLSL_SetUniform_ColorTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ColorTextureMatrix, m, epsilon))
    return;
  Mat4_CopySSE(m, program->t_ColorTextureMatrix);
#else
  if (MatrixCompare(program->t_ColorTextureMatrix, m))
    return;
  MatrixCopy(m, program->t_ColorTextureMatrix);
#endif
  glUniformMatrix4fv(program->u_ColorTextureMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_DiffuseTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_DiffuseTextureMatrix, m, epsilon))
    return;
  Mat4_CopySSE(m, program->t_DiffuseTextureMatrix);
#else
  if (MatrixCompare(program->t_DiffuseTextureMatrix, m))
    return;
  MatrixCopy(m, program->t_DiffuseTextureMatrix);
#endif
	glUniformMatrix4fv(program->u_DiffuseTextureMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_NormalTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_NormalTextureMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_NormalTextureMatrix);
#else
  if (MatrixCompare(program->t_NormalTextureMatrix, m))
    return;

  MatrixCopy(m, program->t_NormalTextureMatrix);
#endif
	glUniformMatrix4fv(program->u_NormalTextureMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_SpecularTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_SpecularTextureMatrix, m, epsilon))
    return;
  Mat4_CopySSE(m, program->t_SpecularTextureMatrix);
#else
  if (MatrixCompare(program->t_SpecularTextureMatrix, m))
    return;
  MatrixCopy(m, program->t_SpecularTextureMatrix);
#endif
	glUniformMatrix4fv(program->u_SpecularTextureMatrix, 1, GL_FALSE, m);
}


extern force_inline void GLSL_SetUniform_AlphaTest(shaderProgram_t * program, uint32_t stateBits)
{
	alphaTest_t			value;

	switch (stateBits & GLS_ATEST_BITS)
	{
  case GLS_ATEST_GT_0:    value = ATEST_GT_0;   break;
  case GLS_ATEST_LT_128:  value = ATEST_LT_128; break;
  case GLS_ATEST_GE_128:  value = ATEST_GE_128; break;
  default:                value = ATEST_NONE;   break;
	}
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- GLSL_SetUniformAlphaTest( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	if (program->t_AlphaTest == value)
		return;

	program->t_AlphaTest = value;

	glUniform1i(program->u_AlphaTest, value);
}

extern force_inline void GLSL_SetUniform_ViewOrigin(shaderProgram_t * program, const vec3_t v)
{
	if (VectorCompare(program->t_ViewOrigin, v))
		return;

	VectorCopy(v, program->t_ViewOrigin);
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ViewOrigin( program = %s, viewOrigin = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	glUniform3f(program->u_ViewOrigin, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_Color(shaderProgram_t * program, const vec4_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(Vec4_Compare(program->t_Color, v))
		return;

	Vector4Copy(v, program->t_Color);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_Color( program = %s, color = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif
	glUniform4f(program->u_Color, v[0], v[1], v[2], v[3]);
}

extern force_inline void GLSL_SetUniform_ColorModulate(shaderProgram_t * program, const vec4_t v)
{
	if (Vec4_Comp(program->t_ColorModulate, v))
		return;

	Vector4Copy(v, program->t_ColorModulate);

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ColorModulate( program = %s, color = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	glUniform4f(program->u_ColorModulate, v[0], v[1], v[2], v[3]);
}

extern force_inline void GLSL_SetUniform_AmbientColor(shaderProgram_t * program, const vec3_t v)
{
	if (VectorCompare(program->t_AmbientColor, v))
		return;

	VectorCopy(v, program->t_AmbientColor);

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_AmbientColor( program = %s, color = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	glUniform3f(program->u_AmbientColor, v[0], v[1], v[2]);
}

extern force_inline void GLSL_SetUniform_LightDir(shaderProgram_t * program, const vec3_t v)
{
	if (VectorCompare(program->t_LightDir, v))
		return;

	VectorCopy(v, program->t_LightDir);

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightDir( program = %s, direction = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	glUniform3f(program->u_LightDir, v[0], v[1], v[2]);
}

extern force_inline void GLSL_SetUniform_LightOrigin(shaderProgram_t * program, const vec3_t v)
{
	if (VectorCompare(program->t_LightOrigin, v))
		return;

	VectorCopy(v, program->t_LightOrigin);

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightOrigin( program = %s, origin = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	glUniform3f(program->u_LightOrigin, v[0], v[1], v[2]);
}

extern force_inline void GLSL_SetUniform_LightColor(shaderProgram_t * program, const vec3_t v)
{
	if (VectorCompare(program->t_LightColor, v))
		return;

	VectorCopy(v, program->t_LightColor);

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightColor( program = %s, color = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	glUniform3f(program->u_LightColor, v[0], v[1], v[2]);
}

extern force_inline void GLSL_SetUniform_LightRadius(shaderProgram_t * program, float value)
{
	if (program->t_LightRadius == value)
		return;

	program->t_LightRadius = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightRadius( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_LightRadius, value);
}

extern force_inline void GLSL_SetUniform_LightParallel(shaderProgram_t * program, qboolean value)
{
	if (program->t_LightParallel == value)
		return;

	program->t_LightParallel = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightParallel( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	glUniform1i(program->u_LightParallel, value);
}

extern force_inline void GLSL_SetUniform_LightScale(shaderProgram_t * program, float value)
{
#if 0
	if (DS_PREPASS_LIGHTING_ENABLED())
	{
		value -= (r_lightScale->value -1);
		value = Q_max(value, 0);
	}
#endif

	if (program->t_LightScale == value)
		return;

	program->t_LightScale = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightScale( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	glUniform1f(program->u_LightScale, value);
}

extern force_inline void GLSL_SetUniform_LightWrapAround(shaderProgram_t * program, float value)
{
	if (program->t_LightWrapAround == value)
		return;

	program->t_LightWrapAround = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightWrapAround( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	glUniform1f(program->u_LightWrapAround, value);
}

extern force_inline void GLSL_SetUniform_LightAttenuationMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_LightAttenuationMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_LightAttenuationMatrix);
#else
  if (MatrixCompare(program->t_LightAttenuationMatrix, m))
    return;

  MatrixCopy(m, program->t_LightAttenuationMatrix);
#endif
	glUniformMatrix4fv(program->u_LightAttenuationMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_ShadowMatrix(shaderProgram_t * program, matrix_t m[MAX_SHADOWMAPS])
{
#if 0
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ShadowMatrix, m))
    return;

  Mat4_CopySSE(m, program->t_ShadowMatrix);
#else
  if (MatrixCompare(program->t_ShadowMatrix, m))
    return;

  MatrixCopy(m, program->t_ShadowMatrix);
#endif
#endif
  //FIXME(0xA5EA): fix the firewall
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		int			i;
		for(i = 0; i < MAX_SHADOWMAPS; i++)
		{
			GLimp_LogComment(va("--- GLSL_SetUniform_ShadowMatrix( program = %s, "
							"matrix(%i) = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							i,
							m[i][0], m[i][4], m[i][8], m[i][12],
							m[i][1], m[i][5], m[i][9], m[i][13],
							m[i][2], m[i][6], m[i][10], m[i][14],
							m[i][3], m[i][7], m[i][11], m[i][15]));
		}
	}
#endif
	glUniformMatrix4fv(program->u_ShadowMatrix, MAX_SHADOWMAPS, GL_FALSE, &m[0][0]);
}

extern force_inline void GLSL_SetUniform_ShadowCompare(shaderProgram_t * program, qboolean value)
{
	if (program->t_ShadowCompare == value)
		return;

	program->t_ShadowCompare = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowCompare( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif
	glUniform1i(program->u_ShadowCompare, value);
}

extern force_inline void GLSL_SetUniform_ShadowTexelSize(shaderProgram_t * program, float value)
{
	if (program->t_ShadowTexelSize == value)
		return;

	program->t_ShadowTexelSize = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowTexelSize( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_ShadowTexelSize, value);
}

extern force_inline void GLSL_SetUniform_ShadowBlur(shaderProgram_t * program, float value)
{
	if (program->t_ShadowBlur == value)
		return;

	program->t_ShadowBlur = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowBlur( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_ShadowBlur, value);
}

extern force_inline void GLSL_SetUniform_ShadowParallelSplitDistances(shaderProgram_t * program, const vec4_t v)
{
	if (Vec4_Comp(program->t_ShadowParallelSplitDistances, v))
		return;
	Vector4Copy(v, program->t_ShadowParallelSplitDistances);
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowParallelSplitDistances( program = %s, distances = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif
	glUniform4f(program->u_ShadowParallelSplitDistances, v[0], v[1], v[2], v[3]);
}

extern force_inline void GLSL_SetUniform_RefractionIndex(shaderProgram_t * program, float value)
{
	if (program->t_RefractionIndex == value)
		return;

	program->t_RefractionIndex = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_RefractionIndex( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_RefractionIndex, value);
}

extern force_inline void GLSL_SetUniform_ParallaxMapping(shaderProgram_t * program, qboolean value)
{
	if (program->t_ParallaxMapping == value)
		return;

	program->t_ParallaxMapping = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ParallaxMapping( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif
	glUniform1i(program->u_ParallaxMapping, value);
}

extern force_inline void GLSL_SetUniform_DepthScale(shaderProgram_t * program, float value)
{
	if (program->t_DepthScale == value)
		return;

	program->t_DepthScale = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DepthScale( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_DepthScale, value);
}

static ID_INLINE void GLSL_SetUniform_EnvironmentInterpolation(shaderProgram_t * program, float value)
{

	if (program->t_EnvironmentInterpolation == value)
		return;

	program->t_EnvironmentInterpolation = value;


#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_EnvironmentInterpolation( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	glUniform1f(program->u_EnvironmentInterpolation, value);
}

extern force_inline void GLSL_SetUniform_PortalClipping(shaderProgram_t * program, qboolean value)
{
	if (program->t_PortalClipping == value)
		return;

	program->t_PortalClipping = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalClipping( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif
	glUniform1i(program->u_PortalClipping, value);
}

extern force_inline void GLSL_SetUniform_PortalPlane(shaderProgram_t * program, const vec4_t v)
{
	if (Vec4_Comp(program->t_PortalPlane, v))
		return;

  Vector4Copy(v, program->t_PortalPlane);
  //	VectorCopy(v, program->t_PortalPlane); //FIXME: xreal uses Vec3_Copy, why ? 0xA5EA
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalPlane( program = %s, plane = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	glUniform4f(program->u_PortalPlane, v[0], v[1], v[2], v[3]);
}

extern force_inline void GLSL_SetUniform_PortalRange(shaderProgram_t * program, float value)
{
	if (program->t_PortalRange == value)
		return;

	program->t_PortalRange = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalRange( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_PortalRange, value);
}

extern force_inline void GLSL_SetUniform_ModelMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ModelMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ModelMatrix);
#else
  if (MatrixCompare(program->t_ModelMatrix, m))
    return;

  MatrixCopy(m, program->t_ModelMatrix);
#endif
#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ModelMatrix( program = %s, "
							"matrix = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							m[0], m[4], m[8], m[12],
							m[1], m[5], m[9], m[13],
							m[2], m[6], m[10], m[14],
							m[3], m[7], m[11], m[15]));
	}
#endif
	glUniformMatrix4fv(program->u_ModelMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_ViewMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ViewMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ViewMatrix);
#else
  if (MatrixCompare(program->t_ViewMatrix, m))
    return;

  MatrixCopy(m, program->t_ViewMatrix);
#endif
	glUniformMatrix4fv(program->u_ViewMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_ModelViewMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ModelViewMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ModelViewMatrix);
#else
  if (MatrixCompare(program->t_ModelViewMatrix, m))
    return;

  MatrixCopy(m, program->t_ModelViewMatrix);
#endif
	glUniformMatrix4fv(program->u_ModelViewMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_ModelViewMatrixTranspose(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ModelViewMatrixTranspose, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ModelViewMatrixTranspose);
#else
  if (MatrixCompare(program->t_ModelViewMatrixTranspose, m))
    return;

  MatrixCopy(m, program->t_ModelViewMatrixTranspose);
#endif
	glUniformMatrix4fv(program->u_ModelViewMatrixTranspose, 1, GL_TRUE, m);
}

extern force_inline void GLSL_SetUniform_ProjectionMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ProjectionMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ProjectionMatrix);
#else
  if (MatrixCompare(program->t_ProjectionMatrix, m))
    return;

  MatrixCopy(m, program->t_ProjectionMatrix);
#endif
	glUniformMatrix4fv(program->u_ProjectionMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_ProjectionMatrixTranspose(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ProjectionMatrixTranspose, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ProjectionMatrixTranspose);
#else
  if (MatrixCompare(program->t_ProjectionMatrixTranspose, m))
    return;

  MatrixCopy(m, program->t_ProjectionMatrixTranspose);
#endif
	glUniformMatrix4fv(program->u_ProjectionMatrixTranspose, 1, GL_TRUE, m);
}

extern force_inline void GLSL_SetUniform_ModelViewProjectionMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_ModelViewProjectionMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_ModelViewProjectionMatrix);
#else
  if (MatrixCompare(program->t_ModelViewProjectionMatrix, m))
    return;

  MatrixCopy(m, program->t_ModelViewProjectionMatrix);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ModelViewProjectionMatrix( program = %s, "
							"matrix = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							m[0], m[4], m[8], m[12],
							m[1], m[5], m[9], m[13],
							m[2], m[6], m[10], m[14],
							m[3], m[7], m[11], m[15]));
	}
#endif
	glUniformMatrix4fv(program->u_ModelViewProjectionMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_UnprojectMatrix(shaderProgram_t * program, const matrix_t m)
{
#ifdef USING_SSE_MATH
  if (Mat4_CompEpsilonSSE(program->t_UnprojectMatrix, m, epsilon))
    return;

  Mat4_CopySSE(m, program->t_UnprojectMatrix);
#else
  if (MatrixCompare(program->t_UnprojectMatrix, m))
    return;

  MatrixCopy(m, program->t_UnprojectMatrix);
#endif
	glUniformMatrix4fv(program->u_UnprojectMatrix, 1, GL_FALSE, m);
}

extern force_inline void GLSL_SetUniform_VertexSkinning(shaderProgram_t * program, qboolean value)
{
	if (program->t_VertexSkinning == value)
		return;

	program->t_VertexSkinning = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_VertexSkinning( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif
	glUniform1i(program->u_VertexSkinning, value);
}

extern force_inline  void GLSL_SetUniform_VertexInterpolation(shaderProgram_t * program, float value)
{
  if (program->t_VertexInterpolation == value)
    return;

  program->t_VertexInterpolation = value;

#if defined(LOG_GLSL_UNIFORMS)
  if (r_logFile->integer)
  {
    GLimp_LogComment(va("--- GLSL_SetUniform_VertexInterpolation( program = %s, value = %f ) ---\n", program->name, value));
  }
#endif
  glUniform1f(program->u_VertexInterpolation, value);
}

extern force_inline void GLSL_SetUniform_Time(shaderProgram_t * program, float value)
{
	if (program->t_Time == value)
		return;

	program->t_Time = value;

#if defined(LOG_GLSL_UNIFORMS)
	if (r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_Time( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif
	glUniform1f(program->u_Time, value);
}

#endif // TR_GLSL_HH_
