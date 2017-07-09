#pragma custom_preprocessor_off

#if !defined(__cplusplus) || defined(__INTELLISENSE__)
	#define _GLSL
#endif

#ifdef _GLSL
	#define U8VEC4 vec4
#else
	#define U8VEC4 glm::u8vec4
#endif

// List of all GPU programs
#define FOREACH_GPUPROGRAM(HANDLE_GPUPROGRAM, HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE, END_PROGRAM)	\
	HANDLE_GPUPROGRAM(PROG_MODEL)				(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_SIMPLE)				(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_LANE)				(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_SKYBOX)				(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	HANDLE_GPUPROGRAM(PROG_TONEMAPPING)			(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM \
	/*HANDLE_GPUPROGRAM(PROG_WATER_SIMULATION)	(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE) END_PROGRAM*/ \
	/* done */
	
#define HANDLE_PROG(x) HANDLE_##x

#define HANDLE_VERTEX_NO_ACTION(vtxStructType)

#define HANDLE_UNIFORM_NO_ACTION(type, varName)

#define HANDLE_UNIFORM_DECLARE(type, varName)	\
	uniform type varName;

#define HANDLE_ATTRIBUTE_NO_ACTION(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)

#define HANDLE_ATTRIBUTE_DECLARE(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
	layout(location=loc)	in varType varName;
	
// -------- Part common to all GPU programs --------
#define HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* uniforms */									\
	HANDLE_UNIFORM(float,		gTime)				\
	HANDLE_UNIFORM(mat4,		gLocalToProjMtx)	\
	HANDLE_UNIFORM(mat4,		gLocalToViewMtx)	\
	HANDLE_UNIFORM(mat4,		gLocalToWorldMtx)	\
	HANDLE_UNIFORM(vec3,		gWorldSpaceCamPos)	\
	HANDLE_UNIFORM(mat4,		gProjToWorldRotMtx)	\
	/* done */

// -------- Part common to all fullscreen triangle programs --------
#define HANDLE_PROG_FULLSCREEN_TRIANGLE(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	HANDLE_VERTEX(VtxFullScreenTriangle)			\
	/* attributes */								\
	HANDLE_ATTRIBUTE(VtxFullScreenTriangle,	vec2, float, GL_FLOAT, GL_FALSE, pos,		0)			\
	HANDLE_ATTRIBUTE(VtxFullScreenTriangle,	vec2, float, GL_FLOAT, GL_FALSE, uv,		1)			\
	/* done */

// -------- Model --------
#define HANDLE_PROG_MODEL(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	HANDLE_VERTEX(VtxModel)									\
	HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* attributes */										\
	HANDLE_ATTRIBUTE(VtxModel,				vec3, float, GL_FLOAT, GL_FALSE, pos,		0)			\
	HANDLE_ATTRIBUTE(VtxModel,				vec2, float, GL_FLOAT, GL_FALSE, uv,		1)			\
	HANDLE_ATTRIBUTE(VtxModel,				vec3, float, GL_FLOAT, GL_FALSE, normal,	2)			\
	/* done */
	
// -------- Simple --------
#define HANDLE_PROG_SIMPLE(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	HANDLE_VERTEX(VtxSimple)									\
	HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	/* attributes */								\
	HANDLE_ATTRIBUTE(VtxSimple,				vec3, float, GL_FLOAT, GL_FALSE , pos,		0)			\
	HANDLE_ATTRIBUTE(VtxSimple,				vec4, float, GL_FLOAT, GL_FALSE , color,	1)			\
	/* done */

// -------- Lane --------
#define HANDLE_PROG_LANE(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	HANDLE_VERTEX(VtxLane)									\
	HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* attributes */								\
	HANDLE_ATTRIBUTE(VtxLane,				vec3, float, GL_FLOAT, GL_FALSE, pos,		0)			\
	HANDLE_ATTRIBUTE(VtxLane,				vec3, float, GL_FLOAT, GL_FALSE, normal,	1)			\
	/* done */

// -------- Skybox ---------
#define HANDLE_PROG_SKYBOX(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	HANDLE_VERTEX(VtxSkybox)									\
	HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	/* attributes */								\
	HANDLE_ATTRIBUTE(VtxSkybox,				vec2, float, GL_FLOAT, GL_FALSE, pos,		0)			\
	HANDLE_ATTRIBUTE(VtxSkybox,				vec2, float, GL_FLOAT, GL_FALSE, uv,		1)			\
	/* done */

// -------- Tonemapping ---------
#define HANDLE_PROG_TONEMAPPING(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)		\
	HANDLE_PROG_COMMON(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)				\
	HANDLE_PROG_FULLSCREEN_TRIANGLE(HANDLE_VERTEX, HANDLE_UNIFORM, HANDLE_ATTRIBUTE)	\
	/* done */

// ************************** Declare vertex structs *************************
#define HANDLE_VERTEX_DECLARE_STRUCT(vtxStructType) \
	struct vtxStructType {

#define HANDLE_ATTRIBUTE_DECLARE_STRUCT(vtxStructType, varType, componentType, componentTypeEnum, normalized, varName, loc)	\
		varType varName;

#define END_PROGRAM_DECLARE_STRUCT \
	};

FOREACH_GPUPROGRAM(HANDLE_PROG, HANDLE_VERTEX_DECLARE_STRUCT, HANDLE_UNIFORM_NO_ACTION, HANDLE_ATTRIBUTE_DECLARE_STRUCT, END_PROGRAM_DECLARE_STRUCT)

#pragma custom_preprocessor_on
