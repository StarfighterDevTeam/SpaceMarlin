#pragma custom_preprocessor_off

#ifndef _SHARED_DEFINES_H
#define _SHARED_DEFINES_H

#if !defined(__cplusplus) || defined(__INTELLISENSE__)
	#define _GLSL
#endif

#ifdef _GLSL
	#define U8VEC4 vec4

	#define M_E        2.71828182845904523536   // e
    #define M_LOG2E    1.44269504088896340736   // log2(e)
    #define M_LOG10E   0.434294481903251827651  // log10(e)
    #define M_LN2      0.693147180559945309417  // ln(2)
    #define M_LN10     2.30258509299404568402   // ln(10)
    #define M_PI       3.14159265358979323846   // pi
    #define M_PI_2     1.57079632679489661923   // pi/2
    #define M_PI_4     0.785398163397448309616  // pi/4
    #define M_1_PI     0.318309886183790671538  // 1/pi
    #define M_2_PI     0.636619772367581343076  // 2/pi
    #define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
    #define M_SQRT2    1.41421356237309504880   // sqrt(2)
    #define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#else
	#define U8VEC4 glm::u8vec4
#endif

// List of all GPU programs
#define FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM, BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE, END_PROGRAM)	\
	HANDLE_GPUPROGRAM(PROG_MODEL)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_MARLIN)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_SIMPLE)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_TEXTURE_2D)			(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_LANE)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_WATER_SIMULATION)	(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_WATER_NORMALS)		(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_SKYBOX)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_TONEMAPPING)			(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_ATOM)				(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	/* done */
	
#define HANDLE_PROG(x) HANDLE_##x

#define BEGIN_PROGRAM_NO_ACTION(vtxStructType, vertexFileName, fragmentFileName)

#define HANDLE_UNIFORM_NO_ACTION(type, varName)

#define HANDLE_UNIFORM_DECLARE(type, varName)	\
	uniform type varName;

#define HANDLE_ATTRIBUTE_NO_ACTION(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)

#define HANDLE_ATTRIBUTE_DECLARE(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
	layout(location=loc)	in varType varName;

#define PROG_VERTEX_SHADER(progId)		HANDLE_##progId(BEGIN_PROGRAM_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_DECLARE)
#define PROG_FRAGMENT_SHADER(progId)	HANDLE_##progId(BEGIN_PROGRAM_NO_ACTION, HANDLE_UNIFORM_DECLARE, HANDLE_ATTRIBUTE_NO_ACTION)
	
// -------- Part common to all GPU programs --------
#define HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* uniforms */										\
	HANDLE_UNIFORM(float,		gTime)					\
	HANDLE_UNIFORM(mat4,		gLocalToProjMtx)		\
	HANDLE_UNIFORM(mat4,		gLocalToViewMtx)		\
	HANDLE_UNIFORM(mat4,		gLocalToWorldMtx)		\
	HANDLE_UNIFORM(mat3,		gLocalToWorldNormalMtx)	\
	HANDLE_UNIFORM(mat4,		gWorldToViewMtx)		\
	HANDLE_UNIFORM(mat4,		gWorldToProjMtx)		\
	HANDLE_UNIFORM(vec3,		gWorldSpaceCamPos)		\
	HANDLE_UNIFORM(mat4,		gProjToWorldRotMtx)		\
	HANDLE_UNIFORM(mat4,		gViewToProjMtx)			\
	HANDLE_UNIFORM(vec2,		gVpSize)				\
	/* done */

// -------- Part common to all fullscreen triangle programs --------
#define HANDLE_PROG_FULLSCREEN_TRIANGLE(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	/* attributes */																			\
	HANDLE_ATTRIBUTE(VtxFullScreenTriangle,	vec2, float, GL_FLOAT, GL_FALSE, pos,		0)		\
	HANDLE_ATTRIBUTE(VtxFullScreenTriangle,	vec2, float, GL_FLOAT, GL_FALSE, uv,		1)		\
	/* done */

// -------- Model --------
#define HANDLE_PROG_MODEL(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxModel, "model.vert", "model.frag")							\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)			\
	/* uniforms */																\
	HANDLE_UNIFORM(sampler2D, texAlbedo)										\
	/* attributes */															\
	HANDLE_ATTRIBUTE(VtxModel, vec3, float, GL_FLOAT, GL_FALSE, pos,	0)		\
	HANDLE_ATTRIBUTE(VtxModel, vec2, float, GL_FLOAT, GL_FALSE, uv,		1)		\
	HANDLE_ATTRIBUTE(VtxModel, vec3, float, GL_FLOAT, GL_FALSE, normal,	2)		\
	/* done */

// -------- Marlin --------
#define HANDLE_PROG_MARLIN(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxMarlin, "marlin.vert", "marlin.frag")						\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)			\
	/* uniforms */																\
	HANDLE_UNIFORM(sampler2D,	texAlbedo)										\
	HANDLE_UNIFORM(vec3,		gSpeed)											\
	/* attributes */															\
	HANDLE_ATTRIBUTE(VtxMarlin, vec3, float, GL_FLOAT, GL_FALSE, pos,		0)	\
	HANDLE_ATTRIBUTE(VtxMarlin, vec2, float, GL_FLOAT, GL_FALSE, uv,		1)	\
	HANDLE_ATTRIBUTE(VtxMarlin, vec3, float, GL_FLOAT, GL_FALSE, normal,	2)	\
	/* done */

// -------- Atom --------
#define HANDLE_PROG_ATOM(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxAtom, "atom.vert", "atom.frag")							\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)			\
	/* uniforms */																\
	HANDLE_UNIFORM(sampler2D, texAlbedo)										\
	/* attributes */															\
	HANDLE_ATTRIBUTE(VtxAtom, vec3, float, GL_FLOAT, GL_FALSE, pos,	0)			\
	HANDLE_ATTRIBUTE(VtxAtom, vec2, float, GL_FLOAT, GL_FALSE, uv,		1)		\
	HANDLE_ATTRIBUTE(VtxAtom, vec3, float, GL_FLOAT, GL_FALSE, normal,	2)		\
	/* done */
	
// -------- Simple --------
#define HANDLE_PROG_SIMPLE(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxSimple, "simple.vert", "simple.frag")						\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)			\
	/* attributes */															\
	HANDLE_ATTRIBUTE(VtxSimple, vec3, float, GL_FLOAT, GL_FALSE , pos,		0)	\
	HANDLE_ATTRIBUTE(VtxSimple, vec4, float, GL_FLOAT, GL_FALSE , color,	1)	\
	/* done */

// -------- Texture2D --------
#define HANDLE_PROG_TEXTURE_2D(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	BEGIN_PROGRAM(VtxTexture2D, "texture2d.vert", "texture2d.frag")				\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)			\
	/* uniforms */																\
	HANDLE_UNIFORM(sampler2D, tex)												\
	/* attributes */															\
	HANDLE_ATTRIBUTE(VtxTexture2D, vec2, float, GL_FLOAT, GL_FALSE , pos,	0)	\
	HANDLE_ATTRIBUTE(VtxTexture2D, vec2, float, GL_FLOAT, GL_FALSE , uv,	1)	\
	/* done */

// -------- Lane --------
#define FOREACH_LANE_KEYFRAME_MEMBER(HANDLE_UNIFORM)						\
	HANDLE_UNIFORM(float, gKeyframeR0)										\
	HANDLE_UNIFORM(float, gKeyframeR1)										\
	HANDLE_UNIFORM(float, gKeyframeHalfDist)								\
	HANDLE_UNIFORM(float, gKeyframeTheta)									\
	HANDLE_UNIFORM(float, gKeyframeCapsulePerimeter)						\
	HANDLE_UNIFORM(float, gKeyframeThreshold0)								\
	HANDLE_UNIFORM(float, gKeyframeThreshold1)								\
	HANDLE_UNIFORM(float, gKeyframeThreshold2)								\
	HANDLE_UNIFORM(float, gKeyframeThreshold3)								\
	HANDLE_UNIFORM(float, gKeyframeThreshold0to1)							\
	HANDLE_UNIFORM(float, gKeyframeThreshold2to3)							\
	HANDLE_UNIFORM(vec2, gKeyframeTopRightPos)								\
	HANDLE_UNIFORM(vec2, gKeyframeTopLeftPos)								\
	HANDLE_UNIFORM(vec2, gKeyframeBottomLeftPos)							\
	HANDLE_UNIFORM(vec2, gKeyframeBottomRightPos)							\
	HANDLE_UNIFORM(vec2, gKeyframeTopTangentVector)							\
	HANDLE_UNIFORM(vec2, gKeyframeBottomTangentVector)						\
	HANDLE_UNIFORM(vec4, gKeyframeRot)										\
	HANDLE_UNIFORM(vec3, gKeyframeTrans)									\
	/* done */

#define HANDLE_PROG_LANE(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	BEGIN_PROGRAM(VtxLane, "lane.vert", "lane.frag")						\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	/* uniforms */															\
	HANDLE_UNIFORM(samplerBuffer, texKeyframes)								\
	HANDLE_UNIFORM(samplerCube, texCubemap)									\
	HANDLE_UNIFORM(sampler2D, texRefraction)								\
	HANDLE_UNIFORM(sampler2D, texHeights)									\
	HANDLE_UNIFORM(sampler2D, texNormals)									\
	/* uniform keyframes */													\
	FOREACH_LANE_KEYFRAME_MEMBER(HANDLE_UNIFORM)							\
	/* attributes */														\
	HANDLE_ATTRIBUTE(VtxLane, vec3, float, GL_FLOAT, GL_FALSE, pos,		0)	\
	HANDLE_ATTRIBUTE(VtxLane, vec3, float, GL_FLOAT, GL_FALSE, normal,	1)	\
	HANDLE_ATTRIBUTE(VtxLane, vec2, float, GL_FLOAT, GL_FALSE, uv,		2)	\
	/* done */

// -------- Water simulation --------
#define HANDLE_PROG_WATER_SIMULATION(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	BEGIN_PROGRAM(VtxWaterSimulation, "water_simulation.vert", "water_simulation.frag")	\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)					\
	/*HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)*/				\
	/* uniforms */																		\
	HANDLE_UNIFORM(sampler2D, texHeights1)												\
	HANDLE_UNIFORM(sampler2D, texHeights2)												\
	HANDLE_UNIFORM(vec2, gTexelSize)													\
	HANDLE_UNIFORM(float, gTerm1)														\
	HANDLE_UNIFORM(float, gTerm2)														\
	HANDLE_UNIFORM(float, gTerm3)														\
	/* attributes */																	\
	HANDLE_ATTRIBUTE(VtxWaterSimulation, vec2, float, GL_FLOAT, GL_FALSE, pos,		0)	\
	/* done */

// -------- Water normals --------
#define HANDLE_PROG_WATER_NORMALS(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxWaterNormals, "water_normals.vert", "water_normals.frag")			\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)					\
	/*HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)*/				\
	/* uniforms */																		\
	HANDLE_UNIFORM(sampler2D, texHeights)												\
	HANDLE_UNIFORM(vec2, gTexelSize)													\
	HANDLE_UNIFORM(vec2, gDistBetweenTexels)											\
	/* attributes */																	\
	HANDLE_ATTRIBUTE(VtxWaterNormals, vec2, float, GL_FLOAT, GL_FALSE, pos,		0)		\
	/* done */

// -------- Skybox ---------
#define HANDLE_PROG_SKYBOX(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	BEGIN_PROGRAM(VtxSkybox, "skybox.vert", "skybox.frag")					\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	/* uniforms */															\
	HANDLE_UNIFORM(samplerCube, texSky)										\
	/* attributes */														\
	HANDLE_ATTRIBUTE(VtxSkybox, vec2, float, GL_FLOAT, GL_FALSE, pos,	0)	\
	HANDLE_ATTRIBUTE(VtxSkybox, vec2, float, GL_FLOAT, GL_FALSE, uv,	1)	\
	/* done */

// -------- Tonemapping ---------
#define HANDLE_PROG_TONEMAPPING(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	BEGIN_PROGRAM(VtxFullScreenTriangle, "tonemapping.vert", "tonemapping.frag")		\
	HANDLE_PROG_COMMON(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)					\
	/* uniforms */																		\
	HANDLE_UNIFORM(sampler2D, texScene)													\
	HANDLE_PROG_FULLSCREEN_TRIANGLE(BEGIN_PROGRAM, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* done */

// ************************** Declare vertex structs *************************
#define BEGIN_PROGRAM_DECLARE_STRUCT(vtxStructType, vertexFileName, fragmentFileName) \
	struct vtxStructType {

#define HANDLE_ATTRIBUTE_DECLARE_STRUCT(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
		varType varName;

#define END_PROGRAM_DECLARE_STRUCT \
	};

FOREACH_GPUPROGRAM(HANDLE_PROG, BEGIN_PROGRAM_DECLARE_STRUCT, HANDLE_UNIFORM_NO_ACTION, HANDLE_ATTRIBUTE_DECLARE_STRUCT, END_PROGRAM_DECLARE_STRUCT)

#endif	// _SHARED_DEFINES_H

#pragma custom_preprocessor_on
